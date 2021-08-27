#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

class Probe:public Component{
public:

	Probe(PinNumber voltagePin,int maxSense,int maxV):Component(),voltageIn(voltagePin) {
 	  RegisterChild(this->readTimer);
		this->voltage=0.0;
		this->current=0.0;
		this->maxSensor=maxSense;
		this->maxVolts=maxV;
	} 

	void ReadVoltage() {
		int value=this->voltageIn.read();
		value=map(value,0,maxSensor,0,maxVolts);
		this->voltage += (((float)value) - this->voltage) * fWeight;
		//this->voltage += ((this->voltageIn.read() * MaxVoltageR) - this->voltage) * fWeight;
	}

	float GetVoltage() {
		return this->voltage;
	}

private:
	AnalogInput voltageIn;

	Timer readTimer;
	float voltage;
	float current;
	int maxSensor=1023;
	int maxVolts=108;

	void privateLoop() {
		this->ReadVoltage();
	}
};