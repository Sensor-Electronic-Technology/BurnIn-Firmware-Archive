#pragma once
#include <ArduinoComponents.h>
#include "TemperatureSensor.h"

using namespace components;

#define tempHLimit			100
#define tempLLimit			0
#define tempDeviation		10  //% error allowed
#define tempOffset			(0)
#define Kp					20
#define Ki					0.1
#define Kd					0
#define maxDuty				100
#define hiDuty				98
#define lowDuty				2
#define readDelay			5
#define dutyTime			500
#define outputPeriod		1000
#define runtime				50
#define DefaultSetPoint		85

enum HeaterState {
	On,
	Off
};

class HeatingPad:public Component{
public:
	HeatingPad(PinNumber sensorPin,PinNumber heatPin) 
		:Component(),
		output(heatPin), 
		tempSensor(sensorPin) {
		
		this->lastErr = this->err;
		this->err = this->tempSP - this->temperature;
		
		RegisterChild(this->dutyTimer);
		RegisterChild(this->runTimer);
		RegisterChild(this->tempSensor);

		this->dutyTimer.onInterval([&](){
			SetHeaterDuty();
		}, dutyTime);

		this->runTimer.onInterval([&]() {
			SetOuput();
		}, runtime);
	}

	void SetHeaterDuty(){
		temperature = this->tempSensor.GetTemperature();
		lastErr = err; 
		err = tempSP - temperature;
		if (abs(err) * 100 / this->tempSetPoint < tempDeviation) {
			this->tempOk = true;
		} else {
			this->tempOk = false;
		}

		iTerm = iTerm + err * Ki; 
		if (iTerm > 100.0) {
			iTerm = 100.0;
		} else if (iTerm < 0) {
			iTerm = 0;
		}

		pTerm = Kp * err;
		dTerm = dTerm + ((Kd * (err - lastErr)) - dTerm) * 0.05;
		Duty = pTerm + iTerm + dTerm;

		if (Duty > 100.0) {
			Duty= 100.0;
		} else if (Duty < 0.0) {
			Duty = 0.0;
		}

		if (temperature > tempHLimit) {
			heaterDuty = 0;
		} else if (temperature < tempLLimit) {
			heaterDuty = maxDuty;
		} else if (temperature < tempHLimit) {
			heaterDuty = (int(Duty) * maxDuty) / 100;
			if ((heaterDuty > hiDuty) || (heaterDuty > maxDuty)) {
				heaterDuty = maxDuty;
			} else if (heaterDuty < lowDuty) {
				heaterDuty = 0;
			}
		}
	}

	void SetOuput() {
		if (millisTime() >= (lastOutputTime + outputPeriod)) {
			lastOutputTime = lastOutputTime + outputPeriod;
			if (heaterDuty > 0) {
				this->output.high();
			} else {
				this->output.low();
			}
		}
		if (millisTime() >= (lastOutputTime + (outputPeriod / 100) * heaterDuty)) {
			this->output.low();
		}
	}

	float GetTemperature() {
		return this->tempSensor.GetTemperature();
	}

	void ReadTempManual() {
		this->tempSensor.Read();
	}
	
	bool TempOK() {
		return this->tempOk;
	}

	void ChangeSetpoint(int setPoint) {
		this->tempSetPoint = setPoint;
	}

	void SetState(HeaterState state) {
		if (state==HeaterState::On) {
			this->tempSP = this->tempSetPoint;
		} else {
			this->tempSP = 0;
		}
	}

	bool ToggleHeating() {
		if (this->tempSP != this->tempSetPoint) {
			this->tempSP = this->tempSetPoint;
			return true;
		} else {
			this->tempSP = 0;
			return false;
		}
	}

private:
	TemperatureSensor tempSensor;
	DigitalOutput output;
	Timer dutyTimer;	
	Timer runTimer;

	volatile int heaterDuty = 100;
	volatile float Duty = 100.0;
	volatile float pTerm = 0.0;
	volatile float err = 0.0;
	volatile float iTerm = 0.0;
	volatile float dTerm = 0.0;
	volatile float lastErr = 0.0;

	volatile float tempSP=0;
	volatile float temperature=tempSP;
	volatile float tempSetPoint = DefaultSetPoint;
	unsigned long lastOutputTime = 0;
	bool tempOk = false;

	void privateLoop() {

	}
};

