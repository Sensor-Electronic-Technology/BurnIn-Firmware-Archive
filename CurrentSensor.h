#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

using namespace components;

class CurrentSensor:public Component{
public:

	CurrentSensor(PinNumber pin):Component(),currentIn(pin){
	}

	void ReadCurrent() {
		int value=this->currentIn.read();
		value=map(value,0,1023,0,200);
		this->current+=(((float)value)-this->current)*fWeight;
		//this->current += ((this->currentIn.read() * CurrentVoltage) - this->current) * fWeight;
	}

	float GetCurrent() {
		return this->current;
	}

private:
	AnalogInput currentIn;

	Timer readTimer;
	float current;

	void privateLoop() {
		this->ReadCurrent();
	}
};
