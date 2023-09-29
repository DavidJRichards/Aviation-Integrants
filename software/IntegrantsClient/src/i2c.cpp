#include <Wire.h>
#include "PWM.h"
#include "Arinc.h"

extern bool i2c_override;
extern float gen_frequency;
extern float syn_frequency;
extern float disp_lhs,disp_rhs;
extern float adc_angle;
extern long ARINC_value;     // value sent to ARINC display
extern sine_table_t sine_table;



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
  uint32_t status;      //  4
  float gen_frequency;  //  4
  float syn_frequency;  //  4
  float adc_angle;      //  4
                        //------
                        // 
};


struct I2cRxStruct {
  uint32_t config;          // 4
  Channel_t channels[12];   // 12 * 8 = 96
  uint16_t  phase_offset;   // 2
  byte padding[30];         // 30
                            //------
                            // = 102+30
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
    txData.adc_angle = adc_angle;
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
          angle2res(rxData.channels[i].channel, rxData.channels[i].config, rxData.channels[i].value);
          pwm_volts[i]=rxData.channels[i].amplitude;
      }
  
      sine_table.sync_offset = rxData.phase_offset;
      ARINC_value=rxData.channels[9].value * 100;
      disp_lhs=rxData.channels[10].value;
      disp_rhs=rxData.channels[11].value;
    }
  }
}

//============

// this function is called by the Wire library when a message is received
void receiveEvent(int numBytesReceived) {

  if (newRxData == false) {
    // copy the data to rxData
 
    Wire.readBytes((byte*)&rxData, numBytesReceived);
/*
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
*/
    newRxData = true;
  } else {
    // dump the data
    while (Wire.available() > 0) {
      byte c = Wire.read();
    }
  }
}

//===========

void requestEvent() {
//  Serial.println("request event");
  Wire.write((byte*)&txData, sizeof(txData));
  rqSent = true;
}

//===========

void i2c_setup(void) {
  Serial.println("\nStarting I2C Slave Responder");
  Serial.print("I2C address: ");
  Serial.println(thisAddress);

  // set up I2C
//  Wire1.setSDA(I2C1_SLAVE_SDA_PIN);  // 0 > 4
//  Wire1.setSCL(I2C1_SLAVE_SCL_PIN);  // 1 > 5
  Wire.setSDA(I2C_SLAVE_SDA_PIN);  // 0 > 4
  Wire.setSCL(I2C_SLAVE_SCL_PIN);  // 1 > 5
  Wire.onReceive(receiveEvent);    // register function to be called when a message arrives
  Wire.onRequest(requestEvent);    // register function to be called when a request arrives
  Wire.begin(thisAddress);         // join i2c bus
  Wire.setClock(400000);


  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);
}

// end
