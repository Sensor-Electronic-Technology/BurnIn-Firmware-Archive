#pragma once
#include <ArduinoComponents.h>
#include "TemperatureSensor.h"



using namespace components;

enum class SwitchState{
    On=true,
    Off=false
};

class CurrentSwitch:public Component{
public:
    CurrentSwitch(bool switchingEnabled,int lPin,int fPin)
        :Component(),
        currentOutput(lPin),fullCurrentOutput(fPin){
            
    }
    void SwitchCurrent(SwitchState state){
        if(switchingEnabled){
            if(state==SwitchState::On){
                this->currentOutput.high();
                this->fullCurrentOutput.high();
            }else{
                this->currentOutput.low();
                this->fullCurrentOutput.low();
            }

        }else{
            if(state==SwitchState::On){
                this->currentOutput.high();
                this->fullCurrentOutput.low();
            }else{
                this->currentOutput.low();
                this->fullCurrentOutput.low();
            }
        }
    }

    void SwitchingEnabled(bool enabled){
        this->switchingEnabled=enabled;
    }

private:
    void privateLoop(){
        
    }
	DigitalOutput currentOutput;
    DigitalOutput fullCurrentOutput;
    bool switchingEnabled;
};

