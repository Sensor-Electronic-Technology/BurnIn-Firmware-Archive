#include "BurnInController.h"

void(*resetFunc) (void) = 0; //declare reset function @ address 0

BurnInController::BurnInController() 
	:Component(),
	currentSwitch(true,CurrentMode::FullMode,LedPin,FullPin){
		this->currentSwitch.SwitchCurrent(SwitchState::Off);
}

bool BurnInController::IsRunning() {
	return this->systemState.IsRunning();
}

void BurnInController::Setup() {
	analogReference(DEFAULT);
	Serial.println(message_table[FimwareInitMsg]);
	for (int i = 0; i < 100; i++) {
		this->realArray[i] = 0;
		this->boolArray[i] = 0;
		if (i < 10) {
			this->limitArray[i] = false;
		}
	}
	Serial.println(message_table[InternalMemMsg]);
	this->LoadFromMemory();
	Serial.println(message_table[MemCheckFinishedMsg]);
	Serial.println(message_table[PrintMemMsg]);
	this->systemState.Print();
	this->settings.Print();
	Serial.println(message_table[SettingIOMsg]);
	this->SetupIO();

	Serial.println(message_table[TakingMeasMsg]);

	for (int i = 0; i < 100; i++) {
		//Initialize Temperature Readings
		for (auto pad : heatingPads) {
			pad->ReadTempManual();
		}
		//Initialize Voltage Readings
		for (auto probe : probes) {
			probe->ReadVoltage();
		}
		//Initialize Current Readings
		for(auto cSensor:currentSensors){
			cSensor->ReadCurrent();
		}
	}

	Serial.println(message_table[IOCompleteMsg]);
	Serial.println(message_table[TimerInitMsg]);
	this->SetupTimers();
	Serial.println(message_table[TimerCompleteMsg]);
	Serial.println(message_table[FirmwareCompleteMsg]);
	this->CheckStart();
}

void BurnInController::SetupTimers() {
	this->updateTimer.onInterval([&]() {
		this->UpdateData();
	},UPDATEPERIOD);

	this->printTimer.onInterval([&]() {
		this->sendComs();
	},COMPERIOD);

	this->saveStateTimer.onInterval([&](){
		if(this->systemState.IsRunning()){
			EEPROM_write(StartAddr,this->systemState);
		}
	},EEPROMPERIOD);

	RegisterChild(this->printTimer);
	RegisterChild(this->updateTimer);
	RegisterChild(this->saveStateTimer);
}

void BurnInController::sendComs(){
	String buffer = "";
	for (int x = 0; x < 18; x++) {
		buffer += "[R" + String(x) + "]{" + String(realArray[x]) + "}";
	}
	for (int x = 0; x <= 4; x++) {
		buffer += "[B" + String(x)+ "]{" + String(boolArray[x]) + "}";
	}
	Serial.println(buffer);
}

void BurnInController::SetupIO() {
	Probe* temp1 = new Probe(Probe1);
	this->probes.push_back(temp1);
	RegisterChild(temp1);

	Probe* temp2 = new Probe(Probe2);
	this->probes.push_back(temp2);
	RegisterChild(temp2);

	Probe* temp3 = new Probe(Probe3);
	this->probes.push_back(temp3);
	RegisterChild(temp3);

	Probe* temp4 = new Probe(Probe4);
	this->probes.push_back(temp4);
	RegisterChild(temp4);

	Probe* temp5 = new Probe(Probe5);
	this->probes.push_back(temp5);
	RegisterChild(temp5);

	Probe* temp6 = new Probe(Probe6);
	this->probes.push_back(temp6);
	RegisterChild(temp6);

	CurrentSensor* sensor1=new CurrentSensor(Current1);
	this->currentSensors.push_back(sensor1);
	RegisterChild(sensor1);

	CurrentSensor* sensor2=new CurrentSensor(Current2);
	this->currentSensors.push_back(sensor2);
	RegisterChild(sensor2);

	CurrentSensor* sensor3=new CurrentSensor(Current3);
	this->currentSensors.push_back(sensor3);
	RegisterChild(sensor3);

	CurrentSensor* sensor4=new CurrentSensor(Current4);
	this->currentSensors.push_back(sensor4);
	RegisterChild(sensor4);

	CurrentSensor* sensor5=new CurrentSensor(Current5);
	this->currentSensors.push_back(sensor5);
	RegisterChild(sensor5);

	CurrentSensor* sensor6=new CurrentSensor(Current6);
	this->currentSensors.push_back(sensor6);
	RegisterChild(sensor6);

	HeatingPad* pad1 = new HeatingPad(TempPin1, heatPin1);
	this->heatingPads.push_back(pad1);
	RegisterChild(pad1);

	HeatingPad* pad2 = new HeatingPad(TempPin2, heatPin2);
	this->heatingPads.push_back(pad2);
	RegisterChild(pad2);

	HeatingPad* pad3 = new HeatingPad(TempPin3, heatPin3);
	this->heatingPads.push_back(pad3);
	RegisterChild(pad3);	
}

