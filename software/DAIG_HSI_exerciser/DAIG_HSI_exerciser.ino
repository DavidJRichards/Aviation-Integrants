/*
  djrm, HSI course reader with ARINC display
  10 August 2023, adafruit feather 2040 (master) and arduino pico rp2040 (slave)
  two core implementation, ADC / display on first core, Encoder / ARINC on second core

  Sketch to test accuracy of decoded values by comparing to heading display on HSI
  Heading display has an offset of about 10 degrees for some reason 
  Offset value compensated for in displayed value
  Overall accuracy appears to be within 1 degree overall.

  1) display synchro angle on display
  decode synchro transmitter with reference connected to A0, A1, A2 DAC inputs 
  display decoded angle on TFT display as meter needle on compass card
  
  2) transmit adjusted value to ARINC display
  read encoder value to get required offset (default -9.9 degrees)
  add offset to decoded transmitter value
  send value to ARINC display on HSI 

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 
 */

#include <Arduino.h>
#include <math.h>             // initial build using floating point, intent to rewrite with fixed point
#include <SPI.h>
#include <TFT_eSPI.h>         // Hardware-specific library
#include <ADCInput.h>         // buffered ADC read stream
#include "Synchro.h"
#include <RotaryEncoder.h>
#include "Arinc.h"
#include "Compass.h"
#include "Encoder.h"
#include <Seg7.h>
#include <Wire.h>
#include <ADS1115_WE.h>
#include "PCA9685.h"
#include "PWM.h"
#include "Timer.h"
#ifndef ARDUINO_ADAFRUIT_FEATHER_RP2040
#error "application needs ARDUINO_ADAFRUIT_FEATHER_RP2040"
// because four ADC channels are used
#endif


PCA9685 pwmController;

#define I2C_ADDRESS 0x48
#define GYRO_POT_FACTOR 56.0 // reading is degrees
#define GYRO_POT_FACTOR_PITCH GYRO_POT_FACTOR
#define GYRO_POT_FACTOR_ROLL  GYRO_POT_FACTOR

/* Create an instance of the library */
ADS1115_WE adc = ADS1115_WE(I2C_ADDRESS);

#if 0
// disabled due to interaction with display, 
// display action clobbered when Seg7 used
Seg7 dsp( 2); // Invoke class with brightness at 2 and # of didgits at 8
#endif

#define RMS_WINDOW 200   // rms window of 40 samples, means 2 periods @50Hz
float VoltRange = 56; 
float maxVal = 9.85; //13.95;
Rms readRms; // create an instance of Rms.


const byte otherAddress = 0x30;

        // data to be sent and received
struct I2cTxStruct {
    char textA[16];         // 16 bytes
    int valA;               //  2
    unsigned long valB;     //  4
    float heading;          //  4
    byte padding[6];        //  6
                            //------
                            // 32
};

struct I2cRxStruct {
    char textB[16];         // 16 bytes
    int valC;               //  2
    unsigned long valD;     //  4
    float heading;          //  4
    byte padding[6];        //  6
                            //------
                            // 32
};

I2cTxStruct txData = {"xxx", 236, 0, 0.0};

uint32_t targetTime = 0;    // millis value for next display event
long target2_time = 0;    // second core event timer value
long ARINC_value;     // value sent to ARINC display
int  encoder_pos;
volatile float theta=0.0;
float angle;                // value sent to display

#define INITIAL_ENCODER_VALUE 0 //-99 // used to subtract 9.9 degree offset on mechanical course reading

#define ADC_READ_FREQ    8000   // frequency of adc reading (per each channel)
#define ADC_READ_BUFFERS 2500
#define ADC_OFFSET       2064   // value with no input present

ADCInput ADCInputs(A0, A1, A2, A3);

float readChannel(ADS1115_MUX channel)
{
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.setVoltageRange_mV(ADS1115_RANGE_4096);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage = adc.getResult_V();
  return voltage;
}

void transmitData() {

{//}    if (newTxData == true) {
        Wire.beginTransmission(otherAddress);
        Wire.write((byte*) &txData, sizeof(txData));
        Wire.endTransmission();    // this is what actually sends the data
#if 0
            // for demo show the data that as been sent
        Serial.print("Sent ");
        Serial.print(txData.textA);
        Serial.print(' ');
        Serial.print(txData.valA);
        Serial.print(' ');
        Serial.print(txData.valB);
        Serial.print(' ');
        Serial.println(txData.heading);
#endif
//        newTxData = false;
//        rqData = true;
    }
}

void waitForSerial(unsigned long timeout_millis) {
  unsigned long start = millis();
  while (!Serial) {
    if (millis() - start > timeout_millis)
      break;
  }
  delay(500);
  Serial.println(__FILE__);
}

