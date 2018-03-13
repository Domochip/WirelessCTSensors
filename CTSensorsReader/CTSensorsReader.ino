//This Sketch is for an ATTiny85 @ 16MHz
#include <SoftwareSerial.h>
#include "EmonLib.h"
#include "Config.h"
#include "SimpleTimer.h"


#define VERSION 1.6

#define READ_PERIOD 1250
#define SEND_EVERY_X_PERIOD 2



Config config;

EnergyMonitor emon1, emon2, emon3;
SoftwareSerial mySerial(5, 0); // RX, TX

//Timer to send datas
SimpleTimer mainTimer;

double Irms;
byte periodCount = 0;






//-----------------------------------------------------------------------
// Main Timer Tick (aka this should be done every 5sec)
//-----------------------------------------------------------------------
void mainTick() {
  //exemple line to send "1:8.50" (end with \r\n)

  periodCount++;

  Irms = emon1.calcIrms(1600, config.readVccCalib);
  if (periodCount == SEND_EVERY_X_PERIOD) {
    mySerial.print(F("1:"));
    mySerial.println(Irms);
  }
  Irms = emon2.calcIrms(1600, config.readVccCalib);
  if (periodCount == SEND_EVERY_X_PERIOD) {
    mySerial.print(F("2:"));
    mySerial.println(Irms);
  }
  Irms = emon3.calcIrms(1600, config.readVccCalib);
  if (periodCount == SEND_EVERY_X_PERIOD) {
    mySerial.print(F("3:"));
    mySerial.println(Irms);
  }

  if (periodCount == SEND_EVERY_X_PERIOD) periodCount = 0;
}




void setup() {

  mySerial.begin(57600);

  config.SetDefaultValues();
  config.Load();
  /*
    if (config.Load()) {
    mySerial.println(F("Current configuration is : "));
    mySerial.print(F("osccalAdjust = ")); mySerial.println(config.osccalAdjust);
    mySerial.print(F("readVccCalib = ")); mySerial.println(config.readVccCalib);
    mySerial.print(F("ctSensorCalib = ")); mySerial.println(config.ctSensorCalib);
    mySerial.println();
    }
    else mySerial.println(F("Config Load Failed"));
  */
  //DEBUG
  //config.ctSensorCalib = 1000.0;
  //config.Save();

  //Adjust Oscillator
  OSCCAL += config.osccalAdjust;

  //ordered pin to match PCB
  //ADC number is passed
  emon1.current(2, config.ctSensorCalib);
  emon2.current(1, config.ctSensorCalib);
  emon3.current(3, config.ctSensorCalib);


  //Setup timer for execution every 5sec
  mainTimer.setInterval(READ_PERIOD, mainTick);
}

// the loop routine runs over and over again forever:
void loop() {

  mainTimer.run();
}
