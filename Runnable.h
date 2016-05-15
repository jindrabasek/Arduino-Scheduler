/*
 * Runnable.h
 *
 *  Created on: 17. 4. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_ARDUINO_SCHEDULER_RUNNABLE_H_
#define LIBRARIES_ARDUINO_SCHEDULER_RUNNABLE_H_

// Do not define virtual destructor on purpose - class
// and its children is not expected to need destructors,
// it saves a lot of SRAM otherwise occupied by VTABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

class Runnable {
private:
    friend class SchedulerClass;
    volatile bool setupExecuted = false;

    void setupInternal(){
        if (!setupExecuted) {
            setupExecuted = true;
            setup();
        }
    }

public:
    virtual void setup(){
    }

    virtual void loop() = 0;
};

#pragma GCC diagnostic pop

#endif /* LIBRARIES_ARDUINO_SCHEDULER_RUNNABLE_H_ */