void setup(void) {

  Serial.begin(115200);
  waitForSerial(8000);
  ADCInputs.setFrequency(ADC_READ_FREQ);
  ADCInputs.setBuffers(3, ADC_READ_BUFFERS);
  ADCInputs.begin();

  // configure for automatic base-line restoration and continuous scan mode:
  readRms.begin(VoltRange, RMS_WINDOW, ADC_12BIT, BLR_ON, CNT_SCAN);
  readRms.start(); //start measuring

  targetTime = millis() + 1000; 
  Serial.println("setup");

}

void setup1() {
  // pins used by encoder
  Serial.println("setup1");
  pinMode(PIN_Hi_429, OUTPUT);
  pinMode(PIN_Lo_429, OUTPUT);
  pinMode(PIN_Debug,  OUTPUT);
  pinMode(LED_BUILTIN,  OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1500);
  encoder_init(INITIAL_ENCODER_VALUE);
  compass_init();
  draw_needle(0,0);

  Wire.begin();
  // n.b. SDA to GPIO 2, and SCL to GPIO 3

  pwmController.resetDevices();       // Resets all PCA9685 devices on i2c line
  pwmController.init();               // Initializes module using default totem-pole driver mode, and default disabled phase balancer

  pwmController.setChannelPWM( 0, 2048); // centre bias output (no current limit resistor)
  
  pwmController.setChannelPWM( 1, 2048); // Set PWM to mid range  (galvo)
  pwmController.setChannelPWM( 2, 2048); // Set PWM to mid range  (galvo)
  pwmController.setChannelPWM( 3, 2048); // Set PWM to mid range  (galvo)
 
  pwmController.setChannelPWM( 5, 4095); // Set PWM to full on  (flag)
  pwmController.setChannelPWM( 6, 4095); // Set PWM to full on  (flag)

  pwmController.setChannelPWM( 7, 2048); // Set PWM to full on  (12 volt - mains relay)

  pwmController.setChannelPWM( 8, 4095); // Set PWM to full on  (lamp)
  pwmController.setChannelPWM( 9, 4095); // Set PWM to full on  (solenoid)
  pwmController.setChannelPWM(10, 4095); // Set PWM to full on  (solenoid)

  if(!adc.init())
  {
    Serial.println("ADS1115 not connected!");
  }
}

void loop() {
  int sinVal;
  int cosVal;
  int refVal;
  int extVal;

  float r2mr1;
  float s1ms3;
  float s3ms2;

  float sinin, cosin, delta, demod;
  int refsqwv;

  // run repeatedly:  read the ADC. ( 0 to 4095 )
  // make value signed +- 2048 by applying offset
  refVal=ADCInputs.read()-ADC_OFFSET; 
  sinVal=ADCInputs.read()-ADC_OFFSET;
  cosVal=ADCInputs.read()-ADC_OFFSET;
  extVal=ADCInputs.read(); // offset handled in RMS module

  // convert to float in range -2PI to +2PI
  s1ms3 =  sinVal / (2048.0 / 2 * M_PI); // Vs1ms3 = Vylw    - Vblu
  s3ms2 =  cosVal / (2048.0 / 2 * M_PI); // Vs3ms2 = Vblu    - Vblk

// note digital input could be used for phase reference if proper square wave available
// hence saving an analogue channel
  if (refVal > 0)
      refsqwv = 1;
  else
      refsqwv = -1;

#if 0
  if(refsqwv>0)
    digitalWrite(LED_BUILTIN, HIGH);
  else if (refsqwv<0)
    digitalWrite(LED_BUILTIN, LOW);                                
#endif

  readRms.update(extVal, refsqwv); // for BLR_ON or for DC(+AC) signals with BLR_OFF

#if 1 
  // scott t transform the synchro inputs
  sinin = s1ms3;
  // cosin = 2/sqrt(3) * (s3ms2 + 0.5 * s1ms3)
  cosin = 1.1547 * (s3ms2 - 0.5 * s1ms3);
#else  
  // or do nothing here if using resolver inputs
  sinin = s1ms3;
  cosin = s3ms2;
#endif  

  // compute error
  delta = sinin*cos(theta) - cosin*sin(theta);

  // demodulate AC error term
  demod = refsqwv * delta;

  // apply gain term to demodulated error term and integrate
  // theta = theta + 1/64*demod
  theta = theta + 0.015625*demod;

  // wrap from -pi to +pi
  theta = fmod((theta),(M_PI));
 
/*
// angle display update every 250mS
  if (targetTime < millis()) 
  {
    targetTime += 250;

#if 0                                   
//    Serial.print("-2048, 2048, ");
    Serial.print("-10, 10, ");
//    Serial.print("ref=");
    Serial.print(refVal);
    Serial.print(", ");
//    Serial.print("sin=");
    Serial.print(cosVal);
    Serial.print(", ");
//    Serial.print("cos=");
    Serial.print(sinVal);
    Serial.print(", ");
#endif

#if 0
    Serial.print(r2mr1,2);
    Serial.print(",");
    Serial.print(s1ms3,2);
    Serial.print(",");
    Serial.print(s3ms2,2);
    Serial.print(",");
#endif

#if 0
//    Serial.print("sinin=");
    Serial.print(sinin,2);
    Serial.print(", ");
//    Serial.print("cosin=");
    Serial.print(cosin,2);
    Serial.print(", ");
#endif

#if 0
    Serial.print("delta=");
    Serial.print(delta,2);
    Serial.print(", ");
    Serial.print("demod=");
    Serial.print(demod,2);
    Serial.print(", ");
#endif

#if 0 // moved to second core
    angle = theta*180/M_PI ;
    angle = fmod(angle+180,360);
    Serial.print("theta=");
    Serial.print(theta,2);
    Serial.print(theta_,2);
    Serial.print(", ");
    Serial.print("angle=");
    Serial.print("0, 360, ");
    Serial.print(angle,2);
    Serial.print("Frequency = ");
    Serial.println(frequency, 1);

    Serial.print("Frequency1 = ");
    Serial.println(frequency1, 1);
    Serial.println();
#endif

  }
*/
}

