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

// these constants represent the gearing between the resolvers
const float ratio0 = (32.2727272727 / 1.00979);              // medium to coarse
const float ratio2 = (1041.5289256198 / 1.00979 / 1.00333);  //ratio1*30; // fine to coarse
const float ratio1 = (ratio2 / ratio0);                      //30;        // fine to medium


float absolute = 0;  //3481.2;
float fine = 0.0;
float medium = 0.0;
float coarse = 0.0;
float heading = 0.0;
float ntos = 0;         //80.0;
float ntos_offset = 0;  //91.5;
float course = 0.0;
float compass = 0.0;
float pitch = 0.0;
float roll = 0.0;


 enum menu_pwm_config {
  pwm_disable,
  pwm_synchro,
  pwm_resolver,
  pwm_sineCha,
  pwm_sineChb,
  pwm_reference
 };


#define DEFAULT_COARSE_OFFSET -0  //-90
#define DEFAULT_MEDIUM_OFFSET 0   //65
#define DEFAULT_FINE_OFFSET 0

int coarse_offset = DEFAULT_COARSE_OFFSET;  // film at left hand end of roll, abs 0
int medium_offset = DEFAULT_MEDIUM_OFFSET;  //
int fine_offset = DEFAULT_FINE_OFFSET;

volatile int step_index = 0; 
int amplitude_div = DIV_CONST;
int amplitude_ref = REF_CONST;  // default reference amplitude is just 8v to limit amplifier power dissipation

uint32_t PWM_Pins[NUM_OF_PINS]     = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
RP2040_PWM* PWM_Instance[NUM_OF_PINS];

int pwm_volts[12]={DIV_FACT, DIV_FACT, DIV_FACT, REF_CONST, DIV_FACT, DIV_FACT,
                   DIV_FACT, DIV_FACT, DIV_FACT, DIV_FACT, DIV_FACT, DIV_FACT};

volatile word gen_usperiod;
float gen_frequency = NAN;

volatile word syn_usperiod;
float syn_frequency;

transport_t transport = {
  // name,  instance,       pin,          duty1, instance,    pin,    duty2, angle, amp1, amp2, duty1, duty2
  "Fine",     "sin",  NULL, PWM_Pins[0],  0.0, "cos",   NULL, PWM_Pins[1],  0.0, 0.0, 0, 0, 0.0, 0.0,   // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Medium",   "sin",  NULL, PWM_Pins[2],  0.0, "cos",   NULL, PWM_Pins[3],  0.0, 0.0, 0, 0, 0.0, 0.0,   // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Coarse",   "sin",  NULL, PWM_Pins[4],  0.0, "cos",   NULL, PWM_Pins[5],  0.0, 0.0, 0, 0, 0.0, 0.0,   // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Reference","ref+", NULL, PWM_Pins[6],  0.0, "ref-",  NULL, PWM_Pins[7],  0.0, 0.0, 0, 0, 0.0, 0.0,   // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "Heading",  "sin",  NULL, PWM_Pins[8],  0.0, "cos",   NULL, PWM_Pins[9],  0.0, 0.0, 0, 0, 0.0, 0.0,   // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  "NtoS",     "sin",  NULL, PWM_Pins[10], 0.0, "cos",   NULL, PWM_Pins[11], 0.0, 0.0, 0, 0, 0.0, 0.0,   // name, instance, pin, level, instance, pin, level, angle, scale1, scale2
  0L,                                                                                                   // absolute
  10,                                                                                                   // autostep
  false,                                                                                                // automatic
  10,                                                                                                   // autodelay
};

uint16_t PWM_data_idle_top = 1330; // to give 100 kHz PWM with 133MHz clock
uint8_t PWM_data_idle_div = 1;
// You can select any value
uint16_t PWM_data_idle = 124;
// dummy values for channel creation
float PWM_frequency = 1000;
float PWM_dutyCycle = 50.0f;

