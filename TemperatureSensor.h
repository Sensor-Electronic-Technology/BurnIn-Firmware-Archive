#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"

#define A_Value	1.945714e-3f
#define B_Value 2.3702e-5f
#define C_Value 1.545e-6f
#define Rref	1000

using namespace components;
using namespace std; 

class TemperatureSensor:public Component{
public:
	TemperatureSensor(PinNumber pin) :Component(),input(pin),temperature(0) {	}

	float Read(){
		double aValue = this->input.read();
		double ntc_res=Rref/(MaxADC/aValue-1);
		//Steinhart and Hart Equation 1/A+Bln(R)+C[ln(R)]^3  
		double temp=1/(A_Value+(B_Value*log(ntc_res))+(C_Value*pow(log(ntc_res),3)));
		this->temperature+=(temp-this->temperature)*tempfilter;
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

