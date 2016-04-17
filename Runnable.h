/*
 * Runnable.h
 *
 *  Created on: 17. 4. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_ARDUINO_SCHEDULER_RUNNABLE_H_
#define LIBRARIES_ARDUINO_SCHEDULER_RUNNABLE_H_

class Runnable {
public:
    virtual ~Runnable(){
    }

    virtual void setup(){
    }

    virtual void loop() = 0;
};

#endif /* LIBRARIES_ARDUINO_SCHEDULER_RUNNABLE_H_ */
