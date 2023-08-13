//===================
// Using I2C to send and receive structs between two Arduinos
//   SDA is the data connection and SCL is the clock connection
//   On an Uno  SDA is A4 and SCL is A5
//   On an Mega SDA is 20 and SCL is 21
//   GNDs must also be connected
//===================
#include <math.h>                     // use sin and cos functions in main loop only
//#include <FreeRTOS.h>
//#include <semphr.h>
#include "json_server.h"
#define USING_TWO_CORES

//=================================
#include "i2c.h"
#include "PWM.h"

uint32_t target2_Time = 0;    // millis value for next display event
char dashLine[] = "=====================================================================";
bool core0ready = false;
bool i2c_override = false;

void waitForSerial(unsigned long timeout_millis) {
  unsigned long start = millis();
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

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  waitForSerial(10000);
  Serial.println();
  Serial.println(dashLine);
  Serial.print("PWM slave, setup A, Core #:");
  Serial.println(get_core_num());
  Serial.println(dashLine);

  json_server_setup();
  digitalWrite(LED_BUILTIN, 0);

  #ifdef USE_I2C
  i2c_setup();
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
  json_server_loop();
  {  // frequency display update every 250mS
    static int del_count = 0;
    int del_value;
    del_value = millis();
    if (del_value - del_count > 250) {
      del_count = del_value;
      #ifdef USE_PWM_
      char buf[20];
      sprintf(buf, "Frequency Gen: %6.1f", gen_frequency);
      Serial.print(buf);
      sprintf(buf, ", Sync: %6.1f", syn_frequency);
      Serial.println(buf);
      #endif
    }
  }

  target2_Time = millis();
  if(target2_Time % 250L == 0)
  {
  }
}

void loop1() {
  tight_loop_contents();
}
//============
