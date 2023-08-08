#include <Arduino.h>
#include <math.h>             // initial build using floating point, intent to rewrite with fixed point
#include <RotaryEncoder.h>

extern RotaryEncoder encoder;
extern void encoder_init(int);
extern int encoder_process(void);

#define PIN_IN1 12
#define PIN_IN2 11
