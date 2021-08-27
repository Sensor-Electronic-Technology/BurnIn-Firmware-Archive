#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

using namespace components;
using namespace std; 

class TemperatureSensor:public Component{
public:
	TemperatureSensor(PinNumber pin) :Component(),input(pin) {
//		RegisterChild(this->readTimer);
/* 		this->readTimer.onInterval([&]() {
			this->Read();
		}, tempReadTime); */
	}

	float Read(){
		float aValue = this->input.read()/1023;
		this->temperature+=((aValue*tempConversion)-this->temperature)*tempfilter;
		return this->temperature;
	}

	float GetTemperature() {
		return this->temperature;
	}

private:
	AnalogInput input;
	Timer readTimer;
	float temperature;

	void privateLoop() {
		this->Read();
	}
};

