#pragma once
#include <Arduino.h>
#include <ArduinoSTL.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

template <class T> int EEPROM_write(int addr, const T& value) {
    const byte* p = (const byte*)(const void*)&value;
    int newAddr;
    for (newAddr = 0; newAddr < sizeof(value); newAddr++) {
        EEPROM.write(addr++, *p++);
    }
    return newAddr;
}//End write any value/type

template <class T> int EEPROM_read(int addr, T& value) {
    byte* p = (byte*)(void*)&value;
    int newAddr;
    for (newAddr = 0; newAddr < sizeof(value); newAddr++)
        *p++ = EEPROM.read(addr++);
    return newAddr;
}//End read any value/type

#define FullCurrent     150

//LED Control
#define LedPin			2
#define FullPin			6

//Heaters
#define heatPin1		3
#define heatPin2		4
#define heatPin3		5

//Probes
#define Probe1			A0
#define Probe2			A1
#define Probe3			A2
#define Probe4			A3
#define Probe5			A4
#define Probe6			A5

//Temp Sensors
#define TempPin1		A6
#define TempPin2		A7
#define TempPin3		A8

//Time Constants
#define BurnTime120 	        (unsigned long)72000000 //20Hrs
#define BurnTime60		        BurnTime120
#define BurnTime150 	        (unsigned long)25200000  //7hrs
#define COMPERIOD		        200
#define UPDATEPERIOD	        50
#define ResetDelay              250
#define TestProbeDelay          1000

//Duty Cycle Constants
#define tempHLimit			    90
#define tempLLimit			    0
#define tempDeviation		    10  //% error allowed
#define tempOffset			    (0)
#define Kp					    20
#define Ki					    0.1
#define Kd					    0
#define maxDuty				    100.0f
#define minDuty                 0.0f
#define hiDuty				    98
#define lowDuty				    2
#define readDelay			    5
#define dutyTime			    500
#define outputPeriod		    1000
#define runtime				    50
#define DefaultSetPoint		    85
#define iTermMax                100.0f
#define ITermMin                0.0f
#define dTermFact               0.05f

//Temp Sensor Constants
#define tempReadTime            50
#define tempConversion          500
#define tempfilter              .1f

//Probe Constants
#define MaxVoltageR			105
#define MaxVoltageS			68
#define ZeroCurrent			2.5
#define CurrentVoltage		5
#define fWeight				.1

//Message Indexes
#define BurnInCompleteMsg       0
#define NotInRangeMsg           1
#define ResettingSystemMsg      2
#define TestingProbeMsg         3
#define TestCompleteMsg         4
#define SettingTempToMsg        5
#define SetTempToZeroMsg        6
#define TestPausedMsg           7
#define TestResumedMsg          8
#define SetCurrentToMsg         9
#define NoSwitchingMsg          10
#define SystemSettingsRecMsg    11
#define ErrorRecievedMsg        12

#define FimwareInitMsg          13
#define InternalMemMsg          14
#define MemCheckFinishedMsg     15
#define PrintMemMsg             16
#define SettingIOMsg            17
#define TakingMeasMsg           18
#define IOCompleteMsg           19
#define TimerInitMsg            20
#define TimerCompleteMsg        21
#define FirmwareCompleteMsg     22
#define SystemSettingsRecMsg    23

const char* const message_table[] PROGMEM = {
"[T]{Burn-In Complete.  Heaters Off}\n[T]{Reset before starting next Burn-In}", 
"[T]{Temperatures are not in range}", 
"[T]{Resetting Device}",
"[T]{Testing Probe Contact - Probe on for 1000msec}", 
"[T]{Testing Complete}", 
"[T]{Setting Temperature to ",
"[T]{Setting Temperature to 0}",
"[T]{Device Paused}",
"[T]{Device Resumed}",
"[T]{Setting Current to ",
"[T]{Switching Not Installed On This Station}",
"[T]{Setting Recieved}",
"[T]{Error Recieving Msg}",
"[T]{Starting Firmware Initialization}",
"[T]{Checking internal memory}",
"[T]{Memory check finished}",
"[T]{System State and System Settings:}",
"[T]{Setting up IO}",
"[T]{Taking initial measurements, please wait...}",
"[T]{IO Setup Complete}",
"[T]{Initializing Timers}",
"[T]{Timer Initialization Complete}",
"[T]{Firmware Initialization Complete}",
"[T]{New System Settings Recieved:}"
};