#define COEF 32
uint32_t filter(uint32_t raw) {
  static uint64_t accum = 0;
  accum = accum - accum / COEF + raw;
  return accum / COEF;
}

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
  Serial.print("Sync Offset count: ");
  Serial.println(sine_table.sync_offset);
  Serial.print("Pulse Offset count: ");
  Serial.println(PULSE_OFFSET_COUNT);

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
#if 0  
  dc_levels[0] = MID_POINT_INT + transport.resolvers[0].amplitude[0] * int_sine_step_value / amplitude_div;
  dc_levels[1] = MID_POINT_INT + transport.resolvers[0].amplitude[1] * int_sine_step_value / amplitude_div;

  dc_levels[2] = MID_POINT_INT + transport.resolvers[1].amplitude[0] * int_sine_step_value / amplitude_div;
  dc_levels[3] = MID_POINT_INT + transport.resolvers[1].amplitude[1] * int_sine_step_value / amplitude_div;

  dc_levels[4] = MID_POINT_INT + transport.resolvers[2].amplitude[0] * int_sine_step_value / amplitude_div;
  dc_levels[5] = MID_POINT_INT + transport.resolvers[2].amplitude[1] * int_sine_step_value / amplitude_div;

  dc_levels[6] = MID_POINT_INT + transport.resolvers[3].amplitude[0] * int_sine_step_value / amplitude_ref;  // reference +sinewave output
  dc_levels[7] = MID_POINT_INT + transport.resolvers[3].amplitude[1] * int_sine_step_value / amplitude_ref;  // reference -sinewave output

  dc_levels[8] = MID_POINT_INT + transport.resolvers[4].amplitude[0] * int_sine_step_value / amplitude_div;
  dc_levels[9] = MID_POINT_INT + transport.resolvers[4].amplitude[1] * int_sine_step_value / amplitude_div;

  dc_levels[10] = MID_POINT_INT + transport.resolvers[5].amplitude[0] * int_sine_step_value / amplitude_div;
  dc_levels[11] = MID_POINT_INT + transport.resolvers[5].amplitude[1] * int_sine_step_value / amplitude_div;
#else
  dc_levels[0] = MID_POINT_INT + transport.resolvers[0].amplitude[0] * int_sine_step_value / pwm_volts[0];
  dc_levels[1] = MID_POINT_INT + transport.resolvers[0].amplitude[1] * int_sine_step_value / pwm_volts[0];

  dc_levels[2] = MID_POINT_INT + transport.resolvers[1].amplitude[0] * int_sine_step_value / pwm_volts[1];
  dc_levels[3] = MID_POINT_INT + transport.resolvers[1].amplitude[1] * int_sine_step_value / pwm_volts[1];

  dc_levels[4] = MID_POINT_INT + transport.resolvers[2].amplitude[0] * int_sine_step_value / pwm_volts[2];
  dc_levels[5] = MID_POINT_INT + transport.resolvers[2].amplitude[1] * int_sine_step_value / pwm_volts[2];

  dc_levels[6] = MID_POINT_INT + transport.resolvers[3].amplitude[0] * int_sine_step_value / pwm_volts[3];  // reference +sinewave output
  dc_levels[7] = MID_POINT_INT + transport.resolvers[3].amplitude[1] * int_sine_step_value / pwm_volts[3];  // reference -sinewave output

  dc_levels[8] = MID_POINT_INT + transport.resolvers[4].amplitude[0] * int_sine_step_value / pwm_volts[4];
  dc_levels[9] = MID_POINT_INT + transport.resolvers[4].amplitude[1] * int_sine_step_value / pwm_volts[4];

  dc_levels[10] = MID_POINT_INT + transport.resolvers[5].amplitude[0] * int_sine_step_value / pwm_volts[5];
  dc_levels[11] = MID_POINT_INT + transport.resolvers[5].amplitude[1] * int_sine_step_value / pwm_volts[5];
