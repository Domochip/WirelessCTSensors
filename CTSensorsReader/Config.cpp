#include <Arduino.h>
#include <EEPROM.h>

#include "Config.h"

uint16_t crc16(const uint8_t* data_p, uint16_t length) {
  uint8_t x;
  uint16_t crc = 0xFFFF;

  while (length--) {
    x = crc >> 8 ^ *data_p++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
  }
  return crc;
}


bool Config::Save() {
#ifdef ESP8266
  EEPROM.begin(sizeof(Config));
#endif

  // Init pointer
  uint8_t * p = (uint8_t *) this ;

  // Init CRC
  this->crc = crc16(p, (uint8_t*)&this->crc - (uint8_t*)this);

  //For each byte of Config object
  for (uint16_t i = 0; i < sizeof(Config); ++i) EEPROM.write(i, *(p + i));

#ifdef ESP8266
  EEPROM.end();
#endif

  return Load();
}


bool Config::Load() {
#ifdef ESP8266
  EEPROM.begin(sizeof(Config));
#endif

  //tmpConfig will be used to load EEPROM datas
  Config tmpConfig;

  //create pointer tmpConfig
  uint8_t * p = (uint8_t *) &tmpConfig ;

  // For size of Config, read bytes
  for (uint16_t i = 0; i < sizeof(Config); ++i) *(p + i) = EEPROM.read(i);

#ifdef ESP8266
  EEPROM.end();
#endif


  // Check CRC
  if (crc16(p, (uint8_t*)&tmpConfig.crc - (uint8_t*)&tmpConfig) == tmpConfig.crc) {
    *this = tmpConfig;
    return true;
  }

  return false;
}




