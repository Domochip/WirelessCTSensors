#ifndef Main_h
#define Main_h

#include "data\status.html.gz.h"
#include "data\config.html.gz.h"
#include "data\fw.html.gz.h"
#include "data\discover.html.gz.h"

//DomoChip Informations
//------------Compile for 1M 64K SPIFFS------------
//Configuration Web Pages :
//http://IP/
//http://IP/config
//http://IP/fw

//include Application header file
#include "WirelessCTSensors.h"

#define APPLICATION_NAME "DomoChip Wireless CT Sensors"
#define APPLICATION_CLASS WebCTSensors
#define APPLICATION_VAR webCTSensors

#define VERSION_NUMBER "3.1.3"

#define MODEL "WCTSensors"

#define DEFAULT_AP_SSID "WirelessCTSensors"
#define DEFAULT_AP_PSK "PasswordCT"

#define SEND_PERIOD 60000

//Enable developper mode (fwdev webpage and SPIFFS is used)
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


