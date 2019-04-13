#ifndef WebCTSensors_h
#define WebCTSensors_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Main.h"
#include "base\Utils.h"
#include "base\Application.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

#include "data\status1.html.gz.h"
#include "data\config1.html.gz.h"

#include <ESP8266HTTPClient.h>
#include "CTSensor.h"
#include "SimpleTimer.h"

//define the number of CTSensor
#define NUMBER_OF_CTSENSOR 3

class WebCTSensors : public Application
{
private:
  float clampRatios[3] = {0.0, 0.0, 0.0};
  float noiseCancellation[3] = {0.0, 0.0, 0.0};

  typedef struct
  {
    char apiKey[48 + 1] = {0};
    char commandType[10 + 1] = {0};
  } Jeedom;

  typedef struct
  {
    byte enabled = 0; //0 : no HA; 1 : Jeedom; 2 : ...
    bool tls = false;
    byte fingerPrint[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char hostname[64 + 1] = {0};
    int clampIds[3] = {0, 0, 0};
    Jeedom jeedom;
  } HomeAutomation;
  HomeAutomation ha;

  CTSensor _ctSensors[NUMBER_OF_CTSENSOR];
  SimpleTimer _sendTimer;
  char serialBuffer[12] = {0};

  String _requests[NUMBER_OF_CTSENSOR];
  int _requestResults[NUMBER_OF_CTSENSOR];

  void SendTimerTick();

  void SetConfigDefaultValues();
  void ParseConfigJSON(DynamicJsonDocument &doc);
  bool ParseConfigWebRequest(AsyncWebServerRequest *request);
  String GenerateConfigJSON(bool forSaveFile);
  String GenerateStatusJSON();
  bool AppInit(bool reInit);
  const uint8_t* GetHTMLContent(WebPageForPlaceHolder wp);
  size_t GetHTMLContentSize(WebPageForPlaceHolder wp);
  void AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void AppRun();

  String GetStatus();

public:
  WebCTSensors(char appId, String fileName);
};

#endif
