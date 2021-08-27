#pragma once
#include <ArduinoComponents.h>
#include "TemperatureSensor.h"



using namespace components;

enum class SwitchState{
    On=true,
    Off=false
};

enum class CurrentMode{
    FullMode,
    HalfMode
};

class CurrentSwitch:public Component{
public:
    CurrentSwitch(bool switchingEnabled,CurrentMode cmode,int lPin,int fPin)
        :Component(),
        currentOutput(lPin),fullCurrentOutput(fPin),mode(cmode){  
    }

    void SwitchCurrent(SwitchState state){
        if(switchingEnabled){
            if(mode==CurrentMode::HalfMode){
                if(state==SwitchState::On){
                    this->currentOutput.high();
                    this->fullCurrentOutput.low();
                }else{
                    this->currentOutput.low();
                    this->fullCurrentOutput.low();
                }
            }else{
                if(state==SwitchState::On){
                    this->currentOutput.high();
                    this->fullCurrentOutput.high();
                }else{
                    this->currentOutput.low();
                    this->fullCurrentOutput.low();
                }
            }
        }else{
            if(state==SwitchState::On){
                this->currentOutput.high();
                this->fullCurrentOutput.high();
            }else{
                this->currentOutput.low();
                this->fullCurrentOutput.low();
            }
        }
    }

    void SwitchingEnabled(bool enabled){
        this->switchingEnabled=enabled;
    }

    void SetCurrentMode(CurrentMode cmode){
        this->mode=cmode;
    }

    CurrentMode ToggleMode(){
        if(this->mode==CurrentMode::FullMode){
            this->mode=CurrentMode::HalfMode;
        }else{
            this->mode=CurrentMode::FullMode;
        }
        return this->mode;
    }
    

private:
    void privateLoop(){
        
    }
	DigitalOutput currentOutput;
    DigitalOutput fullCurrentOutput;
    CurrentMode mode;
    bool switchingEnabled;
    
};