void loop1() {
  // put your main code here, to run repeatedly: 
  static int offset = 0;
  static int sensorValue = 2048;
  char dspbuf[12];
  float voltage_0_1;
  float voltage_2_3;
  unsigned long data, ARINC_data;
  word label, sdi, ssm;
  float voltage,course,heading,compass;
  int pitch,roll;

  encoder_pos = encoder_process();

  label = 0201;   // octal message label
  sdi   = 0;      // source - destination identifiers
  ssm   = 0;      // sign status matrix

  target2_time = millis();

  // update heading bug reading
  if(target2_time % 5L == 0)
  {
    readRms.publish();
    voltage = f_filter(readRms.rmsVal);
  }

  if(target2_time % 250L == 0)
  {
    // update course reading (main arrow), show in lcd
    angle = theta*180/M_PI ;
    angle = fmod(angle+180,360);
    course = angle;

    // update heading setting reading (yellow bug)
    readRms.publish();
    voltage = f_filter(readRms.rmsVal);
    if(voltage > maxVal) voltage = maxVal;
    if(voltage < -maxVal) voltage = -maxVal;
    heading = 180.0 / M_PI * asin(voltage/maxVal);
    heading = fmod(heading+360,360);

    // get compass from encoder
    compass = fmod((encoder_pos+360),360);
    txData.heading  = compass;

    // send data to i2c bus servo transmitter
    transmitData();

    // update LCD
    draw_needle(course,heading);   // draw meter pointer

    // put data on ARINC display
    ARINC_value =  compass * 100;
    if(ARINC_value>=80000L)ARINC_value=79999L;
    ARINC_data = ARINC429_BuildCommand(label, sdi, ARINC_value, ssm);
    ARINC429_SendCommand(ARINC_data);

  // read gyro data from i2c ADC
    voltage_0_1 = readChannel(ADS1115_COMP_0_1);
    voltage_2_3 = readChannel(ADS1115_COMP_2_3);

#if 0
    sprintf(dspbuf,"%-4.0f%4.0f",heading,angle);
    dsp.stg( dspbuf );

    sprintf(dspbuf,"%4d%4d",int(voltage_0_1*GYRO_POT_FACTOR_PITCH),int(voltage_2_3*GYRO_POT_FACTOR_ROLL));
    dsp.stg( dspbuf );
#endif
    pitch = voltage_0_1 * GYRO_POT_FACTOR_PITCH * 10;
    roll = voltage_2_3 * GYRO_POT_FACTOR_ROLL * 10;

    // send galvo data via i2c
    pwmController.setChannelPWM( 2, 2048 + (roll) ); // Set PWM (horizontal galvo)
    pwmController.setChannelPWM( 3, 2048 + (pitch));    // Set PWM (vertical galvo)

#if 0
    Serial.print("Pitch=");
    Serial.print(voltage_0_1);
    Serial.print(",");
    Serial.print(pitch);
    Serial.print(",\t Roll=");
    Serial.print(voltage_2_3);
    Serial.print(",");
    Serial.println(roll);
#endif

#if 0
    Serial.print(compass,2);
    Serial.print(", ");
    Serial.print(course,2);
    Serial.print(", ");
    Serial.print(heading,2);
    Serial.println();
#endif

  }

}



