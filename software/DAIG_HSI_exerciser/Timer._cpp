#include <Arduino.h>
#include <stdint.h>
#include "Timer.h"

volatile word usperiod1;
float frequency1;

#define COEF 32
uint32_t filter(uint32_t raw) {
  static uint64_t accum = 0;
  accum = accum - accum / COEF + raw;
  return accum / COEF;
}

void interrupt_setup(void)
{
  //  pinMode(pinOpSync, OUTPUT);
  pinMode(pinIpTrig, INPUT);
  attachInterrupt(pinIpTrig, syncInput, RISING);
  Serial.println("interrupt");
}

void interrupt_process(void)
{
    {  // perform frequency averaging every 5ms
    static int del_count = 0;
    int del_value;
    del_value = millis();
    if (del_value - del_count > 5) {
      del_count = del_value;

      if (usperiod == 0) {
        frequency = NAN;
      } else {
        frequency = filter(10000000L / usperiod) / 10.0;
        if (frequency > 9999) frequency = NAN;  // prints as 'inf'
        usperiod = 0;
      }
#if 1      
      if (usperiod1 == 0) {
        frequency1 = NAN;
      } else {
        frequency1 = filter(10000000L / usperiod1) / 10.0;
        if (frequency1 > 9999) frequency1 = NAN;  // prints as 'inf'
        usperiod1 = 0;
      }
#endif    
    }
  }

}