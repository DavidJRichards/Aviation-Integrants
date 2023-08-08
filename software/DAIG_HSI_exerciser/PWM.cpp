#include <Arduino.h>
#include <stdint.h>
#include "PWM.h"

#if ( defined(ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
      defined(ARDUINO_GENERIC_RP2040) ) && defined(ARDUINO_ARCH_MBED)

#if(_PWM_LOGLEVEL_>3)
  #warning USING_MBED_RP2040_PWM
#endif

#elif ( defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_ADAFRUIT_FEATHER_RP2040) || \
        defined(ARDUINO_GENERIC_RP2040) ) && !defined(ARDUINO_ARCH_MBED)

#if(_PWM_LOGLEVEL_>3)
  #warning USING_RP2040_PWM
#endif
#else
#error This code is intended to run on the RP2040 mbed_nano, mbed_rp2040 or arduino-pico platform! Please check your Tools->Board setting.
#endif

extern SerialUSB Serial;

sine_table_t sine_table;

// Init RPI_PICO_Timer, can use any from 0-15 pseudo-hardware timers
RPI_PICO_Timer ITimer0(0);

volatile int step_index = 0; 
int amplitude_div = DIV_CONST;
int amplitude_ref = REF_CONST;  // default reference amplitude is just 8v to limit amplifier power dissipation

//uint32_t PWM_Pins[NUM_OF_PINS]     = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
uint32_t PWM_Pins[NUM_OF_PINS]     = { 8, 9 };
//uint32_t PWM_Pins[NUM_OF_PINS]     = { 6, 7, 8, 9, 10, 11 };
//#define NUM_OF_PINS       ( sizeof(PWM_Pins) / sizeof(uint32_t) )
RP2040_PWM* PWM_Instance[NUM_OF_PINS];

volatile word usperiod;
float frequency = NAN;

transport_t transport = {
  // name,      instance, pin, duty1, instance, pin, duty2, angle, amp1, amp2, duty1, duty2
//  "Fine", "sin", NULL, PWM_Pins[0], 0.0, "cos", NULL, PWM_Pins[1], 0.0, 0.0, 0, 0, 0.0, 0.0,         // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
//  "Medium", "sin", NULL, PWM_Pins[2], 0.0, "cos", NULL, PWM_Pins[3], 0.0, 0.0, 0, 0, 0.0, 0.0,       // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
//  "Coarse", "sin", NULL, PWM_Pins[4], 0.0, "cos", NULL, PWM_Pins[5], 0.0, 0.0, 0, 0, 0.0, 0.0,       // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
//  "Reference", "ref+", NULL, PWM_Pins[6], 0.0, "ref-", NULL, PWM_Pins[7], 0.0, 0.0, 0, 0, 0.0, 0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
//  "Heading", "sin", NULL, PWM_Pins[8], 0.0, "cos", NULL, PWM_Pins[9], 0.0, 0.0, 0, 0, 0.0, 0.0,      // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
//  "NtoS", "sin", NULL, PWM_Pins[10], 0.0, "cos", NULL, PWM_Pins[11], 0.0, 0.0, 0, 0, 0.0, 0.0,       // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
//  "Reference", "ref+", NULL, PWM_Pins[0], 0.0, "ref-", NULL, PWM_Pins[1], 0.0, 0.0, 0, 0, 0.0, 0.0,  // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Heading", "sin", NULL, PWM_Pins[0], 0.0, "cos", NULL, PWM_Pins[1], 0.0, 0.0, 0, 0, 0.0, 0.0,      // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
//  "NtoS", "sin", NULL, PWM_Pins[4], 0.0, "cos", NULL, PWM_Pins[5], 0.0, 0.0, 0, 0, 0.0, 0.0,       // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  0L,                                                                                                // absolute
  10,                                                                                                // autostep
  false,                                                                                             // automatic
  10,                                                                                                // autodelay
};


uint16_t PWM_data_idle_top = 1330; // to give 100 kHz PWM with 133MHz clock
uint8_t PWM_data_idle_div = 1;
// You can select any value
uint16_t PWM_data_idle = 124;
// dummy values for channel creation
float PWM_frequency = 1000;
float PWM_dutyCycle = 50.0f;

#if 1
#define COEF 32
uint32_t filter(uint32_t raw) {
  static uint64_t accum = 0;
  accum = accum - accum / COEF + raw;
  return accum / COEF;
}
#endif

