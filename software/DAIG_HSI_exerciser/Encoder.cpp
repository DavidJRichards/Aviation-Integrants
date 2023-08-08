#include "Encoder.h"

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

constexpr float m = 10;
// at 200ms or slower, there should be no acceleration. (factor 1)
constexpr float longCutoff = 50;
// at 5 ms, we want to have maximum acceleration (factor m)
constexpr float shortCutoff = 5;
// To derive the calc. constants, compute as follows:
// On an x(ms) - y(factor) plane resolve a linear formular factor(ms) = a * ms + b;
// where  f(4)=10 and f(200)=1
constexpr float a = (m - 1) / (shortCutoff - longCutoff);
constexpr float b = 1 - longCutoff * a;
// a global variables to hold the last position

static int lastPos, newPos;

void encoder_init(int value){
  lastPos=newPos=value;
  encoder.setPosition(lastPos);
  Serial.println("encoder");
}


int encoder_process(void)
{
  encoder.tick();
  newPos = encoder.getPosition();
  if (lastPos != newPos) {

    // accelerate when there was a previous rotation in the same direction.

    unsigned long ms = encoder.getMillisBetweenRotations();

    if (ms < longCutoff) {
      // do some acceleration using factors a and b

      // limit to maximum acceleration
      if (ms < shortCutoff) {
        ms = shortCutoff;
      }

      float ticksActual_float = a * ms + b;
//      Serial.print("  f= ");
//      Serial.println(ticksActual_float);

      long deltaTicks = (long)ticksActual_float * (newPos - lastPos);
//      Serial.print("  d= ");
//      Serial.println(deltaTicks);

      newPos = newPos + deltaTicks;
      encoder.setPosition(newPos);
    }

//    Serial.println(newPos);
//    Serial.print("  ms: ");
//    Serial.println(ms);
    lastPos = newPos;
  } // if
  return newPos;

}