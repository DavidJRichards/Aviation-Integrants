#include <Wire.h>
#include <TM1637.h>
#include "PWM.h"

extern bool i2c_override;
extern float gen_frequency;
extern float syn_frequency;
extern float disp_lhs,disp_rhs;

#ifdef USE_TM1637
TM1637 module(2, 3);
#endif

static const uint I2C_SLAVE_SDA_PIN = 20;  //PICO_DEFAULT_I2C_SDA_PIN;  // 4
static const uint I2C_SLAVE_SCL_PIN = 21;  //PICO_DEFAULT_I2C_SCL_PIN;  // 5
static const uint I2C1_SLAVE_SDA_PIN = 26;
static const uint I2C1_SLAVE_SCL_PIN = 27;

static const byte thisAddress = 0x30;  //

static bool led_blink = false;


typedef struct {
  uint8_t       channel;        // 1
  uint8_t       config;         // 1
  uint16_t      amplitude;      // 2
  float         value;          // 4
} Channel_t;                // = 8


// data to be sent and received
struct I2cTxStruct {
  int status;           //  2
  float gen_frequency;  //  4
  float syn_frequency;  //  4
                        //------
                        // 32
};


struct I2cRxStruct {
  uint32_t config;               // 4
  Channel_t channels[12];   // 12 * 8 = 96
  byte padding[32];     //  2
                            //------
                            // = 100
};

I2cTxStruct txData;
I2cRxStruct rxData;

bool newTxData = false;
bool newRxData = false;
bool rqSent = false;

void updateDataToSend() {

  // update the data after the previous message has been
  //    sent in response to the request
  // this ensures the new data will ready when the next request arrives
  if (rqSent == true) {
    rqSent = false;

    char sText[] = "SendB";
    //    strcpy(txData.textA, sText);
    txData.status += 10;
    txData.gen_frequency = gen_frequency;
    txData.syn_frequency = syn_frequency;
    if (txData.status > 300) {
      txData.status = 236;
    }
    //    txData.valB = millis();
  }
}

void showNewData() {
  static bool led_blink=false;
  char text[40];
  int value;
  int i;

  if (newRxData == true) {

    led_blink = !led_blink;
    digitalWrite(LED_BUILTIN, led_blink);

    if (!i2c_override) {
      newRxData = false;


    for(i=0; i<12; i++)
    {
//        if(rxData.channels[i].channel == 3) // ch3 resrved for reference output
//          continue;
        angle2res(rxData.channels[i].channel, rxData.channels[i].config, rxData.channels[i].value);
        pwm_volts[i]=rxData.channels[i].amplitude;
    }
 
    disp_lhs=rxData.channels[10].value;
    disp_rhs=rxData.channels[11].value;

#if 0
      angle2res(T_FINE, rxData.pwm_config[0], rxData.pwm_chans[0]);
      angle2res(T_MEDIUM, rxData.pwm_config[1], rxData.pwm_chans[1]);
      angle2res(T_COARSE, rxData.pwm_config[2], rxData.pwm_chans[2] + 120);

      angle2res(T_NtoS, rxData.pwm_config[3], rxData.pwm_chans[3]);
//      angle2res(T_HEADING, rxData.pwm_config[4], rxData.pwm_chans[4]);

      angle2wire(T_HEADING, 0, rxData.pwm_chans[4]);
      angle2wire(T_HEADING, 1, rxData.pwm_chans[5]);

      chan3=rxData.pwm_chans[3];
      chan4=rxData.pwm_chans[4];
      chan5=rxData.pwm_chans[5];
#endif
  //    sprintf(text, "P=%.1f, R=%.1f, M=%.1f, H=%.1f, N=%.1f", chan_angle(T_FINE), chan_angle(T_MEDIUM), chan_angle(T_COARSE), chan_angle(T_HEADING), chan_angle(T_NtoS) );
  //    Serial.println(text);
    } else {
      //  Serial.print("Overriden: ");
    }

    #ifdef USE_TM1637
    value = 10 * rxData.heading;
    sprintf(text, "%3u", value);
    module.setDisplayToString(text);
    #else
    #endif
    //#endif
  }
}

//============

// this function is called by the Wire library when a message is received
void receiveEvent(int numBytesReceived) {

  if (newRxData == false) {
    // copy the data to rxData
 
    Wire1.readBytes((byte*)&rxData, numBytesReceived);

    Serial.print("numbytes=");
    Serial.println(numBytesReceived);
    Serial.print("data9: ");
    Serial.print(rxData.channels[9].channel);
    Serial.print(", ");
    Serial.print(rxData.channels[9].config);
    Serial.print(", ");
    Serial.println(rxData.channels[9].value);

    Serial.print(", 10: ");
    Serial.print(rxData.channels[10].channel);
    Serial.print(", ");
    Serial.print(rxData.channels[10].config);
    Serial.print(", ");
    Serial.println(rxData.channels[10].value);

    Serial.print(", 11: ");
    Serial.print(rxData.channels[11].channel);
    Serial.print(", ");
    Serial.print(rxData.channels[11].config);
    Serial.print(", ");
    Serial.println(rxData.channels[11].value);

    newRxData = true;
  } else {
    // dump the data
    while (Wire1.available() > 0) {
      byte c = Wire1.read();
    }
  }
}

//===========

void requestEvent() {
//  Serial.println("request event");
  Wire1.write((byte*)&txData, sizeof(txData));
  rqSent = true;
}

//===========

void i2c_setup(void) {
  Serial.println("\nStarting I2C Slave Responder");
  Serial.print("I2C address: ");
  Serial.println(thisAddress);

  // set up I2C
  Wire1.setSDA(I2C1_SLAVE_SDA_PIN);  // 0 > 4
  Wire1.setSCL(I2C1_SLAVE_SCL_PIN);  // 1 > 5
  Wire1.onReceive(receiveEvent);    // register function to be called when a message arrives
  Wire1.onRequest(requestEvent);    // register function to be called when a request arrives
  Wire1.begin(thisAddress);         // join i2c bus
  Wire1.setClock(400000);


  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);
}

// end
