#ifndef WebCTSensors_h
#define WebCTSensors_h

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Main.h"
#include "CTSensor.h"
#include "SimpleTimer.h"
#include "src\Utils.h"


//define the number of CTSensor
#define NUMBER_OF_CTSENSOR 3

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

class AppData1 {

  public:
    float clampRatios[3] = {0.0, 0.0, 0.0};
    float noiseCancellation[3] = {0.0, 0.0, 0.0};

    typedef struct {
      bool enabled = false;
      bool tls = false;
      char hostname[64 + 1] = {0};
      char apiKey[48 + 1] = {0};
      char commandType[10 + 1] = {0};
      int clampIds[3] = {0, 0, 0};

      byte fingerPrint[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    } Jeedom;
    Jeedom jeedom;

    void SetDefaultValues() {

      clampRatios[0] = 30.0;
      clampRatios[1] = 30.0;
      clampRatios[2] = 30.0;

      noiseCancellation[0] = 0.0;
      noiseCancellation[1] = 0.0;
      noiseCancellation[2] = 0.0;

      jeedom.enabled = false;
      jeedom.tls = false;
      jeedom.hostname[0] = 0;
      jeedom.apiKey[0] = 0;
      jeedom.commandType[0] = 0;
      jeedom.clampIds[0] = 0;
      jeedom.clampIds[1] = 0;
      jeedom.clampIds[2] = 0;
      memset(jeedom.fingerPrint, 0, 20);
    }

    String GetJSON();

    bool SetFromParameters(AsyncWebServerRequest* request, AppData1 &tempAppData);
};

class WebCTSensors {

  private:
    AppData1* _appData1;
    CTSensor _ctSensors[NUMBER_OF_CTSENSOR];
    SimpleTimer _sendTimer;

    char serialBuffer[12] = {0};

    //for returning Status
    String _requests[NUMBER_OF_CTSENSOR];
    int _requestResults[NUMBER_OF_CTSENSOR];

    void SendTimerTick();
    String GetStatus();
    void Process(char c);

  public:
    void Init(AppData1 &appData1);
    void InitWebServer(AsyncWebServer &server);
    void Run();
};

#endif
