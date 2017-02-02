/**
 * @file Scheduler.cpp
 * @version 1.2
 *
 * @section License
 * Copyright (C) 2015-2016, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include <avr/io.h>
#include <FuncRunnable.h>
#include <Runnable.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <Scheduler.h>
#include <Thread.h>
#include <Task.h>

// Configuration: SRAM and heap handling
#if defined(TEENSYDUINO) && defined(__MK20DX256__)
#undef ARDUINO_ARCH_AVR
#define TEENSY_ARCH_ARM
#define RAMEND 0x20008000

#elif defined(ARDUINO_ARCH_AVR)
extern int __heap_start, *__brkval;
extern char* __malloc_heap_end;
extern size_t __malloc_margin;

#elif defined(ARDUINO_ARCH_SAM)
#define RAMEND 0x20088000

#elif defined(ARDUINO_ARCH_SAMD)
#define RAMEND 0x20008000
#endif

// Main task and run queue
Thread Scheduler::s_main(&Scheduler::s_main,
        NULL, NULL, true);

// Reference running task
Thread* Scheduler::s_running = &Scheduler::s_main;
Thread* Scheduler::s_last = &Scheduler::s_main;

// Initial top stack for task allocation
size_t Scheduler::s_top = Scheduler::DEFAULT_STACK_SIZE;

#ifdef SCHEDULER_SHOW_TASK_ID_ON_LED
#ifndef SCHEDULER_SHOW_TASK_ID_ON_LED_ENABLE_REGISTER
#define SCHEDULER_SHOW_TASK_ID_ON_LED_ENABLE_REGISTER DDRA
#endif
#ifndef SCHEDULER_SHOW_TASK_ID_ON_LED_WRITE_REGISTER
#define SCHEDULER_SHOW_TASK_ID_ON_LED_WRITE_REGISTER PORTA
#endif
#endif


bool Scheduler::begin(size_t stackSize) {
#ifdef SCHEDULER_SHOW_TASK_ID_ON_LED
	SCHEDULER_SHOW_TASK_ID_ON_LED_ENABLE_REGISTER = 0xFF;
#endif

    // Set main task stack size
    s_top = stackSize;
    return (true);
}

Thread* Scheduler::start(Runnable * runnable,
                              size_t stackSize) {

#ifdef SCHEDULER_LOG_SERIAL
    Serial.println();
    Serial.println(F("--- Start thread ---"));
#endif

    // Check called from main task and valid task loop function
    if (s_running != &s_main)
        return NULL;

    // Adjust stack size with size of task context
    stackSize += sizeof(Thread);

    // Allocate stack(s) and check if main stack top should be set
    size_t frame = RAMEND - (size_t) &frame;

#ifdef SCHEDULER_LOG_SERIAL
    Serial.println(F("s_top"));
    Serial.println(s_top, 16);
    Serial.println(s_top, 10);
    Serial.println(F("frame"));
    Serial.println(frame, 16);
    Serial.println(frame, 10);
    Serial.println(F("s_top - frame"));
    Serial.println(s_top - frame, 16);
    Serial.println(s_top - frame, 10);
#endif

    uint8_t stack[s_top - frame];
    if (s_main.stack == NULL)
        s_main.stack = stack;

#if defined(ARDUINO_ARCH_AVR)
    // Check that the task can be allocated
    int HEAPEND = (__brkval == 0 ? (int) &__heap_start : (int) __brkval);

#ifdef SCHEDULER_LOG_SERIAL
    Serial.println(F("HEAPEND"));
    Serial.println(HEAPEND, 16);
    Serial.println(HEAPEND, 10);
    Serial.println(F("stackSize"));
    Serial.println(stackSize, 16);
    Serial.println(stackSize, 10);
    Serial.println(F("(int) stack"));
    Serial.println((int) stack, 16);
    Serial.println((int) stack, 10);
#endif

    int STACKSTART = ((int) stack) - stackSize;

#ifdef SCHEDULER_LOG_SERIAL
    Serial.println(F("STACKSTART"));
    Serial.println(STACKSTART, 16);
    Serial.println(STACKSTART, 10);
#endif

    HEAPEND += __malloc_margin;

#ifdef SCHEDULER_LOG_SERIAL
    Serial.println(F("HEAPEND"));
    Serial.println(HEAPEND, 16);
    Serial.println(HEAPEND, 10);
#endif

    if (STACKSTART < HEAPEND)
        return NULL;

    // Adjust heap limit
    __malloc_heap_end = (char*) STACKSTART;
#endif

#if defined(ARDUINO_ARCH_SAM)  || \
    defined(ARDUINO_ARCH_SAMD) || \
    defined(TEENSY_ARCH_ARM)
    // Check that the task can be allocated
    if (s_top + stackSize > STACK_MAX) return NULL;
#endif

    // Adjust stack top for next task allocation
    s_top += stackSize;

#ifdef SCHEDULER_LOG_SERIAL
    Serial.println(F("s_top"));
    Serial.println(s_top, 16);
    Serial.println(s_top, 10);
    Serial.println(F("(int)(stack - stackSize)"));
    Serial.println((int)(stack - stackSize), 16);
    Serial.println((int)(stack - stackSize), 10);
#endif

    // Initiate task with given functions and stack top
    return init(runnable, stack - stackSize);
}

void Scheduler::yield() {
    // Caller will continue here on yield
    if (setjmp(s_running->context)) {
        return;
    }

    // Next task in run queue will continue
    do {
        s_running = s_running->next;
    } while (!s_running->isEnabled());

#if defined(SCHEDULER_LOG_SERIAL) or defined(SCHEDULER_SHOW_TASK_ID_ON_LED)
    if (s_running->getRunnable() != NULL) {
#ifdef SCHEDULER_SHOW_TASK_ID_ON_LED
    	SCHEDULER_SHOW_TASK_ID_ON_LED_WRITE_REGISTER = ~static_cast<Task *>(s_running->getRunnable())->getTaskId();
#endif
#ifdef SCHEDULER_LOG_SERIAL
    	Serial.print(F("Thread running "));
    	Serial.println(static_cast<Task *>(s_running->getRunnable())->getTaskId());
#endif
    } else {
#ifdef SCHEDULER_SHOW_TASK_ID_ON_LED
    	SCHEDULER_SHOW_TASK_ID_ON_LED_WRITE_REGISTER = ~0;
#endif
#ifdef SCHEDULER_LOG_SERIAL
    	Serial.println(F("Thread running main"));
#endif
    }
#endif

    longjmp(s_running->context, true);
}

void Scheduler::disable(){
    s_running->disable();
}

size_t Scheduler::stack() {
    unsigned char marker;
    return (&marker - s_running->stack);
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-local-addr"
Thread* Scheduler::init(Runnable * runnable, const uint8_t* stack) {
    // Add task last in run queue (main task)
    Thread task(&s_main, stack, runnable);

    s_last->next = &task;
    s_last = &task;

    // Create context for new task, caller will return
    if (setjmp(task.context)) {
        while (1) {
            task.runnable->setupInternal();
            task.runnable->loop();
            if (task.isToDisable()) {
                task.setEnabled(false);
                task.setToDisable(false);
                yield();
            }
        }
    }

    return &task;
}
#pragma GCC diagnostic pop

extern "C" void yield(void) {
    Scheduler::yield();
}


