#pragma once
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
"[T]{Error Recieving Msg}"
};
