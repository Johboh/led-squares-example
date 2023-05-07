#include "ExternalBrightness.h"

#define SATURATED_AT_ADC 5000
#define UPDATE_EVERY_MS 1000

ExternalBrightness::ExternalBrightness(uint8_t pin) : _pin(pin) {}

unsigned int ExternalBrightness::readAdc() {
  int val = 0;
  for (uint8_t i = 0; i < 5; ++i) {
    val += analogRead(_pin);
    delayMicroseconds(100);
  }
  return val / 5;
}

void ExternalBrightness::handle() {
  auto now = millis();
  if (now - _last_updated_at_ms > UPDATE_EVERY_MS) {
    _latest_adc = readAdc();
    _brightness = (_latest_adc / 8191.0) * 100.0;
    _last_updated_at_ms = now;
  }
}

unsigned int ExternalBrightness::getLatestAdcValue() { return _latest_adc; }

bool ExternalBrightness::saturated() { return _latest_adc >= SATURATED_AT_ADC; }
