#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

class Probe:public Component{
public:

	Probe(PinNumber voltagePin):Component(),voltageIn(voltagePin) {
 	  RegisterChild(this->readTimer);
		this->voltage=0.0;
		this->current=0.0;
	} 

	void ReadVoltage() {
		int value=this->voltageIn.read();
		value=map(value,MinADC,MaxADC,MinVoltage,MaxVoltage);
		this->voltage += (((float)value) - this->voltage) * fWeight;
	}

	float GetVoltage() {
		return this->voltage;
	}

private:
	AnalogInput voltageIn;

	Timer readTimer;
	float voltage;
	float current;

	void privateLoop() {
		this->ReadVoltage();
	}
};