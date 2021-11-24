#pragma once
#include <ArduinoComponents.h>
#include "Util.h"

using namespace components;

class CurrentSelector:public Component{
public:

    CurrentSelector(PinNumber out,PinNumber c1,PinNumber c2)
        :Component(),currentPin120(c1),currentPin060(c2),currentOutput(out){
            this->currentOutput.low();
            this->currentPin120.low();
            this->currentPin060.low();

        }

    void SetCurrent(CurrentValue current){
        this->setCurrent=current;
    }

    CurrentValue GetSetCurrent(){
        return this->setCurrent;
    }

    CurrentValue CycleCurrent(){
        switch(this->setCurrent){
            case CurrentValue::c150: {
                this->setCurrent=CurrentValue::c120;
                break;
            }
            case CurrentValue::c120: {
                this->setCurrent=CurrentValue::c060;
                break;
            }
            case CurrentValue::c060: {
                this->setCurrent=CurrentValue::c150;
                break;
            }
        }
         return this->setCurrent;
    }

    void TurnOn(){
        switch(this->setCurrent){
            case CurrentValue::c150: {
                this->currentPin120.low();
                this->currentPin060.low();
                this->currentOutput.high();
                return;
            }
            case CurrentValue::c120: {
                this->currentPin120.high();
                this->currentPin060.low();
                this->currentOutput.high();
                return;
            }
            case CurrentValue::c060: {
                this->currentPin120.low();
                this->currentPin060.high();
                this->currentOutput.high();
                return;
            }
        }
    }


    void TurnOff(){
        this->currentOutput.low();
        this->currentPin060.low();
        this->currentPin120.low();
    }
    

private:
    void privateLoop(){

    }

    DigitalOutput currentOutput;
    DigitalOutput currentPin120;
    DigitalOutput currentPin060;
    CurrentValue setCurrent;
    bool enabled;
};