void build_sinetable(void)
{
  int n;
  sine_table.stepsize = 360.0 / sine_table.num_elements;
  sine_table.timer_interval = 1E6 / sine_table.sinewave_frequency / sine_table.num_elements;

  for(n=0; n<sine_table.num_elements; n++)
  {
    sine_table.levels[n] = int(sin(M_PI*(n*sine_table.stepsize)/180.0) * 512.0);          // table entries -512 to +512
//    sine_table.factors[n] =   (sin(M_PI*(n*sine_table.stepsize)/180.0) * 128) / 256.0;  // table entries -1.0 to + 1.0 (float)
//    sine_table.levels[n] = int(sin(M_PI*(n*sine_table.stepsize)/180.0) * 512);          // table entries -512 to +512
//    sine_table.levels[n] = int(400 + (sin(M_PI*(n*sine_table.stepsize)/180.0) * 399));  // 0 to 799

  }

#if 1
  Serial.print("Sine table num elements: ");
  Serial.println(sine_table.num_elements);
  Serial.print("Sine table step degrees: ");
  Serial.println(sine_table.stepsize);
  Serial.print("Timer interval uS: ");
  Serial.println(sine_table.timer_interval);

  Serial.println();
  for(int j=0; j<4; j++)
  {
    for(int i=0;  i<sine_table.num_elements/4; i++)
    {
      Serial.print(sine_table.levels[(j*sine_table.num_elements/4) + i]);
      Serial.print(",\t");
    }
  Serial.println();
  }
 Serial.println();
#endif
}


//*
// PWM duty change used in stepping sine values of 400 Hz waveforms
bool TimerHandler0(struct repeating_timer *t)
{ 
  static bool pulse_sent = false;
  (void)t;
  int16_t int_sine_step_value;
  static int frequency_count = 0;
  uint16_t dc_levels[NUM_OF_PINS];

#define MID_POINT_INT 500
  int_sine_step_value = (sine_table.levels[step_index]);

  // manual level is integer 0 to 1000 (actually 800 due to output voltage limitation problem)
  // sine_table is +- 512 ( 10 bits )
  // scale is +- 500 (10 bits )
  // product is 20 bits - needs to be 10 - so divide by 10 bits
  dc_levels[0] = MID_POINT_INT + transport.resolvers[0].amplitude[0] * int_sine_step_value / amplitude_div;
  dc_levels[1] = MID_POINT_INT + transport.resolvers[0].amplitude[1] * int_sine_step_value / amplitude_div;
#if 0
  dc_levels[6] = MID_POINT_INT + transport.resolvers[3].amplitude[0] * int_sine_step_value / amplitude_ref;  // reference +sinewave output
  dc_levels[7] = MID_POINT_INT + transport.resolvers[3].amplitude[1] * int_sine_step_value / amplitude_ref;  // reference -sinewave output
#endif

  PWM_Instance[0]->setPWM_manual_Fast(PWM_Pins[0], dc_levels[0]);
  PWM_Instance[1]->setPWM_manual_Fast(PWM_Pins[1], dc_levels[1]);
#if 0  
  PWM_Instance[6]->setPWM_manual_Fast(PWM_Pins[6], dc_levels[6]);
  PWM_Instance[7]->setPWM_manual_Fast(PWM_Pins[7], dc_levels[7]);
#endif
  //xSemaphoreTake(interruptSemaphore, portMAX_DELAY);
  //xSemaphoreTake(mutex_v, portMAX_DELAY);
  {
    if (step_index >= sine_table.num_elements) {
      step_index = 0;
    }

    if (step_index >= PULSE_OFFSET_COUNT) {
      if (pulse_sent == false) {
        digitalWrite(pinOpSync, 0);
        pulse_sent = true;
#if 1
        static word ustick, usold;
        ustick = timer_hw->timelr;
        usperiod = ustick - usold;
        usold = ustick;
#endif
      }
    } else {
      if (pulse_sent == true) {
        digitalWrite(pinOpSync, 1);
        pulse_sent = false;
      }
    }
    step_index++;
  }
  return true;
}

