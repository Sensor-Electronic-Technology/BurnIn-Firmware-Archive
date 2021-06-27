#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include "HeatingPad.h"
#include "Probe.h"
#include "Util.h"
#include <Arduino.h>
#include <ctime>

using namespace components;
using namespace std;



#define LedPin		2
#define heatPin1	3
#define heatPin2	4
#define heatPin3	5
#define BurnTime120 (unsigned long)72000000 //20Hrs
#define BurnTime60	BurnTime120
#define BurnTime150 (unsigned long)25200000  //7hrs


struct SystemState {

	SystemState() {
		this->running = false;
		this->paused = false;
		this->is150On =false;
		this->tempSP = 0;
		this->setCurrent = 150;
		this->elapsed = 0;
	}

	bool running = false;
	bool paused = false;
	bool is150On  = true;
	bool tempsOk = false;
	int tempSP = 0;
	int setCurrent = 150;
	unsigned long elapsed = 0;

	bool IsRunning() {
		return this->running || this->paused;
	}

	void Print() {
		cout << "SystemState: " << endl;
		cout << "Running: " << running << " Paused: " << paused << " Is150On: " << is150On << " tempSP: " << tempSP << " setCurrent: " << setCurrent << " Elapsed: " << elapsed << endl;
	}
};

struct SystemSettings {
	SystemSettings() {
		this->switchingEnabled = false;
		this->current = 150;
		this->current2 = 120;
		this->setTemperature = 85;
	}

	bool switchingEnabled = false;
	int current = 150;
	int current2 = 120;
	int setTemperature = 85;

	void Print() {
		cout << "SystemSettings:" << endl;
		cout << "Switch?: " << switchingEnabled << " Current1: " << current << " Current2: " << current2 << " Temp:: " << setTemperature <<endl;
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
	void HandleSerial();
	void TurnOnOffHeat(HeaterState state);
	int WriteToMemory(int index,void* data);
	void ReadNewSettings(SystemSettings newSettings);
	bool CheckSettings(SystemSettings newSettings);

private:
	vector<HeatingPad*> heatingPads;
	vector<Probe*> probes;


	DigitalOutput ledPin;
	DigitalOutput fullCurrentPin;
	
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

	void privateLoop();
};

