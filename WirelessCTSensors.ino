#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//Please, have a look at Main.h for information and configuration of Arduino project

#include "WirelessCTSensors.h"

String AppData1::GetJSON() {

  char fpStr[60];
  String gc;
  gc = gc + F("\"cr1\":") + clampRatios[0] + F(",\"cr2\":") + clampRatios[1] + F(",\"cr3\":") + clampRatios[2];
  gc = gc + F(",\"cnc1\":") + noiseCancellation[0] + F(",\"cnc2\":") + noiseCancellation[1] + F(",\"cnc3\":") + noiseCancellation[2];

  //there is a predefined special password (mean to keep already saved one)
  gc = gc + F(",\"je\":\"") + (jeedom.enabled ? F("on") : F("off")) + F("\",\"jt\":\"") + (jeedom.tls ? F("on") : F("off")) + F("\",\"jh\":\"") + jeedom.hostname + F("\",\"ja\":\"") + (__FlashStringHelper*)appDataPredefPassword + F("\",\"ct\":\"") + jeedom.commandType + F("\",\"c1\":") + jeedom.clampIds[0] + F(",\"c2\":") + jeedom.clampIds[1] + F(",\"c3\":") + jeedom.clampIds[2] + F(",\"jfp\":\"") + Utils::FingerPrintA2S(fpStr, jeedom.fingerPrint, ':') + '"';

  return gc;
}

bool AppData1::SetFromParameters(AsyncWebServerRequest* request, AppData1 &tempAppData1) {

  if (request->hasParam(F("cr1"), true)) tempAppData1.clampRatios[0] = request->getParam(F("cr1"), true)->value().toFloat();
  if (request->hasParam(F("cnc1"), true)) tempAppData1.noiseCancellation[0] = request->getParam(F("cnc1"), true)->value().toFloat();
  if (request->hasParam(F("cr2"), true)) tempAppData1.clampRatios[1] = request->getParam(F("cr2"), true)->value().toFloat();
  if (request->hasParam(F("cnc2"), true)) tempAppData1.noiseCancellation[1] = request->getParam(F("cnc2"), true)->value().toFloat();
  if (request->hasParam(F("cr3"), true)) tempAppData1.clampRatios[2] = request->getParam(F("cr3"), true)->value().toFloat();
  if (request->hasParam(F("cnc3"), true)) tempAppData1.noiseCancellation[2] = request->getParam(F("cnc3"), true)->value().toFloat();

  if (request->hasParam(F("je"), true)) tempAppData1.jeedom.enabled = (request->getParam(F("je"), true)->value() == F("on"));
  if (request->hasParam(F("jt"), true)) tempAppData1.jeedom.tls = (request->getParam(F("jt"), true)->value() == F("on"));
  if (request->hasParam(F("jh"), true) && request->getParam(F("jh"), true)->value().length() < sizeof(tempAppData1.jeedom.hostname)) strcpy(tempAppData1.jeedom.hostname, request->getParam(F("jh"), true)->value().c_str());
  if (request->hasParam(F("ja"), true) && request->getParam(F("ja"), true)->value().length() < sizeof(tempAppData1.jeedom.apiKey)) strcpy(tempAppData1.jeedom.apiKey, request->getParam(F("ja"), true)->value().c_str());
  if (!tempAppData1.jeedom.hostname[0] || !tempAppData1.jeedom.apiKey[0]) tempAppData1.jeedom.enabled = false;

  if (request->hasParam(F("ct"), true)) strncpy(tempAppData1.jeedom.commandType, request->getParam(F("ct"), true)->value().c_str(), sizeof(tempAppData1.jeedom.commandType) - 1);
  if (request->hasParam(F("c1"), true)) tempAppData1.jeedom.clampIds[0] = request->getParam(F("c1"), true)->value().toInt();
  if (request->hasParam(F("c2"), true)) tempAppData1.jeedom.clampIds[1] = request->getParam(F("c2"), true)->value().toInt();
  if (request->hasParam(F("c3"), true)) tempAppData1.jeedom.clampIds[2] = request->getParam(F("c3"), true)->value().toInt();

  if (request->hasParam(F("jfp"), true)) Utils::FingerPrintS2A(tempAppData1.jeedom.fingerPrint, request->getParam(F("jfp"), true)->value().c_str());

  //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
  if (!strcmp_P(tempAppData1.jeedom.apiKey, appDataPredefPassword)) strcpy(tempAppData1.jeedom.apiKey, jeedom.apiKey);

  return true;
}










