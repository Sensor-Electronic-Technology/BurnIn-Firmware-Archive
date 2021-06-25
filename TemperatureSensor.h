#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>

using namespace components;
using namespace std; 

class TemperatureSensor:public Component{
public:
	TemperatureSensor(PinNumber pin) :Component(),input(pin) {
		RegisterChild(this->readTimer);
		this->readTimer.onInterval([&]() {
			this->Read();
		}, 50);
	}

	float Read(){
		float aValue = this->input.read();
		this->temperature+=((aValue*500)-this->temperature)*.1;
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

	}
};

