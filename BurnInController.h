#pragma once
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include <avr/wdt.h>
#include "CurrentSelector.h"
#include "HeatingPad.h"
#include "Probe.h"
#include "CurrentSwitch.h"
#include "Util.h"
#include "CurrentSensor.h"
#include <ctime>

using namespace components;
using namespace std;

struct SystemState {
	SystemState() {
		this->running = false;
		this->paused = false;
		this->setCurrent =CurrentValue::c150;
		this->tempSP = 0;
		this->elapsed = 0;
		this->tempsOk=false;
	}

	bool running;
	bool paused;
	bool tempsOk;
	int tempSP;
	CurrentValue setCurrent;
	unsigned long elapsed;

	void Set(const SystemState& newState){
		this->running=newState.running;
		this->paused=newState.paused;
		this->tempsOk=newState.tempsOk;
		this->setCurrent=newState.setCurrent;
		this->tempSP=newState.tempSP;
		this->elapsed=newState.elapsed;
		this->tempsOk=newState.tempsOk;
	}
	
	bool IsRunning() {
		return this->running || this->paused;
	}

	void Print() {
		cout<<"[T]{"<< "Running: " << running << " Paused: " << paused << " tempSP: " << tempSP << " setCurrent: " << setCurrent << " Elapsed: " << elapsed <<"}"<< endl;
	}
};

struct SystemSettings {
	SystemSettings() {
		this->switchingEnabled = false;
		this->setCurrent=CurrentValue::c150;
		this->setTemperature = 85;
	}
	bool switchingEnabled = false;
	int setTemperature;
	CurrentValue setCurrent;

	void Set(const SystemSettings& settings){
		this->switchingEnabled=settings.switchingEnabled;
		this->setTemperature=settings.setTemperature;
		this->setCurrent=settings.setCurrent;
	}

	void Print() {
		cout<<"[T]{"<<"System Settings: "<< "Switch?: " << switchingEnabled << " Current2: " << " Temp:: " << setTemperature <<"}"<<endl;
	}
};

struct BurnTimer {

	unsigned long lastCheck=0;
	unsigned long elapsed=0;
	unsigned long lengthSecs=0;
	bool running = false;
	bool paused = false;

	bool check() {
		if(this->running && !this->paused){
			if(millisTime()-this->lastCheck>=(TPeriod*TFactor)){
				this->lastCheck=millisTime();
				this->elapsed+=1;
				this->running=!((this->elapsed*TPeriod)>=this->lengthSecs);
			}
			return !this->running;
		}else{
			return false;
		}
	}

	void start(unsigned long length) {
		if(!this->running && !this->paused){
			this->lengthSecs=length;
			this->lastCheck=millisTime();
			this->elapsed=0;
			this->running=true;
			this->paused=false;
		}
	}

	void start() {
		if(!this->running && !this->paused){
			this->lastCheck=millisTime();
			this->elapsed=0;
			this->running=true;
			this->paused=false;
		}
	}

	void Pause() {
		if(this->running && !this->paused){
			this->paused=true;
		}
	}
	
	void Continue() {
		if(this->running && this->paused){
			this->paused=false;
		}
	}

	void Stop() {
		this->running=false;
		this->paused=false;
		this->lastCheck=0;
		this->elapsed=0;
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
	void ReadNewSettings(SystemSettings newSettings);
	bool CheckSettings(SystemSettings newSettings);

private:
	vector<HeatingPad*> heatingPads;
	vector<Probe*> probes;
	vector<CurrentSensor*> currentSensors;

	//CurrentSwitch currentSwitch;
	CurrentSelector currentSelector;
	Timer printTimer;
	Timer updateTimer;
	Timer saveStateTimer;

	SystemSettings settings;
	SystemState systemState;
	BurnTimer burnTimer;

	int settingsAddr = 0;
	bool fullCurrentOn = true;
	float realArray[100];
	bool boolArray[100];
	boolean limitArray[10];
	boolean climits[6];

	float t1, t2, t3;
	bool firstTimeCheck;

	void privateLoop();
};

