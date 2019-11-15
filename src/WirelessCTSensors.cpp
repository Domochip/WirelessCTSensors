#include "WirelessCTSensors.h"

//Please, have a look at Main.h for information and configuration of Arduino project

//------------------------------------------
// subscribe to MQTT topic after connection
void WebCTSensors::MqttConnectedCallback(MQTTMan *mqttMan, bool firstConnection)
{
  //Subscribe to needed topic
  //prepare topic subscription
  String subscribeTopic = _ha.mqtt.generic.baseTopic;
  //check for final slash
  if (subscribeTopic.length() && subscribeTopic.charAt(subscribeTopic.length() - 1) != '/')
    subscribeTopic += '/';

  switch (_ha.mqtt.type)
  {
  case HA_MQTT_GENERIC:
    //complete the topic
    subscribeTopic += F("$CTNumber$");
    break;
  }

  //Replace placeholders
  if (subscribeTopic.indexOf(F("$sn$")) != -1)
  {
    char sn[9];
    sprintf_P(sn, PSTR("%08x"), ESP.getChipId());
    subscribeTopic.replace(F("$sn$"), sn);
  }

  if (subscribeTopic.indexOf(F("$mac$")) != -1)
    subscribeTopic.replace(F("$mac$"), WiFi.macAddress());

  if (subscribeTopic.indexOf(F("$model$")) != -1)
    subscribeTopic.replace(F("$model$"), APPLICATION1_NAME);

  String thisSensorTopic;

  //for each CT sensors
  for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
  {
    //wich is enabled
    if (clampRatios[i] != 0.0)
    {
      thisSensorTopic = subscribeTopic;

      if (thisSensorTopic.indexOf(F("$CTNumber$")) != -1)
        thisSensorTopic.replace(F("$CTNumber$"), String(i + 1));

      mqttMan->subscribe(thisSensorTopic.c_str());
    }
  }
}

//------------------------------------------
//Callback used when an MQTT message arrived
void WebCTSensors::MqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  //parse CTNumber
  byte ctNumber = topic[strlen(topic) - 1] - '1';

  //check that counter is not yet initialized
  if (ctNumber >= NUMBER_OF_CTSENSOR || _ctSensors[ctNumber].GetReady() || length < 1)
    return;

  unsigned long initialCounter = 0;
  for (byte i = 0; i < length; i++)
    //check that payload is only numbers
    if (isdigit(payload[i]))
      initialCounter = initialCounter * 10 + (payload[i] - '0'); // convert payload
    else
      return; //bad payload

  //initialize counter
  _ctSensors[ctNumber].SetCounterFromRemote(initialCounter);
}

