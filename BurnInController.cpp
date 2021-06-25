#include "BurnInController.h"

BurnInController::BurnInController() :Component(),ledPin(6), fullCurrentPin(2){
	RegisterChild(this->printTimer);
	this->printTimer.onInterval([&]() {
		int count = 0;
		for (auto pad : heatingPads) {
			cout << "T" << count << ": " << pad->GetTemperature() << " ,";
		}
		cout << endl;
		count = 0;
		for (auto probe : probes) {
			cout << "V" << count << ": " << probe->GetVoltage() << " ,";
		}
		cout << endl;
		}, 250);
}

bool BurnInController::IsRunning() {
	return this->systemState.running || this->systemState.paused;
}

void BurnInController::Setup() {
	for (int i = 0; i < 6; i++) {
		Probe* probe = new Probe(i, i + 9);
		this->probes.push_back(probe);
		RegisterChild(probe);
	}

	for (int i = 3; i < 6; i++) {
		HeatingPad* pad = new HeatingPad(i + 3, i);
		this->heatingPads.push_back(pad);
		RegisterChild(pad);
	}
}

int BurnInController::WriteToMemory(int index,void* data) {
	return EEPROM_write(index, data);
}

void BurnInController::LoadFromMemory() {
	this->settingsAddr = EEPROM_read(0, this->systemState);
	EEPROM_read(this->settingsAddr, this->settings);
}

void CheckStart() {

}

void BurnInController::UpdateData() {
	this->boolArray[0] = digitalRead(LedPin) || this->systemState.paused;
	boolArray[1] = digitalRead(heatPin1);
	boolArray[2] = digitalRead(heatPin2);
	boolArray[3] = digitalRead(heatPin3);

	realArray[0] = this->probes[0]->GetVoltage();
	realArray[1] = this->probes[1]->GetVoltage();
	realArray[2] = this->probes[2]->GetVoltage();
	realArray[3] = this->probes[3]->GetVoltage();
	realArray[4] = this->probes[4]->GetVoltage();
	realArray[5] = this->probes[5]->GetVoltage();
	this->systemState.tempsOk = true;
	for (int i = 0; i < 3; i++) {
		realArray[i+6] = this->heatingPads[i]->GetTemperature();
		this->systemState.tempsOk &= this->heatingPads[i]->TempOK();
	}
	//realArray[6] = this->heatingPads[0]->GetTemperature();
	//realArray[7] = this->heatingPads[1]->GetTemperature();
	//realArray[8] = this->heatingPads[2]->GetTemperature();
	
	realArray[10] = this->settings.setTemperature;
	//realArray[11] = //this->settings.;
	realArray[12] = this->probes[0]->GetCurrent();
	realArray[13] = this->probes[1]->GetCurrent();
	realArray[14] = this->probes[2]->GetCurrent();
	realArray[15] = this->probes[3]->GetCurrent();
	realArray[16] = this->probes[4]->GetCurrent();
	realArray[17] = this->probes[5]->GetCurrent();

}

void BurnInController::ReadNewSettings(SystemSettings newSettings) {
	if (this->CheckSettings(newSettings)) {
		this->settings = newSettings;
		EEPROM_write(this->settingsAddr, this->settings);
	} else {
		//print error message
		cout << message_table[ErrorRecievedMsg] << endl;
	}
}

bool BurnInController::CheckSettings(SystemSettings newSettings){
	return (newSettings.current == 120 || newSettings.current == 60)
		&& (newSettings.current2 == 120 || newSettings.current2 == 150)
		&& (newSettings.setTemperature <= 85);
}

void BurnInController::ToggleHeating() {
	bool ret = false;
	for (auto pad : this->heatingPads) {
		ret=pad->ToggleHeating();
	}
	if (ret) {
		Serial.println(message_table[SettingTempToMsg] + String(this->settings.setTemperature) + "}");
	} else {
		Serial.println(message_table[SetTempToZeroMsg]);
	}
}

void BurnInController::TurnOnOffHeat(HeaterState state) {
	for (auto pad : this->heatingPads) {
		pad->SetState(state);
	}
}



