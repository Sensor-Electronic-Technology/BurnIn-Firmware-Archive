#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

using namespace components;
using namespace std; 

class TemperatureSensor:public Component{
public:
	TemperatureSensor(PinNumber pin) :Component(),input(pin),temperature(0) {

	}

	float Read(){
		float aValue = this->input.read();
		this->temperature+=(((aValue*tempConversion)/MaxADC)-this->temperature)*tempfilter;
		return this->temperature;
	}

	float GetTemperature() {
		return this->temperature;
	}

private:
	AnalogInput input;
	Timer readTimer;
	float temperature=0;

	void privateLoop() {
		this->Read();
	}
};

