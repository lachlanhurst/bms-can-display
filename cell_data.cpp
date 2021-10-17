#ifndef H_A
#define H_A

#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy

#endif


#define MAX_CELLS 12

struct CellData {
  uint8_t cellCount;
  int16_t voltages[MAX_CELLS];

  int32_t batteryVoltage;
  int32_t chargeCurrent;
  int32_t cellVoltageHigh;
  int32_t cellVoltageAverage;
  int32_t cellVoltageLow;
  int32_t cellVoltageMisMatch;
  
};
