#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "Util.h"
#include "TemperatureSensor.h"

using namespace components;

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
		//this->err = this->tempSP - this->temperature;
		
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
		if (iTerm > iTermMax) {
			iTerm = iTermMax;
		} else if (iTerm < ITermMin) {
			iTerm = ITermMin;
		}

		pTerm = Kp * err;
		dTerm = dTerm + ((Kd * (err - lastErr)) - dTerm) * dTermFact;
		Duty = pTerm + iTerm + dTerm;

		if (Duty > maxDuty) {
			Duty= maxDuty;
		} else if (Duty < minDuty) {
			Duty = minDuty;
		}

		if (temperature > tempHLimit) {
			heaterDuty = 0;
		} else if (temperature < tempLLimit) {
			heaterDuty = maxDuty;
		} else if (temperature < tempHLimit) {
			heaterDuty = (int(Duty) * maxDuty) / maxDuty;
			if ((heaterDuty > hiDuty) || (heaterDuty > maxDuty)) {
				heaterDuty = maxDuty;
			} else if (heaterDuty < lowDuty) {
				heaterDuty = minDuty;
			}
		}
	}

	void SetOuput() {
		if (millisTime() >= (lastOutputTime + outputPeriod)) {
			lastOutputTime = lastOutputTime + outputPeriod;
			if (heaterDuty > minDuty) {
				this->output.high();
			} else {
				this->output.low();
			}
		}
		if (millisTime() >= (lastOutputTime + (outputPeriod / maxDuty) * heaterDuty)) {
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

	volatile int heaterDuty = maxDuty;
	volatile float Duty = maxDuty;
	volatile float pTerm = 0.0;
	volatile float err = 0.0;
	volatile float iTerm = 0.0;
	volatile float dTerm = 0.0;
	volatile float lastErr = 0.0;

	volatile float tempSP=0;
	volatile float temperature=0;
	volatile float tempSetPoint = DefaultSetPoint;
	unsigned long lastOutputTime = 0;
	bool tempOk = false;

	void privateLoop() {

	}
};