//------------------------------------------
//Function Called by timer when Tick
void WebCTSensors::SendTimerTick() {

  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++) {

    //if CTSensor is not ready, then request for initial value in Jeedom
    if (_appData1->clampRatios[i] != 0.0 && !_ctSensors[i].GetReady() && _appData1->jeedom.clampIds[i] != 0) {

      _requests[i] = F("&type=cmd&id=");
      _requests[i] += _appData1->jeedom.clampIds[i];

      String completeURI;
      completeURI = completeURI + F("http") + (_appData1->jeedom.tls ? F("s") : F("")) + F("://") + _appData1->jeedom.hostname + F("/core/api/jeeApi.php?apikey=") + _appData1->jeedom.apiKey + _requests[i];

      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!_appData1->jeedom.tls) http.begin(completeURI);
      else {
        char fpStr[41];
        http.begin(completeURI, Utils::FingerPrintA2S(fpStr, _appData1->jeedom.fingerPrint));
      }

      _requestResults[i] = http.GET();
      if (_requestResults[i] == 200) {
        String payload = http.getString();
        uint16_t units = 0;
        uint16_t thousands = 0;
        uint16_t millions = 0;
        uint16_t billions = 0;

        yield();
        units = payload.substring(payload.length() >= 3 ? payload.length() - 3 : 0).toInt();
        if (payload.length() > 3) thousands = payload.substring(payload.length() >= 6 ? payload.length() - 6 : 0, payload.length() - 3).toInt();
        if (payload.length() > 6) millions = payload.substring(payload.length() >= 9 ? payload.length() - 9 : 0, payload.length() - 6).toInt();
        if (payload.length() > 9) billions = payload.substring(payload.length() >= 12 ? payload.length() - 12 : 0, payload.length() - 9).toInt();
        http.end();

        unsigned long payloadValue = billions;
        payloadValue = (payloadValue * 1000) + millions;
        payloadValue = (payloadValue * 1000) + thousands;
        payloadValue = (payloadValue * 1000) + units;

        _ctSensors[i].SetCounterFromRemote(payloadValue);
      }
    }


    if (_appData1->clampRatios[i] != 0.0 && _ctSensors[i].GetReady() && _appData1->jeedom.clampIds[i] != 0) {

      _requests[i] = String(F("&type=")) + _appData1->jeedom.commandType + F("&id=") + _appData1->jeedom.clampIds[i] + F("&value=") + _ctSensors[i].GetCounterUpdated();

      String completeURI;
      completeURI = completeURI + F("http") + (_appData1->jeedom.tls ? F("s") : F("")) + F("://") + _appData1->jeedom.hostname + F("/core/api/jeeApi.php?apikey=") + _appData1->jeedom.apiKey + _requests[i];

      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!_appData1->jeedom.tls) http.begin(completeURI);
      else {
        char fpStr[41];
        http.begin(completeURI, Utils::FingerPrintA2S(fpStr, _appData1->jeedom.fingerPrint));
      }

      _requestResults[i] = http.GET();
      http.end();
    }
  }
}

//------------------------------------------
//return CTSensors Status
String WebCTSensors::GetStatus() {

  String statusJSON('{');
  statusJSON = statusJSON + F("\"ci1\":") + _ctSensors[0].GetCurrentI() + F(",\"ci2\":") + _ctSensors[1].GetCurrentI() + F(",\"ci3\":") + _ctSensors[2].GetCurrentI();
  statusJSON = statusJSON + F(",\"cai1\":") + _ctSensors[0].GetAverageI() + F(",\"cai2\":") + _ctSensors[1].GetAverageI() + F(",\"cai3\":") + _ctSensors[2].GetAverageI();
  statusJSON = statusJSON + F(",\"c1\":") + _ctSensors[0].GetCounter() + F(",\"c2\":") + _ctSensors[1].GetCounter() + F(",\"c3\":") + _ctSensors[2].GetCounter();
  if (_appData1->jeedom.enabled) for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++) {
      statusJSON = statusJSON + F(",\"lr") + (i + 1) + F("\":\"") + _requests[i] + F("\",\"lrr") + (i + 1) + F("\":") + _requestResults[i];
    }
  statusJSON += '}';

  return statusJSON;
}

//------------------------------------------
//Process byte that coming from Serial
void WebCTSensors::Process(char c) {
  //don't care about \n
  if (c == '\n') return;
  //if \r, we need to process received datas
  else if (c == '\r') {

    byte i = serialBuffer[0] - '1';
    if (i > NUMBER_OF_CTSENSOR - 1 || serialBuffer[1] != ':') {
      serialBuffer[0] = 0;
      return;
    }

    //convert text to float and pass it to newI
    float newI = atof(serialBuffer + 2) * _appData1->clampRatios[i] / 1000.0;
    newI -= _appData1->noiseCancellation[i];
    if (newI <= 0.0) newI = 0.0;
    _ctSensors[i].NewIFromCTSensor(newI);
    serialBuffer[0] = 0;
  }
  //else we need to complete serialBuffer
  else {
    serialBuffer[strlen(serialBuffer) + 1] = 0;
    serialBuffer[strlen(serialBuffer)] = c;
  }
}




//------------------------------------------
//Function to initiate WebCTSensors with Config
void WebCTSensors::Init(AppData1 &appData1) {

  Serial.print(F("Start CTSensors"));

  _appData1 = &appData1;

  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++) {
    _requests[i].reserve(20);
    _requests[i] = "";
    _requestResults[i] = 0;
  }

  if (_appData1->jeedom.enabled) _sendTimer.setInterval(SEND_PERIOD, [this]() {
    this->SendTimerTick();
  });

  //"flush" serial buffer input
  while (Serial.available()) Serial.read();

  Serial.println(F(" : OK"));
}

//------------------------------------------
void WebCTSensors::InitWebServer(AsyncWebServer &server) {

  server.on("/gs1", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, F("text/json"), GetStatus());
  });
}

//------------------------------------------
//Run for timer
void WebCTSensors::Run() {

  //if we receive data from ATTiny
  if (Serial.available()) Process(Serial.read());

  if (_appData1->jeedom.enabled) _sendTimer.run();
}


