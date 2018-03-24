#include "WirelessCTSensors.h"

//Please, have a look at Main.h for information and configuration of Arduino project

//------------------------------------------
//Function Called by timer when Tick
void WebCTSensors::SendTimerTick()
{

  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
  {

    //if CTSensor is not ready, then request for initial value in Jeedom
    if (clampRatios[i] != 0.0 && !_ctSensors[i].GetReady() && jeedom.clampIds[i] != 0)
    {

      _requests[i] = F("&type=cmd&id=");
      _requests[i] += jeedom.clampIds[i];

      String completeURI;
      completeURI = completeURI + F("http") + (jeedom.tls ? F("s") : F("")) + F("://") + jeedom.hostname + F("/core/api/jeeApi.php?apikey=") + jeedom.apiKey + _requests[i];

      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!jeedom.tls)
        http.begin(completeURI);
      else
      {
        char fpStr[41];
        http.begin(completeURI, Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint));
      }

      _requestResults[i] = http.GET();
      if (_requestResults[i] == 200)
      {
        String payload = http.getString();
        uint16_t units = 0;
        uint16_t thousands = 0;
        uint16_t millions = 0;
        uint16_t billions = 0;

        yield();
        units = payload.substring(payload.length() >= 3 ? payload.length() - 3 : 0).toInt();
        if (payload.length() > 3)
          thousands = payload.substring(payload.length() >= 6 ? payload.length() - 6 : 0, payload.length() - 3).toInt();
        if (payload.length() > 6)
          millions = payload.substring(payload.length() >= 9 ? payload.length() - 9 : 0, payload.length() - 6).toInt();
        if (payload.length() > 9)
          billions = payload.substring(payload.length() >= 12 ? payload.length() - 12 : 0, payload.length() - 9).toInt();
        http.end();

        unsigned long payloadValue = billions;
        payloadValue = (payloadValue * 1000) + millions;
        payloadValue = (payloadValue * 1000) + thousands;
        payloadValue = (payloadValue * 1000) + units;

        _ctSensors[i].SetCounterFromRemote(payloadValue);
      }
    }

    if (clampRatios[i] != 0.0 && _ctSensors[i].GetReady() && jeedom.clampIds[i] != 0)
    {

      _requests[i] = String(F("&type=")) + jeedom.commandType + F("&id=") + jeedom.clampIds[i] + F("&value=") + _ctSensors[i].GetCounterUpdated();

      String completeURI;
      completeURI = completeURI + F("http") + (jeedom.tls ? F("s") : F("")) + F("://") + jeedom.hostname + F("/core/api/jeeApi.php?apikey=") + jeedom.apiKey + _requests[i];

      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!jeedom.tls)
        http.begin(completeURI);
      else
      {
        char fpStr[41];
        http.begin(completeURI, Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint));
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

  jeedom.enabled = false;
  jeedom.tls = false;
  jeedom.hostname[0] = 0;
  jeedom.apiKey[0] = 0;
  jeedom.commandType[0] = 0;
  jeedom.clampIds[0] = 0;
  jeedom.clampIds[1] = 0;
  jeedom.clampIds[2] = 0;
  memset(jeedom.fingerPrint, 0, 20);
};
//------------------------------------------
//Parse JSON object into configuration properties
void WebCTSensors::ParseConfigJSON(JsonObject &root)
{
  if (root["cr1"].success())
    clampRatios[0] = root["cr1"];
  if (root["cnc1"].success())
    noiseCancellation[0] = root["cnc1"];
  if (root["cr2"].success())
    clampRatios[1] = root["cr2"];
  if (root["cnc2"].success())
    noiseCancellation[1] = root["cnc2"];
  if (root["cr3"].success())
    clampRatios[2] = root["cr3"];
  if (root["cnc3"].success())
    noiseCancellation[2] = root["cnc3"];

  if (root["je"].success())
    jeedom.enabled = root["je"];
  if (root["jt"].success())
    jeedom.tls = root["jt"];
  if (root["jh"].success())
    strlcpy(jeedom.hostname, root["jh"], sizeof(jeedom.hostname));
  if (root["ja"].success())
    strlcpy(jeedom.apiKey, root["ja"], sizeof(jeedom.apiKey));
  if (root["ct"].success())
    strlcpy(jeedom.commandType, root["ct"], sizeof(jeedom.commandType));
  if (root["c1"].success())
    jeedom.clampIds[0] = root["c1"];
  if (root["c2"].success())
    jeedom.clampIds[1] = root["c2"];
  if (root["c3"].success())
    jeedom.clampIds[2] = root["c3"];
  if (root["jfp"].success())
    Utils::FingerPrintS2A(jeedom.fingerPrint, root["jfp"]);
};
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebCTSensors::ParseConfigWebRequest(AsyncWebServerRequest *request)
{
  char tempApiKey[48 + 1];

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

  if (request->hasParam(F("je"), true))
    jeedom.enabled = (request->getParam(F("je"), true)->value() == F("on"));
  else
    jeedom.enabled = false;
  if (request->hasParam(F("jt"), true))
    jeedom.tls = (request->getParam(F("jt"), true)->value() == F("on"));
  else
    jeedom.tls = false;
  if (request->hasParam(F("jh"), true) && request->getParam(F("jh"), true)->value().length() < sizeof(jeedom.hostname))
    strcpy(jeedom.hostname, request->getParam(F("jh"), true)->value().c_str());
  //put apiKey into temporary one for predefpassword
  if (request->hasParam(F("ja"), true) && request->getParam(F("ja"), true)->value().length() < sizeof(tempApiKey))
    strcpy(tempApiKey, request->getParam(F("ja"), true)->value().c_str());

  if (request->hasParam(F("ct"), true))
    strncpy(jeedom.commandType, request->getParam(F("ct"), true)->value().c_str(), sizeof(jeedom.commandType) - 1);
  if (request->hasParam(F("c1"), true))
    jeedom.clampIds[0] = request->getParam(F("c1"), true)->value().toInt();
  if (request->hasParam(F("c2"), true))
    jeedom.clampIds[1] = request->getParam(F("c2"), true)->value().toInt();
  if (request->hasParam(F("c3"), true))
    jeedom.clampIds[2] = request->getParam(F("c3"), true)->value().toInt();

  if (request->hasParam(F("jfp"), true))
    Utils::FingerPrintS2A(jeedom.fingerPrint, request->getParam(F("jfp"), true)->value().c_str());

  //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
  if (strcmp_P(tempApiKey, appDataPredefPassword))
    strcpy(jeedom.apiKey, tempApiKey);

  if (!jeedom.hostname[0] || !jeedom.apiKey[0])
    jeedom.enabled = false;

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

  gc = gc + F(",\"je\":") + (jeedom.enabled ? true : false);
  gc = gc + F(",\"jt\":") + (jeedom.tls ? true : false);
  gc = gc + F(",\"jh\":\"") + jeedom.hostname + '"';
  if (forSaveFile)
    gc = gc + F(",\"ja\":\"") + jeedom.apiKey + '"';
  else
    gc = gc + F(",\"ja\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)
  gc = gc + F(",\"ct\":\"") + jeedom.commandType + '"';
  gc = gc + F(",\"c1\":") + jeedom.clampIds[0];
  gc = gc + F(",\"c2\":") + jeedom.clampIds[1];
  gc = gc + F(",\"c3\":") + jeedom.clampIds[2];
  if (forSaveFile)
    gc = gc + F(",\"jfp\":\"") + Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint) + '"';
  else
    gc = gc + F(",\"jfp\":\"") + Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint, ':') + '"';

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
  if (jeedom.enabled)
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
  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
  {
    if (reInit)
      _ctSensors[i].Reset();

    if (!reInit)
      _requests[i].reserve(20);

    _requests[i] = "";
    _requestResults[i] = 0;
  }

  if (reInit && _sendTimer.getNumTimers())
    _sendTimer.deleteTimer(0);

  if (jeedom.enabled)
    _sendTimer.setInterval(SEND_PERIOD, [this]() {
      this->SendTimerTick();
    });

  //"flush" serial buffer input
  while (Serial.available())
    Serial.read();

  return true;
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

      byte i = serialBuffer[0] - '1';
      if (i > NUMBER_OF_CTSENSOR - 1 || serialBuffer[1] != ':')
      {
        serialBuffer[0] = 0;
        return;
      }

      //convert text to float and pass it to newI
      float newI = atof(serialBuffer + 2) * clampRatios[i] / 1000.0;
      newI -= noiseCancellation[i];
      if (newI <= 0.0)
        newI = 0.0;
      _ctSensors[i].NewIFromCTSensor(newI);
      serialBuffer[0] = 0;
    }
    //else we need to put this char in serialBuffer
    else
    {
      serialBuffer[strlen(serialBuffer) + 1] = 0;
      serialBuffer[strlen(serialBuffer)] = c;
    }
  }

  //Run sendTimer
  if (jeedom.enabled)
    _sendTimer.run();
}

//------------------------------------------
//Constructor
WebCTSensors::WebCTSensors(char appId, String appName) : Application(appId, appName) {}
