#include "WirelessCTSensors.h"

//Please, have a look at Main.h for information and configuration of Arduino project

//------------------------------------------
//Function Called by timer when Tick
void WebCTSensors::PublishTick()
{

  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
  {

    //if CTSensor is not ready, then request for initial value in Home Automation
    if (clampRatios[i] != 0.0 && !_ctSensors[i].GetReady() && _ha.clampIds[i] != 0)
    {

      String completeURI;

      switch (_ha.enabled)
      {
      case 1:
        _requests[i] = String(F("&type=")) + _ha.jeedom.commandType + F("&id=") + _ha.clampIds[i];
        completeURI = completeURI + F("http") + (_ha.tls ? F("s") : F("")) + F("://") + _ha.hostname + F("/core/api/jeeApi.php?apikey=") + _ha.jeedom.apiKey + _requests[i];
        break;
      }
      //create HTTP request
      WiFiClient client;
      WiFiClientSecure clientSecure;
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!_ha.tls)
        http.begin(client, completeURI);
      else
      {
        char fpStr[41];
        clientSecure.setFingerprint(Utils::FingerPrintA2S(fpStr, _ha.fingerPrint));
        http.begin(clientSecure, completeURI);
      }

      _requestResults[i] = http.GET();
      if (_requestResults[i] == 200)
      {
        uint16_t units = 0;
        uint16_t thousands = 0;
        uint16_t millions = 0;
        uint16_t billions = 0;
        unsigned long payloadValue = 0;

        switch (_ha.enabled)
        {
        case 1:
          String payload = http.getString();

          yield();
          units = payload.substring(payload.length() >= 3 ? payload.length() - 3 : 0).toInt();
          if (payload.length() > 3)
            thousands = payload.substring(payload.length() >= 6 ? payload.length() - 6 : 0, payload.length() - 3).toInt();
          if (payload.length() > 6)
            millions = payload.substring(payload.length() >= 9 ? payload.length() - 9 : 0, payload.length() - 6).toInt();
          if (payload.length() > 9)
            billions = payload.substring(payload.length() >= 12 ? payload.length() - 12 : 0, payload.length() - 9).toInt();
          break;
        }

        http.end();

        payloadValue = billions;
        payloadValue = (payloadValue * 1000) + millions;
        payloadValue = (payloadValue * 1000) + thousands;
        payloadValue = (payloadValue * 1000) + units;

        _ctSensors[i].SetCounterFromRemote(payloadValue);
      }
    }

    //this time ctSensor is ready
    if (clampRatios[i] != 0.0 && _ctSensors[i].GetReady() && _ha.clampIds[i] != 0)
    {

      String completeURI;

      switch (_ha.enabled)
      {
      case 1:
        _requests[i] = String(F("&type=")) + _ha.jeedom.commandType + F("&id=") + _ha.clampIds[i] + F("&value=") + _ctSensors[i].GetCounterUpdated();
        completeURI = completeURI + F("http") + (_ha.tls ? F("s") : F("")) + F("://") + _ha.hostname + F("/core/api/jeeApi.php?apikey=") + _ha.jeedom.apiKey + _requests[i];
        break;
      }

      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!_ha.tls)
      {
        WiFiClient client;
        http.begin(client, completeURI);
      }
      else
      {
        WiFiClientSecure clientSecure;
        char fpStr[41];
        clientSecure.setFingerprint(Utils::FingerPrintA2S(fpStr, _ha.fingerPrint));
        http.begin(clientSecure, completeURI);
      }

      _requestResults[i] = http.GET();
      http.end();
    }
  }
}

