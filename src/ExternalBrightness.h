#include <Arduino.h>

#ifndef __EXTERNAL_BRIGHTNESS_H__
#define __EXTERNAL_BRIGHTNESS_H__

class ExternalBrightness {
public:
  ExternalBrightness(uint8_t pin);

public:
  void handle();
  double getBrightness() { return _brightness; }
  unsigned int getLatestAdcValue();
  bool saturated();

private:
  unsigned int readAdc();

private:
  uint8_t _pin;
  double _brightness;
  unsigned int _latest_adc;
  unsigned long _last_updated_at_ms = 0;
};

#endif //__EXTERNAL_BRIGHTNESS_H__
