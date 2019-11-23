#ifndef WebCTSensors_h
#define WebCTSensors_h

#include "Main.h"
#include "base\Utils.h"
#include "base\MQTTMan.h"
#include "base\Application.h"

const char appDataPredefPassword[] PROGMEM = "ewcXoCt4HHjZUvY1";

#include "data\status1.html.gz.h"
#include "data\config1.html.gz.h"

#include <ESP8266HTTPClient.h>
#include <Ticker.h>
#include "CTSensor.h"

//define the number of CTSensor
#define NUMBER_OF_CTSENSOR 3

class WebCTSensors : public Application
{
private:
  float clampRatios[3] = {0.0, 0.0, 0.0};
  float noiseCancellation[3] = {0.0, 0.0, 0.0};

#define HA_MQTT_GENERIC 0

  typedef struct
  {
    byte type = HA_MQTT_GENERIC;
    uint32_t port = 1883;
    char username[128 + 1] = {0};
    char password[150 + 1] = {0};
    struct
    {
      char baseTopic[64 + 1] = {0};
    } generic;
  } MQTT;

#define HA_PROTO_DISABLED 0
#define HA_PROTO_MQTT 1

  typedef struct
  {
    byte protocol = HA_PROTO_DISABLED;
    char hostname[64 + 1] = {0};
    uint16_t uploadPeriod = 60;
    MQTT mqtt;
  } HomeAutomation;

  HomeAutomation _ha;
  int _haSendResult = 1;
  WiFiClient _wifiClient;

  CTSensor _ctSensors[NUMBER_OF_CTSENSOR];
  char _serialBuffer[12] = {0};

  bool _needPublish = false;
  Ticker _publishTicker;
  
  MQTTMan _mqttMan;

  void mqttConnectedCallback(MQTTMan *mqttMan, bool firstConnection);
  void mqttCallback(char *topic, uint8_t *payload, unsigned int length);
  void publishTick();

  void setConfigDefaultValues();
  void parseConfigJSON(DynamicJsonDocument &doc);
  bool parseConfigWebRequest(AsyncWebServerRequest *request);
  String generateConfigJSON(bool forSaveFile);
  String generateStatusJSON();
  bool appInit(bool reInit);
  const uint8_t *getHTMLContent(WebPageForPlaceHolder wp);
  size_t getHTMLContentSize(WebPageForPlaceHolder wp);
  void appInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication);
  void appRun();

public:
  WebCTSensors(char appId, String fileName);
};

#endif