//------------------------------------------
//Used to initialize configuration properties to default values
void WebCTSensors::SetConfigDefaultValues()
{
  clampRatios[0] = 30.0;
  clampRatios[1] = 30.0;
  clampRatios[2] = 30.0;

  noiseCancellation[0] = 0.0;
  noiseCancellation[1] = 0.0;
  noiseCancellation[2] = 0.0;

  _ha.enabled = 0;
  _ha.tls = false;
  memset(_ha.fingerPrint, 0, 20);
  _ha.hostname[0] = 0;
  _ha.clampIds[0] = 0;
  _ha.clampIds[1] = 0;
  _ha.clampIds[2] = 0;
  _ha.jeedom.apiKey[0] = 0;
  _ha.jeedom.commandType[0] = 0;
};
//------------------------------------------
//Parse JSON object into configuration properties
void WebCTSensors::ParseConfigJSON(DynamicJsonDocument &doc)
{
  //Retrocompatibility block to be removed after v3.1.5 --
  if (!doc["je"].isNull())
    _ha.enabled = doc["je"] ? 1 : 0;
  if (!doc["jt"].isNull())
    _ha.tls = doc["jt"];
  if (!doc["jh"].isNull())
    strlcpy(_ha.hostname, doc["jh"], sizeof(_ha.hostname));
  if (!doc["ct"].isNull())
    strlcpy(_ha.jeedom.commandType, doc["ct"], sizeof(_ha.jeedom.commandType));
  if (!doc["c1"].isNull())
    _ha.clampIds[0] = doc["c1"];
  if (!doc["c2"].isNull())
    _ha.clampIds[1] = doc["c2"];
  if (!doc["c3"].isNull())
    _ha.clampIds[2] = doc["c3"];
  if (!doc["jfp"].isNull())
    Utils::FingerPrintS2A(_ha.fingerPrint, doc["jfp"]);
  // --

  if (!doc["cr1"].isNull())
    clampRatios[0] = doc["cr1"];
  if (!doc["cnc1"].isNull())
    noiseCancellation[0] = doc["cnc1"];
  if (!doc["cr2"].isNull())
    clampRatios[1] = doc["cr2"];
  if (!doc["cnc2"].isNull())
    noiseCancellation[1] = doc["cnc2"];
  if (!doc["cr3"].isNull())
    clampRatios[2] = doc["cr3"];
  if (!doc["cnc3"].isNull())
    noiseCancellation[2] = doc["cnc3"];

  if (!doc[F("hae")].isNull())
    _ha.enabled = doc[F("hae")];
  if (!doc[F("hatls")].isNull())
    _ha.tls = doc[F("hatls")];
  if (!doc[F("hah")].isNull())
    strlcpy(_ha.hostname, doc["hah"], sizeof(_ha.hostname));
  if (!doc[F("hacid1")].isNull())
    _ha.clampIds[0] = doc[F("hacid1")];
  if (!doc[F("hacid2")].isNull())
    _ha.clampIds[1] = doc[F("hacid2")];
  if (!doc[F("hacid3")].isNull())
    _ha.clampIds[2] = doc[F("hacid3")];
  if (!doc["hafp"].isNull())
    Utils::FingerPrintS2A(_ha.fingerPrint, doc["hafp"]);

  if (!doc["ja"].isNull())
    strlcpy(_ha.jeedom.apiKey, doc["ja"], sizeof(_ha.jeedom.apiKey));
  if (!doc["jct"].isNull())
    strlcpy(_ha.jeedom.commandType, doc["jct"], sizeof(_ha.jeedom.commandType));
};
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebCTSensors::ParseConfigWebRequest(AsyncWebServerRequest *request)
{
  if (request->hasParam(F("cr1"), true))
    clampRatios[0] = request->getParam(F("cr1"), true)->value().toFloat();
  if (request->hasParam(F("cnc1"), true))
    noiseCancellation[0] = request->getParam(F("cnc1"), true)->value().toFloat();
  if (request->hasParam(F("cr2"), true))
    clampRatios[1] = request->getParam(F("cr2"), true)->value().toFloat();
  if (request->hasParam(F("cnc2"), true))
    noiseCancellation[1] = request->getParam(F("cnc2"), true)->value().toFloat();
  if (request->hasParam(F("cr3"), true))
    clampRatios[2] = request->getParam(F("cr3"), true)->value().toFloat();
  if (request->hasParam(F("cnc3"), true))
    noiseCancellation[2] = request->getParam(F("cnc3"), true)->value().toFloat();

  if (request->hasParam(F("hae"), true))
    _ha.enabled = request->getParam(F("hae"), true)->value().toInt();

  //if an home Automation system is enabled then get common param
  if (_ha.enabled)
  {
    if (request->hasParam(F("hatls"), true))
      _ha.tls = (request->getParam(F("hatls"), true)->value() == F("on"));
    else
      _ha.tls = false;
    if (request->hasParam(F("hah"), true) && request->getParam(F("hah"), true)->value().length() < sizeof(_ha.hostname))
      strcpy(_ha.hostname, request->getParam(F("hah"), true)->value().c_str());
    if (request->hasParam(F("hacid1"), true))
      _ha.clampIds[0] = request->getParam(F("hacid1"), true)->value().toInt();
    if (request->hasParam(F("hacid2"), true))
      _ha.clampIds[1] = request->getParam(F("hacid2"), true)->value().toInt();
    if (request->hasParam(F("hacid3"), true))
      _ha.clampIds[2] = request->getParam(F("hacid3"), true)->value().toInt();
    if (request->hasParam(F("hafp"), true))
      Utils::FingerPrintS2A(_ha.fingerPrint, request->getParam(F("hafp"), true)->value().c_str());
  }

  //Now get specific param
  switch (_ha.enabled)
  {
  case 1: //Jeedom
    char tempApiKey[48 + 1];
    //put apiKey into temporary one for predefpassword
    if (request->hasParam(F("ja"), true) && request->getParam(F("ja"), true)->value().length() < sizeof(tempApiKey))
      strcpy(tempApiKey, request->getParam(F("ja"), true)->value().c_str());
    if (request->hasParam(F("jct"), true))
      strncpy(_ha.jeedom.commandType, request->getParam(F("jct"), true)->value().c_str(), sizeof(_ha.jeedom.commandType) - 1);
    //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
    if (strcmp_P(tempApiKey, appDataPredefPassword))
      strcpy(_ha.jeedom.apiKey, tempApiKey);
    if (!_ha.hostname[0] || !_ha.jeedom.apiKey[0])
      _ha.enabled = 0;
    break;
  }

  return true;
};
//------------------------------------------
//Generate JSON from configuration properties
String WebCTSensors::GenerateConfigJSON(bool forSaveFile = false)
{
  String gc('{');

  char fpStr[60];
  gc = gc + F("\"cr1\":") + clampRatios[0];
  gc = gc + F(",\"cr2\":") + clampRatios[1];
  gc = gc + F(",\"cr3\":") + clampRatios[2];
  gc = gc + F(",\"cnc1\":") + noiseCancellation[0];
  gc = gc + F(",\"cnc2\":") + noiseCancellation[1];
  gc = gc + F(",\"cnc3\":") + noiseCancellation[2];

  gc = gc + F(",\"hae\":") + _ha.enabled;
  gc = gc + F(",\"hatls\":") + _ha.tls;
  gc = gc + F(",\"hah\":\"") + _ha.hostname + '"';
  gc = gc + F(",\"hacid1\":") + _ha.clampIds[0];
  gc = gc + F(",\"hacid2\":") + _ha.clampIds[1];
  gc = gc + F(",\"hacid3\":") + _ha.clampIds[2];
  gc = gc + F(",\"hafp\":\"") + Utils::FingerPrintA2S(fpStr, _ha.fingerPrint, forSaveFile ? 0 : ':') + '"';

  if (forSaveFile)
  {
    if (_ha.enabled == 1)
      gc = gc + F(",\"ja\":\"") + _ha.jeedom.apiKey + '"';
  }
  else
    gc = gc + F(",\"ja\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)
  gc = gc + F(",\"jct\":\"") + _ha.jeedom.commandType + '"';

  gc = gc + '}';

  return gc;
};
//------------------------------------------
//Generate JSON of application status
String WebCTSensors::GenerateStatusJSON()
{
  String gs('{');

  gs = gs + F("\"ci1\":") + _ctSensors[0].GetCurrentI();
  gs = gs + F(",\"ci2\":") + _ctSensors[1].GetCurrentI();
  gs = gs + F(",\"ci3\":") + _ctSensors[2].GetCurrentI();
  gs = gs + F(",\"cai1\":") + _ctSensors[0].GetAverageI();
  gs = gs + F(",\"cai2\":") + _ctSensors[1].GetAverageI();
  gs = gs + F(",\"cai3\":") + _ctSensors[2].GetAverageI();
  gs = gs + F(",\"c1\":") + _ctSensors[0].GetCounter();
  gs = gs + F(",\"c2\":") + _ctSensors[1].GetCounter();
  gs = gs + F(",\"c3\":") + _ctSensors[2].GetCounter();
  if (_ha.enabled)
    for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
    {
      gs = gs + F(",\"lr") + (i + 1) + F("\":\"") + _requests[i] + '"';
      gs = gs + F(",\"lrr") + (i + 1) + F("\":") + _requestResults[i];
    }

  gs += '}';

  return gs;
};
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebCTSensors::AppInit(bool reInit)
{
  //Stop Publish
  _publishTicker.detach();

  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
  {
    if (reInit)
      _ctSensors[i].Reset();

    if (!reInit)
      _requests[i].reserve(20);

    _requests[i] = "";
    _requestResults[i] = 0;
  }

  if (_ha.enabled)
    _publishTicker.attach(SEND_PERIOD, [this]() { this->_needPublish = true; });

  //"flush" serial buffer input
  while (Serial.available())
    Serial.read();

  return true;
};
//------------------------------------------
//Return HTML Code to insert into Status Web page
const uint8_t *WebCTSensors::GetHTMLContent(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
  case status:
    return (const uint8_t *)status1htmlgz;
    break;
  case config:
    return (const uint8_t *)config1htmlgz;
    break;
  default:
    return nullptr;
    break;
  };
  return nullptr;
};
//and his Size
size_t WebCTSensors::GetHTMLContentSize(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
  case status:
    return sizeof(status1htmlgz);
    break;
  case config:
    return sizeof(config1htmlgz);
    break;
  default:
    return 0;
    break;
  };
  return 0;
};
//------------------------------------------
//code to register web request answer to the web server
void WebCTSensors::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication){
    //Nothing to do
};

