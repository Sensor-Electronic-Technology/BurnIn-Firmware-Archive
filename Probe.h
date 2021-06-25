#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include <Arduino.h>

#define MaxVoltageR			105
#define MaxVoltageS			68
#define ZeroCurrent			2.5
#define CurrentVoltage		5
#define fWeight				.1

class Probe:public Component{
public:
	Probe(PinNumber voltagePin, PinNumber currentPin):Component(),voltageIn(voltagePin),currentIn(currentPin) {
		RegisterChild(this->readTimer);
		this->readTimer.onInterval([&]() {
			this->ReadVoltage();
			this->ReadCurrent();
		}, 100);
	}

	void ReadVoltage() {
		this->voltage += ((this->voltageIn.read() * MaxVoltageR) - this->voltage) * fWeight;
	}

	void ReadCurrent() {
		this->current += ((this->currentIn.read() * CurrentVoltage) - this->current) * fWeight;
	}

	float GetVoltage() {
		return this->voltage;
	}

	float GetCurrent() {
		return this->current;
	}

private:
	AnalogInput voltageIn;
	AnalogInput currentIn;
	
	Timer readTimer;
	float voltage;
	float current;

	void privateLoop() {

	}
};