#endif
  PWM_Instance[0]->setPWM_manual_Fast(PWM_Pins[0], dc_levels[0]);
  PWM_Instance[1]->setPWM_manual_Fast(PWM_Pins[1], dc_levels[1]);
  PWM_Instance[2]->setPWM_manual_Fast(PWM_Pins[2], dc_levels[2]);
  PWM_Instance[3]->setPWM_manual_Fast(PWM_Pins[3], dc_levels[3]);
  PWM_Instance[4]->setPWM_manual_Fast(PWM_Pins[4], dc_levels[4]);
  PWM_Instance[5]->setPWM_manual_Fast(PWM_Pins[5], dc_levels[5]);
  PWM_Instance[6]->setPWM_manual_Fast(PWM_Pins[6], dc_levels[6]);
  PWM_Instance[7]->setPWM_manual_Fast(PWM_Pins[7], dc_levels[7]);
  PWM_Instance[8]->setPWM_manual_Fast(PWM_Pins[8], dc_levels[8]);
  PWM_Instance[9]->setPWM_manual_Fast(PWM_Pins[9], dc_levels[9]);
  PWM_Instance[10]->setPWM_manual_Fast(PWM_Pins[10], dc_levels[10]);
  PWM_Instance[11]->setPWM_manual_Fast(PWM_Pins[11], dc_levels[11]);

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
        gen_usperiod = ustick - usold;
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
  Serial.print(F("\nStarting Sine table lookup timer on "));
  Serial.println(BOARD_NAME);
  Serial.println(RPI_PICO_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = "));
  Serial.print(F_CPU / 1000000);
  Serial.println(F(" MHz"));

  // sine table lookup timer, Interval in microsecs
  if (ITimer0.attachInterruptInterval(sine_table.timer_interval, TimerHandler0))
  {
    Serial.print(F("Starting ITimer0 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));

}


// ISR for frequency sync input
void syncInput(void) {
  ITimer0.stopTimer();
  step_index = sine_table.sync_offset;
  ITimer0.restartTimer();
#if 1
  static word ustick, usold;
  ustick = timer_hw->timelr;
  syn_usperiod = ustick - usold;
  usold = ustick;
#endif
}

void pwm_setup(void)
{  
  pinMode(pinOpSync, OUTPUT);
  pinMode(pinIpTrig, INPUT_PULLDOWN);
  build_sinetable();
//  ref2res();  // sets T_REFERENCE
//  abs2res(0);
  angle2res(T_FINE,pwm_synchro);
  angle2res(T_MEDIUM,pwm_synchro);
  angle2res(T_COARSE,pwm_synchro);
  angle2res(T_REFERENCE,pwm_reference);
//  angle2res(T_REFERENCE,pwm_sineCha);
//  angle2res(T_REFERENCE,pwm_sineChb);
  angle2res(T_HEADING,pwm_synchro);
  angle2res(T_NtoS,pwm_synchro);

#if 1
  Serial.print(F("Starting 400 Hz PWM generation on "));
  Serial.println(BOARD_NAME);
  Serial.println(RP2040_PWM_VERSION);
#endif

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

  Serial.print("PWM Actual frequency[0] = ");
  Serial.print(PWM_Instance[0]->getActualFreq() / 10);
  Serial.println(" kHz");

  timer0_setup();

  Serial.println("\nStarting sync input interrupt");
  attachInterrupt(pinIpTrig, syncInput, RISING);

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

/*
void ref2res(void)
{
  // reference
  transport.resolvers[T_REFERENCE].amplitude[0] = 500.0;
  transport.resolvers[T_REFERENCE].amplitude[1] = -500.0;
}

void angle2wire(int channel, int part, float angle) {
  float target;
  target = fmod(angle, 360) * M_PI/180.0;
  transport.resolvers[channel].amplitude[part] = -sin(target) * 500;
}
*/
void angle2res(int channel, byte config, float value/*=NAN*/, float bump/*=0*/) {
  float target;

  switch(config){
    case pwm_resolver:
    // output to resolver receiver
    if(value!=value)
      value=transport.resolvers[channel].angle;
    value = fmod(value + bump, 360);
    transport.resolvers[channel].angle = value;
    target = fmod(transport.resolvers[channel].angle, 360) * M_PI / 180.0;
    transport.resolvers[channel].amplitude[0] = sin(target) * 500;
    transport.resolvers[channel].amplitude[1] = cos(target) * 500;
    break;

    case pwm_synchro:
    // output to synchro receiver
    if(value!=value)
      value=transport.resolvers[channel].angle;
    value = fmod(value + bump, 360);
    transport.resolvers[channel].angle = value;
    target = fmod(transport.resolvers[channel].angle + 120, 360) * M_PI / 180.0;
    transport.resolvers[channel].amplitude[0] = sin(target) * 500;

    target = -fmod(transport.resolvers[channel].angle + 240, 360) * M_PI / 180.0;
    transport.resolvers[channel].amplitude[1] = sin(target) * 500;
    break;

    case pwm_sineCha:
    // output to first half of synchro pair
    target = fmod(value, 360) * M_PI/180.0;
    transport.resolvers[channel].amplitude[0] = sin(target) * 500;
    break;

    case pwm_sineChb:
    // outout to 2nd half of synchro pair
    target = fmod(value, 360) * M_PI/180.0;
    transport.resolvers[channel].amplitude[1] = sin(target) * 500;
    break;

    case pwm_reference:
    // reference
    transport.resolvers[channel].amplitude[0] = 500.0;
    transport.resolvers[channel].amplitude[1] = -500.0;
    break;

    default:
    break;
  }
}

float chan_angle(int channel)
{
  return transport.resolvers[channel].angle;
}


// convert required absolute film position to resolver angles
void abs2res(float bump) {
  float target;

  absolute += bump;
  if (absolute < 0) absolute = 0;

  transport.resolvers[T_COARSE].angle = (absolute / ratio2) + coarse_offset;
  coarse = (absolute / ratio2);

  transport.resolvers[T_MEDIUM].angle = fmod((absolute / ratio1) + medium_offset, 360);
  medium = fmod((absolute / ratio1), 360);

  transport.resolvers[T_FINE].angle = fmod((absolute) + fine_offset, 360);
  fine = fmod((absolute), 360);

  // translate to modbus registers
  //cracked_value.f = absolute;
  //mb.Ireg(absoluteIreg, cracked_value.w[1]);
  //mb.Ireg(absoluteIreg + 1, cracked_value.w[0]);

  // fine
  target = fmod(transport.resolvers[T_FINE].angle, 360) * M_PI / 180.0;
  transport.resolvers[T_FINE].amplitude[0] = sin(target) * 500;
  transport.resolvers[T_FINE].amplitude[1] = cos(target) * 500;
  // medium
  target = fmod(transport.resolvers[T_MEDIUM].angle, 360) * M_PI / 180.0;
  transport.resolvers[T_MEDIUM].amplitude[0] = sin(target) * 500;
  transport.resolvers[T_MEDIUM].amplitude[1] = cos(target) * 500;
  // coarse
  target = fmod(transport.resolvers[T_COARSE].angle, 360) * M_PI / 180.0;
  transport.resolvers[T_COARSE].amplitude[0] = sin(target) * 500;
  transport.resolvers[T_COARSE].amplitude[1] = cos(target) * 500;
  // reference
//  transport.resolvers[T_REFERENCE].amplitude[0] = 500.0;
//  transport.resolvers[T_REFERENCE].amplitude[1] = -500.0;
}


void interrupt_process(void)
{
    {  // perform frequency averaging every 5ms
    static int del_count = 0;
    int del_value;
    del_value = millis();
    if (del_value - del_count > 50) {
      del_count = del_value;

      if (gen_usperiod == 0) {
        gen_frequency = NAN;
      } else {
        gen_frequency = filter(10000000L / gen_usperiod) / 10.0;
        if (gen_frequency > 9999) gen_frequency = NAN;  // prints as 'inf'
          gen_usperiod = 0;
      }
#if 1      
      if (syn_usperiod == 0) {
        syn_frequency = NAN;
      } else {
        syn_frequency = filter(10000000L / syn_usperiod) / 10.0;
        if (syn_frequency > 9999) syn_frequency = NAN;  // prints as 'inf'
          syn_usperiod = 0;
      }
#endif    
    }
  }

}

