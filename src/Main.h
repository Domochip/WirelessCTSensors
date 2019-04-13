#ifndef Main_h
#define Main_h

#include <arduino.h>

//DomoChip Informations
//------------Compile for 1M 64K SPIFFS------------
//Configuration Web Pages :
//http://IP/
//http://IP/config
//http://IP/fw

//include Application header file
#include "WirelessCTSensors.h"

#define APPLICATION1_NAME "WCTSensors"
#define APPLICATION1_DESC "DomoChip Wireless CT Sensors"
#define APPLICATION1_CLASS WebCTSensors

#define VERSION_NUMBER "3.1.7"

#define DEFAULT_AP_SSID "WirelessCTSensors"
#define DEFAULT_AP_PSK "PasswordCT"

#define SEND_PERIOD 60000

//Enable developper mode (SPIFFS editor)
#define DEVELOPPER_MODE 0

//Pin RX used to receive CTSensorsReader infos @ 57600bauds
#define SERIAL_SPEED 57600

//Choose Pin used to boot in Rescue Mode
#define RESCUE_BTN_PIN 2

//construct Version text
#if DEVELOPPER_MODE
#define VERSION VERSION_NUMBER "-DEV"
#else
#define VERSION VERSION_NUMBER
#endif

#endif


