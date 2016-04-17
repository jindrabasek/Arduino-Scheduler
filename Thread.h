/*
 * Thread.h
 *
 *  Created on: 17. 4. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_
#define LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_

#include <Runnable.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

class Thread {

private:
    friend class SchedulerClass;

    Thread* next;       //!< Next task.
    Thread* prev;       //!< Previous task.
    const uint8_t* stack;   //!< Task stack.
    volatile bool enabled;
    volatile bool disableFlag;

    jmp_buf context;        //!< Task context.

    Runnable * runnable;

    Thread(Thread* next, Thread* prev, const uint8_t* stack, Runnable * runnable) :
            next(next),
            prev(prev),
            stack(stack),
            enabled(true),
            disableFlag(false),
            runnable(runnable){
    }

public:

    bool isEnabled() {
        return enabled;
    }

    void enable() {
        this->enabled = true;
    }

    void disable() {
        this->disableFlag = true;
    }
};

#endif /* LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_ */
