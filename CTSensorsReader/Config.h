#ifndef Config_h
#define Config_h

class Config {
  public:
    byte osccalAdjust = 0;
    long readVccCalib = 0;
    double ctSensorCalib = 0.0;

    void SetDefaultValues() {
      osccalAdjust = 0;
      readVccCalib = 1126400L;
      ctSensorCalib = 1000.0;
    }
    bool Save();
    bool Load();
  private :
    uint16_t crc; ///!\ crc should always stay in last position
};

#endif

