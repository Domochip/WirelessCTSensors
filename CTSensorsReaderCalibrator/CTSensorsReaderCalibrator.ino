#include <SoftwareSerial.h>

#include "Config.h"


#define VERSION "1.1"


SoftwareSerial mySerial(1, 0); // RX, TX
Config config;
bool oscccalTuningDone = false;



// Timer/Counter0 Compare Match A interrupt handler
ISR (TIMER0_COMPA_vect) {
  PORTB ^= 1 << PINB4;        // Invert pin PB4
}

void setup() {

  //OSCCAL -= 5;                // User calibration

  mySerial.begin(57600);
  mySerial.println();

  config.SetDefaultValues();

  if (config.Load()) {
    mySerial.println(F("Current configuration is : "));
    mySerial.print(F("osccalAdjust = ")); mySerial.println(config.osccalAdjust);
    mySerial.print(F("readVccCalib = ")); mySerial.println(config.readVccCalib);
    mySerial.println();
  }
  else mySerial.println(F("Config Load Failed"));

  mySerial.print(F("1st step is OSCCAL Tuning (initial OSCCAL is "));
  mySerial.print(OSCCAL);
  mySerial.println(')');
  mySerial.println(F("A Square signal of 2KHz is now on PB4 (pin 3), use it for tuning"));
  mySerial.println(F("Press '+' or '-' for changing OSCCAL, 'c' to continue to next step"));



  pinMode(4, OUTPUT);         // Set PB4 to output
  TCNT0 = 0;                  // Count up from 0
  TCCR0A = 2 << WGM00;        // CTC mode
  if (CLKPR == 3)             // If clock set to 1MHz
    TCCR0B = (1 << CS00);   // Set prescaler to /1 (1uS at 1Mhz)
  else                        // Otherwise clock set to 8MHz
    TCCR0B = (2 << CS00);   // Set prescaler to /8 (1uS at 8Mhz)
  GTCCR |= 1 << PSR0;         // Reset prescaler
  OCR0A = 249;                 // 249 + 1 = 250 microseconds (2KHz)
  TIFR = 1 << OCF0A;          // Clear output compare interrupt flag
  TIMSK |= 1 << OCIE0A;       // Enable output compare interrupt

}

long readVcc(void) {
  // Not quite the same as the emonLib version
  long result;

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRB &= ~_BV(MUX5);   // Without this the function always returns -1 on the ATmega2560 http://openenergymonitor.org/emon/node/2253#comment-11432
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#endif

  delay(3);          // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);        // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  // return the raw count  - that's more useful here
  return result;
}


void loop() {

  //Step One : adjust OSCCAL
  if (!oscccalTuningDone && mySerial.available()) {
    char c = mySerial.read();
    if (c == '\r' || c == '\n') return;
    switch (c) {
      case '+':
        config.osccalAdjust++;
        OSCCAL++;
        break;
      case '-':
        config.osccalAdjust--;
        OSCCAL--;
        break;
      case 'c':
        oscccalTuningDone = true;
        TIMSK &= ~(1 << OCIE0A);       // disable output compare interrupt
        mySerial.println(F("2nd step is Vcc reading calibration"));
        mySerial.print(F("Please enter measured Vcc voltage : "));
        break;
    }
    if (!oscccalTuningDone) {
      delay(1);
      mySerial.print(F("osccalAdjust = "));
      mySerial.print(config.osccalAdjust);
      mySerial.print(F("(OSCAL:"));
      mySerial.print(OSCCAL);
      mySerial.println(')');
    }
  }

  //Step Two : adjust readVcc Constant
  if (oscccalTuningDone) {

    char inbuf[10];
    float Vs = 0;
    long Vref = 0;
    long RefConstant = 0;
    config.readVccCalib = 0;
    char c = 0;
    byte i = 0;

    readVcc();

    //remove initial \r and \n
    c = mySerial.read();
    while (c == '\r' || c == '\n') {
      while (!mySerial.available()) delay(1);
      c = mySerial.read();
    }
    //c isnot anymore line end so copy it into inbuf
    inbuf[i] = c;
    mySerial.print(c);
    i++;

    //while c is not line end
    while (c != 'V' && c != '\r' && c != '\n') {
      while (!mySerial.available()) delay(1);
      c = mySerial.read();
      inbuf[i] = c;
      mySerial.print(c);
      i++;
    }
    inbuf[i] = 0;


    Vs = atof(inbuf);
    Vref = readVcc();
    config.readVccCalib = (long)(Vs * Vref * 1000);
    mySerial.println();
    mySerial.print(F("You measured "));
    mySerial.print(Vs);
    mySerial.print(F(" V.\nThat means your internal reference is "));
    mySerial.print(Vref, 3);
    mySerial.print(F(" counts or "));
    mySerial.print(Vs / 1024.0 * Vref, 3);
    mySerial.print(F(" V if measured with your meter.\r\nThe constant for that is "));
    mySerial.print(config.readVccCalib);
    mySerial.print(F("L\r\nUsing that in \"readVcc( )\", it will return "));
    mySerial.print(config.readVccCalib / Vref);
    mySerial.print(F(" mV."));
    if (RefConstant / Vref != Vs * 1000) mySerial.print(F(" The difference is due to integer rounding."));

    if (config.readVccCalib > 1228800L || config.readVccCalib < 1024000) mySerial.print(F("\nThe normal range should be 1024000L - 1228800L"));
    mySerial.println();
    mySerial.println();
    mySerial.println(F("Do you want to save it? (Y/N)"));
    c = 0;
    while (c != 'Y' && c != 'y' && c != 'N' && c != 'n') {
      while (!mySerial.available()) delay(1);
      c = mySerial.read();
      mySerial.print(c);
    }
    mySerial.println();
    if (c == 'Y' || c == 'y') {
      config.Save();
      delay(10);;
      mySerial.println(F("Calibration saved in EEPROM"));
    }
  }
}

