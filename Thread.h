/*
 * Thread.h
 *
 *  Created on: 17. 4. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_
#define LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_

#include <Arduino.h>
#include <Runnable.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

class Thread {

private:
    friend class Scheduler;

    Thread* next;       //!< Next task.
    const uint8_t* stack;   //!< Task stack.

    volatile uint8_t flags;

    static const uint8_t ENABLED_FLAG_BIT = 0;
    static const uint8_t MARKED_TO_DISABLE_FLAG_BIT = ENABLED_FLAG_BIT + 1;

    jmp_buf context;        //!< Task context.

    Runnable * runnable;
    Runnable * runnableToSet;

    Thread(Thread* next, const uint8_t* stack, Runnable * runnable,
           bool enabled) :
            next(next),
            stack(stack),
            flags(0),
            runnable(runnable),
            runnableToSet(NULL) {
        setEnabled(enabled);
    }

    Thread(Thread* next, const uint8_t* stack, Runnable * runnable) :
            Thread(next, stack, runnable,
                    runnable != NULL) {
    }

    void setEnabled(bool enabled) {
        bitWrite(flags, ENABLED_FLAG_BIT, enabled);
    }

    void setToDisable(bool toDisable) {
        bitWrite(flags, MARKED_TO_DISABLE_FLAG_BIT, toDisable);
    }

    bool isToDisable() {
        return bitRead(flags, MARKED_TO_DISABLE_FLAG_BIT);
    }

public:

    bool isEnabled() {
        return bitRead(flags, ENABLED_FLAG_BIT);
    }

    void enable() {
        if (runnable != NULL || runnableToSet != NULL) {
            if (runnableToSet != NULL) {
                runnable = runnableToSet;
                runnableToSet = NULL;
            }

            setEnabled(true);
        }
    }

    void disable() {
        setToDisable(true);
    }

    /**
     * This function should be called only if thread is not running and is disabled
     */
    void setRunnable(Runnable* runnable) {
        this->runnableToSet = runnable;
    }

	Runnable* getRunnable() {
		return runnable;
	}
};

#endif /* LIBRARIES_ARDUINO_SCHEDULER_THREAD_H_ */
