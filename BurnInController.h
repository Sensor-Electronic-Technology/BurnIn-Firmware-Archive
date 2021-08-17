#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "HeatingPad.h"
#include "Probe.h"
#include "CurrentSwitch.h"
#include "Util.h"
#include <Arduino.h>
#include <ctime>

using namespace components;
using namespace std;




struct SystemState {
	SystemState() {
		this->running = false;
		this->paused = false;
		this->isFullCurrent =false;
		this->tempSP = 0;
		this->setCurrent = 150;
		this->elapsed = 0;
		this->tempsOk=false;
	}

	bool running;
	bool paused;
	bool isFullCurrent;
	bool tempsOk;
	int tempSP;
	int setCurrent;
	unsigned long elapsed;

	bool IsRunning() {
		return this->running || this->paused;
	}

	void Print() {
		cout<<"[T]{"<< "Running: " << running << " Paused: " << paused << " Is150On: " << isFullCurrent << " tempSP: " << tempSP << " setCurrent: " << setCurrent << " Elapsed: " << elapsed <<"}"<< endl;
	}
};

struct SystemSettings {
	SystemSettings() {
		this->switchingEnabled = true;
		this->current2 = 60;
		this->setCurrent=this->current2;
		this->setTemperature = 85;
	}

	bool switchingEnabled = false;
	int current2;
	int setTemperature;
	int setCurrent;

	void Print() {
		cout<<"[T]{"<<"System Settings: "<< "Switch?: " << switchingEnabled << " Current2: " << current2 << " Temp:: " << setTemperature <<"}"<<endl;
	}
};

struct BurnTimer {
	unsigned long burnInStartTime=0;
	unsigned long elapsed=0;
	unsigned long burnInTime = 0;
	unsigned long burnInTimeLength=0;
	unsigned long pausedTime=0;
	bool running = false;
	bool paused = false;

	bool check() {
		if (this->running && !this->paused) {
			this->burnInTime = millisTime();
			this->elapsed = this->burnInTime - this->burnInStartTime;
			this->running = !(this->elapsed >= this->burnInTimeLength);
			return !this->running;
		} else {
			return false;
		}
	}

	void start(unsigned long length) {
		if (!this->running && !this->paused) {
			this->burnInTimeLength = length;
			this->burnInStartTime = millisTime();
			this->pausedTime = 0;
			this->elapsed = 0;
			this->running = true;
			this->paused = false;
			this->elapsed = 0;
		}
	}

	void start() {
		if (!this->running && !this->paused) {
			this->burnInStartTime = millisTime();
			this->pausedTime = 0;
			this->elapsed = 0;
			this->running = true;
			this->paused = false;
			this->elapsed = 0;
		}
	}

	void Pause() {
		if (!this->running && !this->paused) {
			this->paused = true;
			this->pausedTime = millisTime();
		}
	}
	
	void Continue() {
		if (this->paused && this->running) {
			this->paused = false;
			this->burnInTime = millisTime();
			this->burnInTimeLength += (this->burnInTime - this->pausedTime);
		}
	}

	void Stop() {
		if (this->running || this->paused) {
			this->running = false;
			this->paused = false;
			this->burnInTime = 0;
			this->elapsed = 0;
			this->burnInStartTime = 0;
		}
	}
};

enum class ControllerState {
	Running,
	Idle,
	Paused,
	WarmingUp
};

class BurnInController:public Component{
public:
	BurnInController();
	bool IsRunning();
	void Setup();
	void UpdateData();
	void StartTest();
	void Reset();
	void TestProbe();
	void ToggleHeating();
	void ToggleCurrent();
	void PauseTest();
	void LoadFromMemory();
	void CheckStart();
	void SetupIO();
	void SetupTimers();
	void HandleSerial();
	void sendComs();
	void TurnOnOffHeat(HeaterState state);
	int WriteToMemory(int index,void* data);
	void ReadNewSettings(SystemSettings newSettings);
	bool CheckSettings(SystemSettings newSettings);

private:
	vector<HeatingPad*> heatingPads;
	vector<Probe*> probes;

	CurrentSwitch currentSwitch;
	//DigitalOutput ledPin;
	//DigitalOutput fullCurrentPin;
	
	Timer printTimer;
	Timer updateTimer;

	SystemSettings settings;
	SystemState systemState;
	BurnTimer burnTimer;

	int settingsAddr = 0;
	bool fullCurrentOn = true;
	float realArray[100];
	bool boolArray[100];
	boolean limitArray[10];

	float t1, t2, t3;
	bool firstTimeCheck;

	void privateLoop();
};