void BurnInController::LoadFromMemory() {
	SystemState state;
	this->settingsAddr = EEPROM_read(StartAddr, state);
	if(state.IsRunning()){
		this->systemState.Set(state);
	}
	EEPROM_read(this->settingsAddr, this->settings);
	for (auto pad : heatingPads) {
		pad->ChangeSetpoint(this->settings.setTemperature);
	}
	this->currentSwitch.SwitchingEnabled(this->settings.switchingEnabled);
	if(this->systemState.isFullCurrent){
		this->currentSwitch.SetCurrentMode(CurrentMode::FullMode);
	}else{
		this->currentSwitch.SetCurrentMode(CurrentMode::HalfMode);
	}
}

void BurnInController::CheckStart() {
	if (this->systemState.IsRunning()) {
		for (int c = 0; c <= 5; c++) {
			limitArray[c] = false;
		}
		this->systemState.running = true;
		this->systemState.paused = false;
		this->TurnOnOffHeat(HeaterState::On);
		if (this->systemState.isFullCurrent) {
			this->burnTimer.burnInTimeLength = BurnTime150;
			this->systemState.setCurrent = FullCurrent;
		} else {
			this->burnTimer.burnInTimeLength = BurnTime120;
			this->systemState.setCurrent = this->settings.current2;
		}
		this->burnTimer.elapsed = this->systemState.elapsed;
		this->burnTimer.burnInStartTime = millisTime() - this->burnTimer.elapsed;
		this->burnTimer.running = true;
		this->burnTimer.paused = false;
		this->currentSwitch.SwitchCurrent(SwitchState::On);
		
		unsigned long timeLeft = (this->burnTimer.burnInTimeLength - this->burnTimer.elapsed) / 1000;
		unsigned long hrs = timeLeft / 3600;
		unsigned long mins = (timeLeft / 60) % 60;
		unsigned long seconds = (timeLeft % 60);
		String time = String(hrs) + ':' + String(mins) + ':' + String(seconds);
		Serial.println("[T]{Continuing " + String(this->systemState.setCurrent) + "mA Test Now:Runtime= " + time + "}");
	}
}

void BurnInController::UpdateData() {
	boolArray[0] = this->systemState.IsRunning();
	boolArray[1] = digitalRead(heatPin1);
	boolArray[2] = digitalRead(heatPin2);
	boolArray[3] = digitalRead(heatPin3);
  	boolArray[4]=this->systemState.paused;

	realArray[0] = this->probes[0]->GetVoltage();
	realArray[1] = this->probes[1]->GetVoltage();
	realArray[2] = this->probes[2]->GetVoltage();
	realArray[3] = this->probes[3]->GetVoltage();
	realArray[4] = this->probes[4]->GetVoltage();
	realArray[5] = this->probes[5]->GetVoltage();

	realArray[12]=this->currentSensors[0]->GetCurrent();
	realArray[13]=this->currentSensors[1]->GetCurrent();
	realArray[14]=this->currentSensors[2]->GetCurrent();
	realArray[15]=this->currentSensors[3]->GetCurrent();
	realArray[16]=this->currentSensors[4]->GetCurrent();
	realArray[17]=this->currentSensors[5]->GetCurrent();


	bool t1Okay,t2Okay,t3Okay;

	realArray[6]=this->heatingPads[0]->GetTemperature();
	t1Okay=this->heatingPads[0]->TempOK();

	realArray[7]=this->heatingPads[1]->GetTemperature();
	t2Okay=this->heatingPads[1]->TempOK();

	realArray[8]=this->heatingPads[2]->GetTemperature();
	t3Okay=this->heatingPads[2]->TempOK();

	this->systemState.tempsOk = t1Okay & t2Okay & t2Okay;
	realArray[10] = this->systemState.tempSP;
	realArray[11]=this->systemState.setCurrent;
}

