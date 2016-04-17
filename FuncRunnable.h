/*
 * FuncRunnable.h
 *
 *  Created on: 17. 4. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_ARDUINO_SCHEDULER_FUNCRUNNABLE_H_
#define LIBRARIES_ARDUINO_SCHEDULER_FUNCRUNNABLE_H_

#include <Runnable.h>
#include <stddef.h>

class FuncRunnable : public Runnable {
public:

    /**
     * Function prototype (task setup and loop functions).
     */
    typedef void (*func_t)();

    FuncRunnable(func_t setupFunc, func_t loopFunc) :
            setupFunc(setupFunc),
            loopFunc(loopFunc) {

    }

    virtual void setup(){
        if (setupFunc != NULL) {
            setupFunc();
        }
    }

    virtual void loop(){
        loopFunc();
    }


private:
    func_t setupFunc;
    func_t loopFunc;
};

#endif /* LIBRARIES_ARDUINO_SCHEDULER_FUNCRUNNABLE_H_ */
