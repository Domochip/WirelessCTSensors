#ifndef CTSensor_h
#define CTSensor_h

#include <arduino.h>

#define LIVE_VOLTAGE 236

class CTSensor {

  private:
    bool _firstReadDone = false;

    unsigned long _startMillis;
    unsigned long _lastIrecMillis;

    float _currentI = 0.0;
    float _avgI = 0.0; //contains average of current

    float decimalLeft = 0;
    unsigned long _counter = 0;
    bool _counterReady = false;


  public:
    void newIFromCTSensor(float I);
    unsigned long getCounterUpdated();
    float getCurrentI();
    float getAverageI();
    unsigned long getCounter(); //used for status only
    void setCounterFromRemote(unsigned long remoteCounter);
    bool getReady();
    void reset();
};

#endif
