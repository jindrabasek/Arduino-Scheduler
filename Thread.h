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
    Runnable * runnableToSet;

    Thread(Thread* next, Thread* prev, const uint8_t* stack,
           Runnable * runnable, bool enabled) :
            next(next),
            prev(prev),
            stack(stack),
            enabled(enabled),
            disableFlag(false),
            runnable(runnable),
            runnableToSet(NULL) {
    }

    Thread(Thread* next, Thread* prev, const uint8_t* stack,
           Runnable * runnable) :
            Thread(next, prev, stack, runnable, runnable != NULL) {
    }

public:

    bool isEnabled() {
        return enabled;
    }

    void enable() {
        if (runnable != NULL || runnableToSet != NULL) {
            if (runnableToSet != NULL) {
                runnable = runnableToSet;
                runnableToSet = NULL;
            }

            this->enabled = true;
        }
    }

    void disable() {
        this->disableFlag = true;
    }

    /**
     * This function should be called only if thread is not running and is disabled
     */
    void setRunnable(Runnable* runnable) {
        this->runnableToSet = runnable;
    }
};

#endif /* LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_ */
