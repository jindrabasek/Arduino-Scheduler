/*
 * SingleThreadPool.h
 *
 *  Created on: 18. 4. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_ARDUINO_SCHEDULER_SINGLETHREADPOOL_H_
#define LIBRARIES_ARDUINO_SCHEDULER_SINGLETHREADPOOL_H_

#include <stddef.h>
#include <Scheduler/Semaphore.h>
#include <Scheduler.h>
#include <Thread.h>

class SingleThreadPool {
    Semaphore accessMutex;
    Thread * thread;

public:

    SingleThreadPool(size_t stackSize = Scheduler::DEFAULT_STACK_SIZE) :
            thread(Scheduler::start(NULL, stackSize)) {
    }

    Thread * aquireThread(){
        await(!thread->isEnabled());
        accessMutex.wait();
        return thread;
    }

    Thread * aquireThreadNonBlocking(){
        if (!thread->isEnabled()) {
            accessMutex.wait();
            return thread;
        }
        return NULL;
    }

    void releaseThread(){
        accessMutex.increase();
    }

};

#endif /* LIBRARIES_ARDUINO_SCHEDULER_SINGLETHREADPOOL_H_ */
