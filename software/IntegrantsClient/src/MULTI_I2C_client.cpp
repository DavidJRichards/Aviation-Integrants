//===================
// Using I2C to send and receive structs between two Arduinos
//   SDA is the data connection and SCL is the clock connection
//   GNDs must also be connected
//===================
#include <Arduino.h>
#include <math.h>                     // use sin and cos functions in main loop only
#define USING_TWO_CORES

//=================================
#include "i2c.h"
#include "PWM.h"

#define USE_ADC
#ifdef USE_ADC
  extern void adc_setup(void);
  extern void adc_loop(void);
  extern float adc_process(void);
  extern float adc_angle;
#endif  

#define USE_ARINC
#ifdef USE_ARINC
#include "Arinc.h"
#endif


#define SEG7DSP
#ifdef SEG7DSP
#include <Seg7.h>         // 8 digit 7 segment display
#include <SPI.h>
Seg7 dsp( 2); // Invoke class with brightness at 2 and # of didgits at 8
char dspbuf[12];
#endif
float disp_lhs,disp_rhs;

#define USE_MAX532
#ifdef USE_MAX532
const int slaveSelectPin  = 13;
const int SPI1_MOSI       = 11;
const int SPI1_MISO       = 12;
const int SPI1_SCK        = 10;

uint16_t channel_B=0x555;
uint16_t channel_A=0xAAA;
uint16_t channel_D;
uint16_t channel_C;
uint16_t channel_F;
uint16_t channel_E;

byte spi_msg[9];
uint16_t* channels[3][2]=
{
    {&channel_A, &channel_B},
    {&channel_C, &channel_D},
    {&channel_E, &channel_F},
};


#define SPI_BITRATE 100000 // 1M also ok
SPISettings setMAX532(SPI_BITRATE, MSBFIRST, SPI_MODE0);

void SPI_setup(void)
{
  pinMode(slaveSelectPin, OUTPUT);
  SPI1.setSCK(SPI1_SCK);
  SPI1.setCS(slaveSelectPin);
  SPI1.setRX(SPI1_MISO);
  SPI1.setTX(SPI1_MOSI);
  SPI1.begin(false);
}

void SPI_update(byte* pmsg, int index, uint16_t chan1, uint16_t chan2)
{ 
  index = index * 3;
  pmsg[index+0] = (byte) (   chan2 >> 4);
  pmsg[index+1] = (byte) ( ( chan2 << 4 ) | ( chan1 >> 8 ));
  pmsg[index+2] = (byte) (   chan1 & 0xFF);
}


void SPI_write(void) 
{
#if 0
  SPI_update(spi_msg, 0, channel_A, channel_B);
  SPI_update(spi_msg, 1, channel_C, channel_D);
  SPI_update(spi_msg, 2, channel_E, channel_F);
//#else
  msg[0] = (byte) (   channel_B >> 4);
  msg[1] = (byte) ( ( channel_B << 4 ) | ( channel_A >> 8 ));
  msg[2] = (byte) (   channel_A & 0xFF);

  msg[3] = (byte) (   channel_D >> 4);
  msg[4] = (byte) ( ( channel_D << 4 ) | ( channel_C >> 8 ));
  msg[5] = (byte) (   channel_C & 0xFF);

  msg[6] = (byte) (   channel_F >> 4);
  msg[7] = (byte) ( ( channel_F << 4 ) | ( channel_E >> 8 ));
  msg[8] = (byte) (   channel_E & 0xFF);
#endif
  SPI1.beginTransaction(setMAX532);
  digitalWrite(slaveSelectPin, LOW);
  SPI1.transfer(spi_msg, sizeof(spi_msg));
  SPI1.endTransaction();
  digitalWrite(slaveSelectPin, HIGH);

}
#endif // USE_MAX532



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

  pinMode(PIN_Hi_429, OUTPUT);
  pinMode(PIN_Lo_429, OUTPUT);
  pinMode(LED_BUILTIN,  OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  #ifdef USE_I2C
  i2c_setup();
  #endif

  #ifdef USE_MQTT
  mqtt_setup();
  #endif

  #ifdef USE_ADC1
  adc_setup();
  #endif  

  #ifdef USE_MAX532
  SPI_setup();
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
//#warning "Using two cores"
//#warning "Check core useage in serial report"
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
  
  #ifdef USE_ADC
  adc_setup();
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

  #ifdef USE_ADC1
  adc_loop();
  #endif

  if(target2_Time < millis() )
  {
    target2_Time += 200;

#ifdef USE_MAX532
//    SPI_write();
#endif

#ifdef SEG7DSP
    sprintf(dspbuf,"%-4.0f%4.0f",disp_lhs,disp_rhs);
    dsp.stg( dspbuf );
#endif

#ifdef USE_ADC
    adc_process();
    Serial.print("adc angle: ");
    Serial.println(adc_angle);
#endif    

#ifdef USE_ARINC
  {
  word label, sdi, ssm;
  label = 0201;   // octal message label
  sdi   = 0;      // source - destination identifiers
  ssm   = 0;      // sign status matrix
  
  if( (ARINC_value<80000L) && (ARINC_value>=0L) )
  {
    ARINC_data = ARINC429_BuildCommand(label, sdi, ARINC_value, ssm);
//    ARINC429_PrintCommand(ARINC_data);
    ARINC429_SendCommand(ARINC_data);
  }
  }
#endif

 //   Serial.print(":");
  }
}

void loop1() {
  tight_loop_contents();
  #ifdef USE_ADC
  adc_loop();
  #endif
}
//============