//------------------------------------------
//Function Called by timer when Tick
void WebCTSensors::PublishTick()
{
  //if Home Automation upload not enabled then return
  if (_ha.protocol == HA_PROTO_DISABLED)
    return;

  //----- MQTT Protocol configured -----
  if (_ha.protocol == HA_PROTO_MQTT)
  {
    //if we are connected
    if (m_mqttMan.connected())
    {
      //At this point, MQTT is enabled and connected, first PublishTick occurs after ha upload period
      //initial counter values should have been received through MqttCallback
      //if a counter is not ready/initialized, we assume that there is no value to start with
      //for each CT sensor
      for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
        if (clampRatios[i] != 0.0 && !_ctSensors[i].GetReady()) //if it's enabled and not ready
          _ctSensors[i].SetCounterFromRemote(0);                //initialize it with 0

      //prepare topic
      String completeTopic = _ha.mqtt.generic.baseTopic;

      //check for final slash
      if (completeTopic.length() && completeTopic.charAt(completeTopic.length() - 1) != '/')
        completeTopic += '/';

      switch (_ha.mqtt.type)
      {
      case HA_MQTT_GENERIC:
        //complete the topic
        completeTopic += F("$CTNumber$");
        break;
      }

      //Replace placeholders
      if (completeTopic.indexOf(F("$sn$")) != -1)
      {
        char sn[9];
        sprintf_P(sn, PSTR("%08x"), ESP.getChipId());
        completeTopic.replace(F("$sn$"), sn);
      }

      if (completeTopic.indexOf(F("$mac$")) != -1)
        completeTopic.replace(F("$mac$"), WiFi.macAddress());

      if (completeTopic.indexOf(F("$model$")) != -1)
        completeTopic.replace(F("$model$"), APPLICATION1_NAME);

      _haSendResult = true;
      String thisSensorTopic;

      //for each CT sensors
      for (byte i = 0; i < NUMBER_OF_CTSENSOR && _haSendResult; i++)
      {
        if (clampRatios[i] != 0.0) //which is enabled
        {
          //copy completeTopic in order to "complete" it ...
          thisSensorTopic = completeTopic;

          if (thisSensorTopic.indexOf(F("$CTNumber$")) != -1)
            thisSensorTopic.replace(F("$CTNumber$"), String(i + 1));

          //send USING RETAIN
          _haSendResult = m_mqttMan.publish(thisSensorTopic.c_str(), String(_ctSensors[i].GetCounterUpdated()).c_str(), true); //USING RETAIN
        }
      }
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

  _ha.protocol = HA_PROTO_DISABLED;
  _ha.hostname[0] = 0;
  _ha.uploadPeriod = 60;

  _ha.mqtt.type = HA_MQTT_GENERIC;
  _ha.mqtt.port = 1883;
  _ha.mqtt.username[0] = 0;
  _ha.mqtt.password[0] = 0;
  _ha.mqtt.generic.baseTopic[0] = 0;
};
//------------------------------------------
//Parse JSON object into configuration properties
void WebCTSensors::ParseConfigJSON(DynamicJsonDocument &doc)
{
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

  if (!doc[F("haproto")].isNull())
    _ha.protocol = doc[F("haproto")];
  if (!doc[F("hahost")].isNull())
    strlcpy(_ha.hostname, doc[F("hahost")], sizeof(_ha.hostname));
  if (!doc[F("haupperiod")].isNull())
    _ha.uploadPeriod = doc[F("haupperiod")];

  if (!doc[F("hamtype")].isNull())
    _ha.mqtt.type = doc[F("hamtype")];
  if (!doc[F("hamport")].isNull())
    _ha.mqtt.port = doc[F("hamport")];
  if (!doc[F("hamu")].isNull())
    strlcpy(_ha.mqtt.username, doc[F("hamu")], sizeof(_ha.mqtt.username));
  if (!doc[F("hamp")].isNull())
    strlcpy(_ha.mqtt.password, doc[F("hamp")], sizeof(_ha.mqtt.password));

  if (!doc[F("hamgbt")].isNull())
    strlcpy(_ha.mqtt.generic.baseTopic, doc[F("hamgbt")], sizeof(_ha.mqtt.generic.baseTopic));
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

  //Parse HA protocol
  if (request->hasParam(F("haproto"), true))
    _ha.protocol = request->getParam(F("haproto"), true)->value().toInt();

  //if an home Automation protocol has been selected then get common param
  if (_ha.protocol != HA_PROTO_DISABLED)
  {
    if (request->hasParam(F("hahost"), true) && request->getParam(F("hahost"), true)->value().length() < sizeof(_ha.hostname))
      strcpy(_ha.hostname, request->getParam(F("hahost"), true)->value().c_str());
    if (request->hasParam(F("haupperiod"), true))
      _ha.uploadPeriod = request->getParam(F("haupperiod"), true)->value().toInt();
  }

  //Now get specific param
  switch (_ha.protocol)
  {
  case HA_PROTO_MQTT:

    if (request->hasParam(F("hamtype"), true))
      _ha.mqtt.type = request->getParam(F("hamtype"), true)->value().toInt();
    if (request->hasParam(F("hamport"), true))
      _ha.mqtt.port = request->getParam(F("hamport"), true)->value().toInt();
    if (request->hasParam(F("hamu"), true) && request->getParam(F("hamu"), true)->value().length() < sizeof(_ha.mqtt.username))
      strcpy(_ha.mqtt.username, request->getParam(F("hamu"), true)->value().c_str());
    char tempPassword[64 + 1] = {0};
    //put MQTT password into temporary one for predefpassword
    if (request->hasParam(F("hamp"), true) && request->getParam(F("hamp"), true)->value().length() < sizeof(tempPassword))
      strcpy(tempPassword, request->getParam(F("hamp"), true)->value().c_str());
    //check for previous password (there is a predefined special password that mean to keep already saved one)
    if (strcmp_P(tempPassword, appDataPredefPassword))
      strcpy(_ha.mqtt.password, tempPassword);

    switch (_ha.mqtt.type)
    {
    case HA_MQTT_GENERIC:
      if (request->hasParam(F("hamgbt"), true) && request->getParam(F("hamgbt"), true)->value().length() < sizeof(_ha.mqtt.generic.baseTopic))
        strcpy(_ha.mqtt.generic.baseTopic, request->getParam(F("hamgbt"), true)->value().c_str());

      if (!_ha.hostname[0] || !_ha.mqtt.generic.baseTopic[0])
        _ha.protocol = HA_PROTO_DISABLED;
      break;
    }
    break;
  }

  return true;
};
//------------------------------------------
//Generate JSON from configuration properties
String WebCTSensors::GenerateConfigJSON(bool forSaveFile = false)
{
  String gc('{');

  gc = gc + F("\"cr1\":") + clampRatios[0];
  gc = gc + F(",\"cr2\":") + clampRatios[1];
  gc = gc + F(",\"cr3\":") + clampRatios[2];
  gc = gc + F(",\"cnc1\":") + noiseCancellation[0];
  gc = gc + F(",\"cnc2\":") + noiseCancellation[1];
  gc = gc + F(",\"cnc3\":") + noiseCancellation[2];

  gc = gc + F(",\"haproto\":") + _ha.protocol;
  gc = gc + F(",\"hahost\":\"") + _ha.hostname + '"';
  gc = gc + F(",\"haupperiod\":") + _ha.uploadPeriod;

  //if for WebPage or protocol selected is MQTT
  if (!forSaveFile || _ha.protocol == HA_PROTO_MQTT)
  {
    gc = gc + F(",\"hamtype\":") + _ha.mqtt.type;
    gc = gc + F(",\"hamport\":") + _ha.mqtt.port;
    gc = gc + F(",\"hamu\":\"") + _ha.mqtt.username + '"';
    if (forSaveFile)
      gc = gc + F(",\"hamp\":\"") + _ha.mqtt.password + '"';
    else
      gc = gc + F(",\"hamp\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)

    gc = gc + F(",\"hamgbt\":\"") + _ha.mqtt.generic.baseTopic + '"';
  }

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

  gs = gs + F(",\"has1\":\"");
  switch (_ha.protocol)
  {
  case HA_PROTO_DISABLED:
    gs = gs + F("Disabled");
    break;
  case HA_PROTO_MQTT:
    gs = gs + F("MQTT Connection State : ");
    switch (m_mqttMan.state())
    {
    case MQTT_CONNECTION_TIMEOUT:
      gs = gs + F("Timed Out");
      break;
    case MQTT_CONNECTION_LOST:
      gs = gs + F("Lost");
      break;
    case MQTT_CONNECT_FAILED:
      gs = gs + F("Failed");
      break;
    case MQTT_CONNECTED:
      gs = gs + F("Connected");
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      gs = gs + F("Bad Protocol Version");
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      gs = gs + F("Incorrect ClientID ");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      gs = gs + F("Server Unavailable");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      gs = gs + F("Bad Credentials");
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      gs = gs + F("Connection Unauthorized");
      break;
    }

    if (m_mqttMan.state() == MQTT_CONNECTED)
      gs = gs + F("\",\"has2\":\"Last Publish Result : ") + (_haSendResult ? F("OK") : F("Failed"));

    break;
  }
  gs += '"';

  gs += '}';

  return gs;
};
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebCTSensors::AppInit(bool reInit)
{
  //Stop Publish
  _publishTicker.detach();

  //Stop MQTT
  m_mqttMan.disconnect();

  //if MQTT used so configure it
  if (_ha.protocol == HA_PROTO_MQTT)
  {
    //setup MQTT
    m_mqttMan.setClient(_wifiClient).setServer(_ha.hostname, _ha.mqtt.port);
    m_mqttMan.setConnectedCallback(std::bind(&WebCTSensors::MqttConnectedCallback, this, std::placeholders::_1, std::placeholders::_2));
    m_mqttMan.setCallback(std::bind(&WebCTSensors::MqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //Connect
    m_mqttMan.connect(_ha.mqtt.username, _ha.mqtt.password);
  }

  if (reInit)
    for (byte i = 0; i < NUMBER_OF_CTSENSOR; i++)
      _ctSensors[i].Reset();

  //if HA and upload period != 0, then start ticker
  if (_ha.protocol != HA_PROTO_DISABLED && _ha.uploadPeriod != 0)
    _publishTicker.attach(_ha.uploadPeriod, [this]() { this->_needPublish = true; });

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

  if (_ha.protocol == HA_PROTO_MQTT)
    m_mqttMan.loop();

  if (_needPublish)
  {
    _needPublish = false;
    LOG_SERIAL.println(F("PublishTick"));
    PublishTick();
  }
}

//------------------------------------------
//Constructor
WebCTSensors::WebCTSensors(char appId, String appName) : Application(appId, appName) {}
