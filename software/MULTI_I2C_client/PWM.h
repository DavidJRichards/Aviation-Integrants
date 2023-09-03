#define USE_PWM

#define TIMER_INTERRUPT_DEBUG         2
#define _TIMERINTERRUPT_LOGLEVEL_     0 //4
#define _PWM_LOGLEVEL_                2 //2

#define pinOpSync     14
#define pinIpTrig     15

extern volatile word gen_usperiod;
extern volatile word syn_usperiod;
extern float gen_frequency;
extern float syn_frequency;

extern uint32_t filter(uint32_t);
extern void syncInput(void);
extern void interrupt_process(void);

#define NUM_OF_PINS 12 //4 //12 // two per resolver
enum RESOLVERS { ABSOLUTE = -1,
                 T_FINE = 0,
                 T_MEDIUM,
                 T_COARSE,
                 T_REFERENCE,
                 T_HEADING,
                 T_NtoS };


//#define DIV_CONST 740
#define AMPLITUDE_FS 15.16  //16.33    // volts rms ful scale output, measured
#define DIV_CONST 2000 //1000 //608       // divisor for 13 volt resolver outputs       832         // divisor for desired 11 volt ouput
#define REF_CONST 608       // divisor for 13 volt reference output (26 v phase to phase)
#define DIV_FACT 560        // multiplier, for menu voltage out calculation - found by experiment (needs updating)

#include "RPi_Pico_TimerInterrupt.h" // pwm duty cycle change timer 
#include "RP2040_PWM.h"      // to define PWM channels

#define NUM_SINE_ELEMENTS 36 //100 //36        // steps per cycle of 400Hz wave
// choose 396 to select lower of two possible frequencies, 400 selects 402
#define SINEWAVE_FREQUENCY_HZ 400 //396 //400   // target frequency
#define SYNC_OFFSET_COUNT 6         // sin table index to use when sync pulse detected (ref phase)
#define PULSE_OFFSET_COUNT 2        // sine table index to use for sync output pulse (pulse position)

typedef struct {
  int num_elements=NUM_SINE_ELEMENTS;
  int sinewave_frequency=SINEWAVE_FREQUENCY_HZ;
  int sync_offset=SYNC_OFFSET_COUNT;
  int16_t levels[NUM_SINE_ELEMENTS];
  uint16_t timer_interval;              // calculated in build sinetable function
  float stepsize;                       // calculated in build sinetable function
} sine_table_t;

#if 0
  int_sine_step_value = ( sine_table.levels[step_index] ); 
#endif

typedef struct {
  const char    *name;
  RP2040_PWM*    PWM_Instance;
  const uint32_t PWM_pin;       // pin number to initialise to
  float          PWM_duty;      // working PWM duty cycle variable
} PWM_t;

typedef struct {
  const char    *name;
  PWM_t         PWM[2];         // PWM channels for this resolver
  float         angle;          // target resolver angle
  int           amplitude[2];   // channel 400Hz sin/cos amplitude vectors (int +- 500)
  float         level[2];       // channel 400Hz sin/cos amplitude vectors (float +- 50.0)
 } resolver_t;

typedef struct {
  resolver_t resolvers[NUM_OF_PINS/2];
//  resolver_t resolvers[2];
  long absolute;                // moving map absolute position
  int  autostep;                // step size for automatic (or encoder) movement
  bool  automatic;               // enable automatic map movement (E/W)
  int  autodelay;               // time between automatic steps
} transport_t;        

extern uint32_t PWM_Pins[NUM_OF_PINS];
extern RP2040_PWM* PWM_Instance[NUM_OF_PINS];

extern volatile word usperiod;
extern volatile word usperiod1;
extern float frequency;
extern float frequency1;

extern float heading;

extern uint16_t PWM_data_idle_top;
extern uint8_t PWM_data_idle_div;
extern uint16_t PWM_data_idle;
extern float PWM_frequency;
extern float PWM_dutyCycle;

extern uint32_t filter(uint32_t raw);
extern void build_sinetable(void);
extern sine_table_t sine_table;

//extern int amplitude;
extern int amplitude_div;
extern transport_t transport; 
extern volatile int step_index; 
extern RPI_PICO_Timer ITimer0;
extern bool TimerHandler0(struct repeating_timer *t);
extern void timer0_setup(void);
extern void syncInput(void);
extern void pwm_setup(void);
extern void abs2res(float bump);
extern void ref2res(void);
extern void angle2wire(int channel, int part, float angle);
extern void angle2res(int channel, float value=NAN, float bump=0);
extern float chan_angle(int chan);

