//===================
// Using I2C to send and receive structs between two Arduinos
//   SDA is the data connection and SCL is the clock connection
//   On an Uno  SDA is A4 and SCL is A5
//   On an Mega SDA is 20 and SCL is 21
//   GNDs must also be connected
//===================
#include <math.h>                     // use sin and cos functions in main loop only
#include <FreeRTOS.h>
#include <semphr.h>
#define USING_TWO_CORES_

//=================================
#include "i2c.h"
#include "PWM.h"
//#include "Timer.h"

uint32_t target2_Time = 0;    // millis value for next display event
char dashLine[] = "=====================================================================";
bool core0ready = false;

void waitForSerial(unsigned long timeout_millis) {
  unsigned long start = millis();
  while (!Serial) {
    if (millis() - start > timeout_millis)
      break;
  }
  delay(500);
  Serial.println("djrm");
}


void setup() {

  Serial.begin(115200);
  waitForSerial(5000);
  Serial.println();
  Serial.println(dashLine);
  Serial.print("SMiths HSI exerciser, setup A, Core #:");
  Serial.println(get_core_num());
  Serial.println(dashLine);

  i2c_setup();

  Serial.println("setup A finished");
  target2_Time = millis() + 1000; 
  core0ready = true;
}

void setup1(){
#ifdef USING_TWO_CORES
  delay(5000);         // dont know why this is needed as well as next line
#endif  
  while (!core0ready)  // wait for setup A to finish ...
  {
    tight_loop_contents();
  }

  Serial.println();
  Serial.println(dashLine);
  Serial.print("Smiths HSI exerciser, setup B, Core #:");
  Serial.println(get_core_num());
  Serial.println(dashLine);

  pwm_setup();
  Serial.println("\nsetup B finished");
  Serial.println(dashLine);
}

//============

void loop() {

  // this bit checks if a message has been received
  showNewData();

  // this function updates the data in txData
  updateDataToSend();
  interrupt_process();

  {  // frequency display update every 250mS
    static int del_count = 0;
    int del_value;
    del_value = millis();
    if (del_value - del_count > 250) {
      del_count = del_value;
      char buf[20];
      sprintf(buf, "Frequency Gen: %6.1f", gen_frequency);
      Serial.print(buf);
      sprintf(buf, ", Sync: %6.1f", syn_frequency);
      Serial.println(buf);
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
