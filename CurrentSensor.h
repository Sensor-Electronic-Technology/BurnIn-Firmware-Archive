#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

using namespace components;

class CurrentSensor:public Component{
public:

	CurrentSensor(PinNumber pin):Component(),currentIn(pin){
		this->current=0;
	}

	void ReadCurrent() {
		int value=this->currentIn.read();
		value=map(value,MinADC,MaxADC,MinCurrent,MaxCurrent);
		this->current+=(((float)value)-this->current)*.01;
	}

	float GetCurrent() {
		return this->current;
	}

private:
	AnalogInput currentIn;

	Timer readTimer;
	float current=0;

	void privateLoop() {
		this->ReadCurrent();
	}
};