void BurnInController::ReadNewSettings(SystemSettings newSettings) {
	if (this->CheckSettings(newSettings)) {
		this->settings = newSettings;
		EEPROM_write(this->settingsAddr, this->settings);
	} else {
		cout << message_table[ErrorRecievedMsg] << endl;
	}
}

bool BurnInController::CheckSettings(SystemSettings newSettings){
	return  (newSettings.current2 == 120 || newSettings.current2 == 60)
			&& (newSettings.setTemperature <= 85);
}

void BurnInController::ToggleHeating() {
	bool ret = false;
	for (auto pad : this->heatingPads) {
		ret=pad->ToggleHeating();
	}
	if (ret) {
		this->systemState.tempSP=this->settings.setTemperature;
		Serial.println(message_table[SettingTempToMsg] + String(this->settings.setTemperature) + "}");
	} else {
		this->systemState.tempSP=0;
		Serial.println(message_table[SetTempToZeroMsg]);
	}
}

void BurnInController::TurnOnOffHeat(HeaterState state) {
	for (auto pad : this->heatingPads) {
		pad->SetState(state);
	}
	if(state==HeaterState::On){
		this->systemState.tempSP=this->settings.setTemperature;
	}else{
		this->systemState.tempSP=0;
	}
}

void BurnInController::StartTest() {
	for (int c = 0; c <= 5; c++) {
		limitArray[c] = false;
	}
	if (this->systemState.tempsOk=true) {
		if (this->systemState.isFullCurrent) {
			this->systemState.setCurrent = FullCurrent;
			this->burnTimer.start(BurnTime150);
		} else {
			this->systemState.setCurrent = this->settings.current2;
			this->burnTimer.start(BurnTime120);
		}
		this->burnTimer.start(BurnTime150);
		this->systemState.elapsed = 0;
		this->systemState.running = true;
		this->systemState.paused = false;
		this->settingsAddr = EEPROM_write(StartAddr, this->systemState);
		this->currentSwitch.SwitchCurrent(SwitchState::On);

		unsigned long timeLeft = this->burnTimer.burnInTimeLength / 1000;
		unsigned long hrs = timeLeft / 3600;
		unsigned long mins = (timeLeft / 60) % 60;
		unsigned long seconds = (timeLeft % 60);
		String time = String(hrs) + ':' + String(mins) + ':' + String(seconds);
		Serial.println("[T]{Starting " + String(this->systemState.setCurrent) + "mA Test Now:Runtime= " + time + "}");
	} else {
		Serial.println(message_table[NotInRangeMsg]);
	}
}

void BurnInController::Reset() {
	Serial.println(message_table[ResettingSystemMsg]);
	this->currentSwitch.SwitchCurrent(SwitchState::Off);
	this->systemState.elapsed = 0;
	this->systemState.running = false;
	this->systemState.paused = false;
	this->systemState.isFullCurrent=true;
	this->systemState.setCurrent=150;
	for(int i=0;i<100;i++){
		this->realArray[i]=0;
	}
	EEPROM_write(StartAddr, this->systemState);
	wdt_disable();
	wdt_enable(WDTO_15MS);
	while(1){;}
}

void BurnInController::TestProbe() {
	Serial.println(message_table[TestingProbeMsg]);
	this->currentSwitch.SwitchCurrent(SwitchState::On);
	delay(TestProbeDelay);
	this->currentSwitch.SwitchCurrent(SwitchState::Off);
	Serial.println(message_table[TestCompleteMsg]);
}

void BurnInController::PauseTest() {
	if (!this->systemState.paused) {
		this->burnTimer.Pause();
		this->systemState.paused = true;
		this->settingsAddr = EEPROM_write(StartAddr, this->systemState);
		Serial.println(message_table[TestPausedMsg]);
		this->currentSwitch.SwitchCurrent(SwitchState::Off);
	} else {
		this->burnTimer.Continue();
		this->systemState.elapsed = this->burnTimer.elapsed;
		this->systemState.paused = false;
		this->settingsAddr = EEPROM_write(StartAddr, this->systemState);
		Serial.println(message_table[TestResumedMsg]);
		this->currentSwitch.SwitchCurrent(SwitchState::On);
		for (int c = 0; c <= 5; c++) {
			limitArray[c] = false;
		}
	}
}

