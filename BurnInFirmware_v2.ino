// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       BurnInFirmware_v2.ino
    Created:	6/24/2021 7:47:49 AM
    Author:     SETI\AElmendo
*/

#include <EEPROM.h>
#include <ArduinoSTL.h>
#include <ArduinoComponents.h>
#include <ArduinoComponents/IO/AnalogInput.h>
#include "BurnInController.h"



using namespace components;
using namespace std;

BurnInController controller;

void setup(){
    Serial.begin(38400);
    cout << "Starting heating pads: " << endl;
    controller.Setup();
    //Serial.println("Starting Heating Pads: ");
}

void loop(){
    controller.loop();
}

void serialEvent() {
    controller.HandleSerial();
}
