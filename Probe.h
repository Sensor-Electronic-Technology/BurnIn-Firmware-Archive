#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

class Probe:public Component{
public:


	Probe(PinNumber voltagePin):Component(),voltageIn(voltagePin) {
	RegisterChild(this->readTimer);
	this->readTimer.onInterval([&]() {
		this->ReadVoltage();
		}, 50);
	}

	void ReadVoltage() {
		this->voltage += ((this->voltageIn.read() * MaxVoltageR) - this->voltage) * fWeight;
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

	}
};