//------------------------------------------
//Run for timer
void WebCTSensors::AppRun()
{
  //if we receive data from ATTiny
  if (Serial.available())
  {
    char c = Serial.read();

    //don't care about \n
    if (c == '\n')
      return;
    //if \r, we need to process received datas
    else if (c == '\r')
    {

      byte i = _serialBuffer[0] - '1';
      if (i > NUMBER_OF_CTSENSOR - 1 || _serialBuffer[1] != ':')
      {
        _serialBuffer[0] = 0;
        return;
      }

      //convert text to float and pass it to newI
      float newI = atof(_serialBuffer + 2) * clampRatios[i] / 1000.0;
      newI -= noiseCancellation[i];
      if (newI <= 0.0)
        newI = 0.0;
      _ctSensors[i].NewIFromCTSensor(newI);
      _serialBuffer[0] = 0;
    }
    //else we need to put this char in _serialBuffer
    else
    {
      _serialBuffer[strlen(_serialBuffer) + 1] = 0;
      _serialBuffer[strlen(_serialBuffer)] = c;
    }
  }

  if (_needPublish)
  {
    _needPublish = false;
    Serial.println(F("PublishTick"));
    PublishTick();
  }
}

//------------------------------------------
//Constructor
WebCTSensors::WebCTSensors(char appId, String appName) : Application(appId, appName) {}
