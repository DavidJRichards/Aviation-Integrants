//===================
// Using I2C to send and receive structs between two Arduinos
//   SDA is the data connection and SCL is the clock connection
//   GNDs must also be connected
//===================
#include <math.h>                     // use sin and cos functions in main loop only
//#include "json_server.h"
#define USING_TWO_CORES

//=================================
#include "i2c.h"
#include "PWM.h"

#define SEG7DSP
#ifdef SEG7DSP
/*
	#define MAX7219_DIN	19
	#define MAX7219_CS	13
	#define MAX7219_CLK	18
  */
#include <Seg7.h>         // 8 digit 7 segment display
#include <SPI.h>
Seg7 dsp( 2); // Invoke class with brightness at 2 and # of didgits at 8
char dspbuf[12];
#endif
float disp_lhs,disp_rhs;


uint32_t target2_Time = 0;    // millis value for next display event
char dashLine[] = "=====================================================================";
bool core0ready = false;
bool i2c_override = false;

void waitForSerial(unsigned long timeout_millis) {
  unsigned long start = millis();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  while (!Serial) {
    if (millis() - start > timeout_millis)
      break;
  }
  delay(1000);
  Serial.println(__FILE__);
  digitalWrite(LED_BUILTIN, 0);
}


void setup() {

  Serial.begin(115200);
  waitForSerial(10000);
  Serial.println();
  Serial.println(dashLine);
  Serial.print("PWM slave, setup A, Core #:");
  Serial.println(get_core_num());
  Serial.println(dashLine);

  digitalWrite(LED_BUILTIN, 0);

  #ifdef USE_I2C
  i2c_setup();
  #endif

  #ifdef USE_MQTT
  mqtt_setup();
  #endif

  target2_Time = millis() + 1000; 
  Serial.println("setup A finished");
  core0ready = true;
#ifndef USING_TWO_CORES
  setup0();
#endif
}

#ifndef USING_TWO_CORES
// this is a fake setup function called by setup
#warning "Using single core only"
void setup0()

#else
// this is the real second core setup function
#warning "Using two cores"
#warning "Check core useage in serial report"
void setup1()
#endif
{
  delay(5000);         // dont know why this is needed as well as next line
  while (!core0ready)  // wait for setup A to finish ...
  {
    tight_loop_contents();
  }

  Serial.println();
  Serial.println(dashLine);
  Serial.print("PWM slave, setup B, Core #:");
  Serial.println(get_core_num());
  Serial.println(dashLine);

  #ifdef USE_PWM
  pwm_setup();
  #endif
  
  Serial.println("\nsetup B finished");
  Serial.println(dashLine);
}

//============
void loop() {

  // this bit checks if a message has been received
  #ifdef USE_I2C
  showNewData();
  #endif

  // this function updates the data in txData
  #ifdef USE_I2C
  updateDataToSend();
  #endif

  #ifdef USE_PWM
  interrupt_process();
  #endif

  if(target2_Time < millis() )
  {
    target2_Time += 500;
#ifdef SEG7DSP
    sprintf(dspbuf,"%-4.0f%4.0f",disp_lhs,disp_rhs);
    dsp.stg( dspbuf );
#endif
 //   Serial.print(":");
  }
}

void loop1() {
  tight_loop_contents();
}
//============
