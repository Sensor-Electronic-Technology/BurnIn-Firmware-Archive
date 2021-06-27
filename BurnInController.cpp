#include "BurnInController.h"

void(*resetFunc) (void) = 0; //declare reset function @ address 0

BurnInController::BurnInController() 
	:Component(),
	ledPin(2), 
	fullCurrentPin(6){	}

bool BurnInController::IsRunning() {
	return this->systemState.IsRunning();
}

void BurnInController::Setup() {
	for (int i = 0; i < 100; i++) {
		this->realArray[i] = 0;
		this->boolArray[i] = 0;
		if (i < 10) {
			this->limitArray[i] = false;
		}
	}

	Serial.println("Registered Timers");
	Probe* temp1 = new Probe(A0, A9);
	this->probes.push_back(temp1);
	RegisterChild(temp1);

	Probe* temp2 = new Probe(A1, A10);
	this->probes.push_back(temp2);
	RegisterChild(temp2);

	Probe* temp3 = new Probe(A2, A11);
	this->probes.push_back(temp3);
	RegisterChild(temp3);

	Probe* temp4 = new Probe(A3, A12);
	this->probes.push_back(temp4);
	RegisterChild(temp4);

	Probe* temp5 = new Probe(A4, A13);
	this->probes.push_back(temp5);
	RegisterChild(temp5);

	Probe* temp6 = new Probe(A5, A14);
	this->probes.push_back(temp6);
	RegisterChild(temp6);

	Serial.println("Registered Probes");

	HeatingPad* pad1 = new HeatingPad(A6, 3);
	this->heatingPads.push_back(pad1);
	RegisterChild(pad1);

	HeatingPad* pad2 = new HeatingPad(A7, 4);
	this->heatingPads.push_back(pad2);
	RegisterChild(pad2);

	HeatingPad* pad3 = new HeatingPad(A8, 5);
	this->heatingPads.push_back(pad3);
	RegisterChild(pad3);

	Serial.println("Registered HeatingPads");
	Serial.println("Reading Setting From memory");
	this->LoadFromMemory();
	this->systemState.Print();
	this->settings.Print();

	Serial.println("Taking initial measurments, please wait..");

	for (int i = 0; i < 100; i++) {
		for (auto pad : heatingPads) {
			pad->ReadTempManual();
		}

		for (auto probe : probes) {
			probe->ReadVoltage();
			probe->ReadCurrent();
		}
	}

	Serial.println("Measurements Complete,setting up timers");


	//analogReference(EXTERNAL);
	this->updateTimer.onInterval([&]() {
		this->UpdateData();
		}, 100);

	this->printTimer.onInterval([&]() {
		String buffer = "";
		for (int x = 0; x < 18; x++) {
			String val = "";
			if (x < 12) {
				val = String(realArray[x]);
			} else {
				val = String(realArray[x] * 1000);
			}
			buffer += "[R" + (String)x + "]{" + val + "}";
		}
		for (int x = 0; x <= 4; x++) {
			buffer += "[B" + (String)x + "]{" + (String)boolArray[x] + "}";
		}
		Serial.println(buffer);
		}, 400);
	RegisterChild(this->printTimer);
	RegisterChild(this->updateTimer);
	Serial.println("Timer setup complete.");
	Serial.println("Starting Program");
}

int BurnInController::WriteToMemory(int index,void* data) {
	return EEPROM_write(index, data);
}

void BurnInController::LoadFromMemory() {
	this->settingsAddr = EEPROM_read(0, this->systemState);
	EEPROM_read(this->settingsAddr, this->settings);
	for (auto pad : heatingPads) {
		pad->ChangeSetpoint(this->settings.setTemperature);
	}
}

void CheckStart() {

}

void BurnInController::UpdateData() {
	this->boolArray[0] = this->systemState.running || this->systemState.paused;
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
	realArray[10] = this->settings.setTemperature;
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
	//digitalWrite(6, HIGH);
	delay(1000);
	//digitalWrite(6, LOW);
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
			int isEnabled = ((char)Serial.read()) - '0';  //1
			int ch = ((char)Serial.read()) - '0';         //2
			int ct = ((char)Serial.read()) - '0';		  //3
			int co = ((char)Serial.read()) - '0';		  //4

			int ch2 = ((char)Serial.read()) - '0';         //2
			int ct2 = ((char)Serial.read()) - '0';		  //3
			int co2 = ((char)Serial.read()) - '0';		  //4

			int tt = ((char)Serial.read()) - '0';			  //5
			int to = ((char)Serial.read()) - '0';         //6
			int current = (ch * 100) + (ct * 10) + co;
			int current2 = (ch2 * 100) + (ct2 * 10) + co2;
			int temp = (tt * 10) + (to);
			SystemSettings newSettings;
			newSettings.switchingEnabled = (bool)isEnabled;
			newSettings.current = current2;
			newSettings.current2 = current;
			newSettings.setTemperature = temp;
			//Serial.println("Recieved: Current: ")
			cout << "Recieved: Switched?: " << isEnabled << " Current: " << current << " Current2: " << current2 << " Temp: " << temp << endl;
			if (this->CheckSettings(newSettings)) {
				this->settings = newSettings;
				if (this->settingsAddr != 0) {
					EEPROM_write(this->settingsAddr, this->settings);
				} else {
					SystemState state;
					this->settingsAddr = EEPROM_read(0, state);
					EEPROM_write(this->settingsAddr, this->settings);
				}
				Serial.println("[T]{System settings received: Current: " + String(current) + " Temp: " + String(temp) + "}");
			} else {
				Serial.println(message_table[ErrorRecievedMsg]);
			}
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
		this->realArray[9] = this->burnTimer.elapsed / 1000;
		if (done) {
			this->ledPin.low();
			this->TurnOnOffHeat(HeaterState::Off);
			this->systemState.running = false;
			this->systemState.paused = false;

			//EEPROM_write(0, this->systemState);
		}
	}
}