void BurnInController::StartTest() {
	for (int c = 0; c <= 5; c++) {
		limitArray[c] = false;
	}

	if (this->systemState.tempsOk) {
		//for (int i = 0; i < 6; i++) {
		//	aValue = analogRead(currentSen[i].aPin); delay(1);
		//	currentSen[i].zeroValue = (((float)(aValue) * 5.0) / 1024.0);
		//}
		//digitalWrite(ledPin, HIGH);
		this->ledPin.high();
		if (this->fullCurrentOn) {
			if (this->settings.current == 150) {
				this->burnTimer.start(BurnTime150);
			} else {
				this->burnTimer.start(BurnTime120);
			}
			this->systemState.setCurrent = this->settings.current;
		} else {
			if (this->settings.current2 == 120) {
				this->burnTimer.start(BurnTime120);
			} else {
				this->burnTimer.start(BurnTime60);
			}
			this->systemState.setCurrent = this->settings.current2;
		}

		this->systemState.elapsed = 0;
		this->systemState.running = true;
		this->systemState.is150On = true;
		this->systemState.paused = false;
		this->settingsAddr = EEPROM_write(0, this->systemState);

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
	this->systemState.elapsed = 0;
	this->systemState.running = false;
	this->systemState.is150On = this->fullCurrentOn;
	this->systemState.paused = false;
	EEPROM_write(0, this->systemState);
	delay(500);
	resetFunc();
}

void BurnInController::TestProbe() {
	Serial.println(message_table[TestingProbeMsg]);
	this->ledPin.high();
	delay(1000);
	this->ledPin.low();
	Serial.println(message_table[TestCompleteMsg]);
}

void BurnInController::PauseTest() {
	if (!this->systemState.paused) {
		this->burnTimer.Pause();
		this->systemState.paused = true;
		this->settingsAddr = EEPROM_write(0, this->systemState);
		Serial.println(message_table[TestPausedMsg]);
		this->ledPin.low();
	} else {
		this->burnTimer.Continue();
		//burnInStartTime = burnInStartTime + (millis() - burnInPauseTime);
		//runningTest.elapsed = burnInTimeMillis;
		//runningTest.paused = false;
		this->systemState.elapsed = this->burnTimer.elapsed;
		this->systemState.paused = false;
		this->settingsAddr = EEPROM_write(0, this->systemState);
		Serial.println(message_table[TestResumedMsg]);
		this->ledPin.high();
		for (int c = 0; c <= 5; c++) {
			limitArray[c] = false;
		}
	}
}

void BurnInController::ToggleCurrent() {
	if (!this->fullCurrentOn) {
		Serial.println(message_table[SetCurrentToMsg] + String(this->settings.current) + "mA");
		this->systemState.is150On = true;
		this->systemState.setCurrent = this->settings.current;
		if (this->settings.current == 150) {
			this->burnTimer.burnInTimeLength = BurnTime150;
		} else {
			this->burnTimer.burnInTimeLength = BurnTime120;
		}
		this->settingsAddr = EEPROM_write(0, this->systemState);
	} else {
		if (this->settings.switchingEnabled) {

			Serial.println(message_table[SetCurrentToMsg] + String(settings.current2) + "mA");
			this->systemState.setCurrent = settings.current2;
			this->systemState.is150On = false;
			this->fullCurrentOn = false;
			if (this->systemState.setCurrent == 120) {
				burnTimer.burnInTimeLength = BurnTime120;
			} else {
				burnTimer.burnInTimeLength = BurnTime60;
			}
			this->settingsAddr = EEPROM_write(0, this->systemState);
		} else {
			Serial.println(message_table[NoSwitchingMsg]);
		}
	}
}

void BurnInController::HandleSerial() {
	byte inByte1 = 0;
	word buff = 0;
	while (Serial.available()) {
		// get the new byte:
		inByte1 = (byte)Serial.read();
		if (((char)inByte1 == 'S') && (!this->systemState.IsRunning())) {
			this->StartTest();
		} else if ((char)inByte1 == 'R') {
			this->Reset();
		} else if (((char)inByte1 == 'T') && (!this->systemState.IsRunning())) {
			this->TestProbe();
		} else if (((char)inByte1 == 'H') && (!this->systemState.IsRunning())) {
			this->ToggleHeating();
		} else if ((char)inByte1 == 'P') {
			this->PauseTest();
		} else if (((char)inByte1 == 'C') && (!this->systemState.IsRunning())) {
			this->ToggleCurrent();
		} else if (((char)inByte1 == 'U') && (!this->systemState.IsRunning())) {
			int isEnabled = ((char)Serial.read()) - '0';  //0
			//int aversion = ((char)Serial.read()) - '0';   //1
			int ch = ((char)Serial.read()) - '0';         //2
			int ct = ((char)Serial.read()) - '0';		  //3
			int co = ((char)Serial.read()) - '0';		  //4
			int tt = ((char)Serial.read()) - '0';			  //5
			int to = ((char)Serial.read()) - '0';         //6
			int current = (ch * 100) + (ct * 10) + co;
			int temp = (tt * 10) + (to);
			SystemSettings newSettings;
			newSettings.switchingEnabled = (bool)isEnabled;
			newSettings.current2 = current;
			newSettings.setTemperature = temp;
			if (this->CheckSettings()) {
				this->settings = newSettings;
				EEPROM_write(this->settingsAddr, this->settings);
				Serial.println("[T]{System settings received: Current: " + String(current) + " Temp: " + String(temp) + "}");
			} else {
				Serial.println(message_table[ErrorRecievedMsg]);
			}

			//this->settings.switchingEnabled = (bool)isEnabled;
			//this->settings.current2 = current;
			//this->settings.setTemperature = temp;
			//

			//systemSettings.switchingEnabled = (bool)isEnabled;
			//systemSettings.current2 = current;
			//systemSettings.temperature = temp;
			//systemSettings.analogVersion = aversion;
			//if (checkMemoryData(false)) {
			//	EEPROM_write(settingsAddr, systemSettings);
			//	Serial.println("[T]{System settings received: Current: " + String(current) + " Temp: " + String(temp) + "}");
			//} else {
			//	Serial.println("[T]{Error receiving one or more settings}");
			//}
		} else if ((char)inByte1 == 'G') {
			//sendComs_v2();
		}
	}
}

void BurnInController::privateLoop() {
	if (this->IsRunning()) {
		bool done = this->burnTimer.check();
		this->systemState.elapsed = this->burnTimer.elapsed;
		realArray[9] = this->burnTimer.elapsed / 1000;
		if (done) {
			this->ledPin.low();
			this->ToggleHeating();
			this->TurnOnOffHeat(HeaterState::Off);
			this->systemState.running = false;
			this->systemState.paused = false;
			
		} else {

		}
	}
}