void timer0_setup(void)
{
 // Interval in microsecs
  if (ITimer0.attachInterruptInterval(sine_table.timer_interval, TimerHandler0))
  {
    Serial.print(F("Starting ITimer0 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));

  Serial.print("PWM Actual frequency[7] = ");
  Serial.print(PWM_Instance[7]->getActualFreq()/10);
  Serial.println(" kHz");

}
#if 1
// ISR for frequency sync input
void syncInput(void) {
  ITimer0.stopTimer();
  step_index = sine_table.sync_offset;
  ITimer0.restartTimer();
#if 1
  static word ustick, usold;
  ustick = timer_hw->timelr;
  usperiod1 = ustick - usold;
  usold = ustick;
#endif
}
#endif

void pwm_setup(void)
{  
  pinMode(pinOpSync, OUTPUT);
  //pinMode(pinIpTrig, INPUT);
  build_sinetable();
  for (uint8_t index = 0; index < NUM_OF_PINS; index++)
  {
    // use dummy values
    PWM_Instance[index] = new RP2040_PWM(PWM_Pins[index], PWM_frequency, PWM_dutyCycle);

    if (PWM_Instance[index])
    {
      // initial values
      PWM_Instance[index]->setPWM_manual(PWM_Pins[index], PWM_data_idle_top, PWM_data_idle_div, PWM_data_idle, true);

      uint32_t div = PWM_Instance[index]->get_DIV();
      uint32_t top = PWM_Instance[index]->get_TOP();
      PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance[index]->get_freq_CPU());
    }
  }
//  ref2res();
//  ntos2res(0);
  heading2res(0);
  Serial.println("pwm");
  timer0_setup();

}


// activated when TIMER_ INTERRUPT_ DEBUG > 2
void printPWMInfo(RP2040_PWM* PWM_Instance)
{
  uint32_t div = PWM_Instance->get_DIV();
  uint32_t top = PWM_Instance->get_TOP();

  // PWM_Freq = ( F_CPU ) / [ ( TOP + 1 ) * ( DIV + DIV_FRAC/16) ]
  PWM_LOGINFO1("Actual PWM Frequency = ",
               PWM_Instance->get_freq_CPU() / ( (PWM_Instance->get_TOP() + 1) * (PWM_Instance->get_DIV() ) ) );

  PWM_LOGDEBUG5("TOP =", top, ", DIV =", div, ", CPU_freq =", PWM_Instance->get_freq_CPU());

//  delay(100);
}

float heading,ntos;
const int ntos_offset=0;

#if 0
void ref2res(void)
{
  // reference
  transport.resolvers[T_REFERENCE].amplitude[0] = 500.0;
  transport.resolvers[T_REFERENCE].amplitude[1] = -500.0;

}
#endif

#define HEADING_SYNCHRO
void heading2res(float bump) {
  float target;
  heading = fmod(heading + bump, 360);
  transport.resolvers[T_HEADING].angle = heading;

  // translate to modbus registers
//  cracked_value.f = heading;
 // mb.Ireg(headingIreg, cracked_value.w[1]);
 // mb.Ireg(headingIreg + 1, cracked_value.w[0]);

#ifndef HEADING_SYNCHRO
?  // output to resolver receiver
  target = fmod(transport.resolvers[T_HEADING].angle, 360) * M_PI / 180.0;
  transport.resolvers[T_HEADING].amplitude[0] = sin(target) * 500;
  transport.resolvers[T_HEADING].amplitude[1] = cos(target) * 500;
#else
  // output to synchro receiver
  target = fmod(transport.resolvers[T_HEADING].angle + 120, 360) * M_PI / 180.0;
  transport.resolvers[T_HEADING].amplitude[0] = sin(target) * 500;

  target = -fmod(transport.resolvers[T_HEADING].angle + 240, 360) * M_PI / 180.0;
  transport.resolvers[T_HEADING].amplitude[1] = sin(target) * 500;
#endif
}

#if 0
#define NTOS_SYNCHRO
void ntos2res(float bump) {
  float target;
  ntos = fmod(ntos + bump, 360);

  // translate to modbus registers
//  cracked_value.f = ntos;
//  mb.Ireg(ntosIreg, cracked_value.w[1]);
//  mb.Ireg(ntosIreg + 1, cracked_value.w[0]);

  transport.resolvers[T_NtoS].angle = fmod(ntos + ntos_offset, 360);
#ifndef NTOS_SYNCHRO
  if (ntos < -90) ntos = -90;
  if (ntos > 90) ntos = 90;
  target = fmod(transport.resolvers[T_NtoS].angle, 360) * M_PI / 180.0;
  transport.resolvers[T_NtoS].amplitude[0] = sin(target) * 500;
  transport.resolvers[T_NtoS].amplitude[1] = cos(target) * 500;
#else
  // output to synchro receiver
  target = fmod(transport.resolvers[T_NtoS].angle + 120, 360) * M_PI / 180.0;
  transport.resolvers[T_NtoS].amplitude[0] = sin(target) * 500;

  target = -fmod(transport.resolvers[T_NtoS].angle + 240, 360) * M_PI / 180.0;
  transport.resolvers[T_NtoS].amplitude[1] = sin(target) * 500;
#endif
}
#endif

#if 0
  // Use at low freq to check
  printPWMInfo(PWM_Instance[0]);
  printPWMInfo(PWM_Instance[1]);
  printPWMInfo(PWM_Instance[2]);
  printPWMInfo(PWM_Instance[3]);
  printPWMInfo(PWM_Instance[4]);
  printPWMInfo(PWM_Instance[5]);
  printPWMInfo(PWM_Instance[6]);
  printPWMInfo(PWM_Instance[7]);
  delay(500);
#endif
