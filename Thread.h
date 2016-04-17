/*
 * Thread.h
 *
 *  Created on: 17. 4. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_
#define LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_

#include <setjmp.h>
#include <stdint.h>

class Thread {

private:
    friend class SchedulerClass;

    Thread* next;       //!< Next task.
    Thread* prev;       //!< Previous task.
    const uint8_t* stack;   //!< Task stack.
    volatile bool enabled;
    volatile bool disableFlag;

public:


    jmp_buf context;        //!< Task context.

    Thread(Thread* next, Thread* prev, const uint8_t* stack) :
            next(next),
            prev(prev),
            stack(stack),
            enabled(true),
            disableFlag(false){
    }

    bool isEnabled() {
        return enabled;
    }

    void enable() {
        this->enabled = true;
    }

    void disable() {
        this->disableFlag = true;
    }

    Thread* getNext() {
        return next;
    }

    Thread* getPrev() {
        return prev;
    }

    const uint8_t* getStack() {
        return stack;
    }

    void setNext(Thread* next) {
        this->next = next;
    }

    void setPrev(Thread* prev) {
        this->prev = prev;
    }

    void setStack(const uint8_t* stack) {
        this->stack = stack;
    }
};

#endif /* LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_ */
