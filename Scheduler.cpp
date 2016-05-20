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

bool Scheduler::begin(size_t stackSize) {
    // Set main task stack size
    s_top = stackSize;
    return (true);
}

Thread* Scheduler::start(Runnable * runnable,
                              size_t stackSize) {
    // Check called from main task and valid task loop function
    if (s_running != &s_main)
        return NULL;

    // Adjust stack size with size of task context
    stackSize += sizeof(Thread);

    // Allocate stack(s) and check if main stack top should be set
    size_t frame = RAMEND - (size_t) &frame;
    uint8_t stack[s_top - frame];
    if (s_main.stack == NULL)
        s_main.stack = stack;

#if defined(ARDUINO_ARCH_AVR)
    // Check that the task can be allocated
    int HEAPEND = (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    int STACKSTART = ((int) stack) - stackSize;
    HEAPEND += __malloc_margin;
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