void BurnInController::ToggleCurrent() {
	if(this->settings.switchingEnabled){
		//this->currentSwitch.SwitchingEnabled(true);
		CurrentMode mode=this->currentSwitch.ToggleMode();
		if(mode==CurrentMode::FullMode){
			this->systemState.isFullCurrent = true;
			this->systemState.setCurrent = FullCurrent;
			if (this->systemState.setCurrent == 150) {
				this->burnTimer.burnInTimeLength = BurnTime150;
			} else {
				this->burnTimer.burnInTimeLength = BurnTime120;
			}
		}else{
			this->systemState.setCurrent = settings.current2;
			this->systemState.isFullCurrent = false;
			this->burnTimer.burnInTimeLength=BurnTime120;
		}
		Serial.println(message_table[SetCurrentToMsg] + String(this->systemState.setCurrent) + "mA");
	}else{
		Serial.println(message_table[NoSwitchingMsg]);
	}
}

void BurnInController::HandleSerial() {
	byte inByte1 = 0;
	word buff = 0;
	while (Serial.available()) {
		inByte1 = (byte)Serial.read();
		if (((char)inByte1 == 'S') && (!this->systemState.IsRunning())) {
			this->StartTest();
		} else if ((char)inByte1 == 'R') {
			this->Reset();
		} else if (((char)inByte1 == 'T') && (!this->systemState.IsRunning())) {
			this->TestProbe();
		} else if (((char)inByte1 == 'H') && (!this->systemState.IsRunning())) {
			this->ToggleHeating();
		} else if (((char)inByte1 == 'P')  && (this->systemState.IsRunning())) {
			this->PauseTest();
		} else if (((char)inByte1 == 'C') && (!this->systemState.IsRunning())) {
			this->ToggleCurrent();
		} else if (((char)inByte1 == 'U') && (!this->systemState.IsRunning())) {
			String input = Serial.readStringUntil('\n');
			int isEnabled = input.charAt(0) - '0';  
			int ch = input.charAt(1) - '0';         
			int ct = input.charAt(2) - '0';		  
			int co = input.charAt(3) - '0';				

			int tt = input.charAt(4) - '0';			 
			int to = input.charAt(5) - '0';         

			int current = (ch * 100) + (ct * 10) + co;
			//int current2 = (ch2 * 100) + (ct2 * 10) + co2;
			int temp = (tt * 10) + (to);
			SystemSettings newSettings;
			newSettings.switchingEnabled = (bool)isEnabled;
			newSettings.current2=current;
			newSettings.setTemperature = temp;
			if (this->CheckSettings(newSettings)) {
				this->settings.Set(newSettings);
				this->currentSwitch.SwitchingEnabled(this->settings.switchingEnabled);
				for(auto heater:heatingPads){
					heater->ChangeSetpoint(this->settings.setTemperature);
				}
				if (this->settingsAddr != 0) {
					EEPROM_write(this->settingsAddr, this->settings);
				} else {
					SystemState state;
					this->settingsAddr = EEPROM_read(StartAddr, state);
					EEPROM_write(this->settingsAddr, this->settings);
				}
				Serial.println(message_table[SystemSettingsRecMsg]);
				this->settings.Print();
			} else {
				Serial.println(message_table[ErrorRecievedMsg]);
			}
		}
	}
}

void BurnInController::privateLoop() {
	if (this->IsRunning()) {
		bool done = this->burnTimer.check();
		this->systemState.elapsed = this->burnTimer.elapsed;
		this->realArray[9] = this->burnTimer.elapsed / 1000;
		if (done) {
			this->currentSwitch.SwitchCurrent(SwitchState::Off);
			this->TurnOnOffHeat(HeaterState::Off);
			this->systemState.running = false;
			this->systemState.paused = false;
			this->systemState.elapsed=0;
			EEPROM_write(StartAddr,this->systemState);
		}
	}
}
