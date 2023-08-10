#include <Wire.h>
#include <TM1637.h>

extern float heading;
extern void heading2res(float);

TM1637 module(2, 3);  

static const uint I2C_SLAVE_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN; // 4
static const uint I2C_SLAVE_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN; // 5

static const byte thisAddress = 0x30; // 

static bool led_blink = false;

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

I2cTxStruct txData = {"xxx", 236, 0, 1.0};
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
        strcpy(txData.textA, sText);
        txData.valA += 10;
        txData.heading += 0.1;
        if (txData.valA > 300) {
            txData.valA = 236;
        }
        txData.valB = millis();

    }
}

//=========

void showTxData() {

            // for demo show the data that as been sent
        Serial.print("Sent ");
        Serial.print(txData.textA);
        Serial.print(' ');
        Serial.print(txData.valA);
        Serial.print(' ');
        Serial.print(txData.valB);
        Serial.print(' ');
        Serial.println(txData.heading);

}

//=============

void showNewData() {
  char text[4];
  int value;

    if (newRxData == true) {
#if 0      
      Serial.print("This just in    ");
      Serial.print(rxData.textB);
      Serial.print(' ');
      Serial.print(rxData.valC);
      Serial.print(' ');
      Serial.print(rxData.valD);
      Serial.print(' ');
      Serial.println(rxData.heading);
#endif      
      heading = rxData.heading;
      heading2res(0);
      value = 10 * rxData.heading;
      sprintf(text, "%3u", value);
      module.setDisplayToString(text);
      newRxData = false;
      led_blink = ! led_blink;
      digitalWrite(LED_BUILTIN, led_blink);

    }
}

//============

        // this function is called by the Wire library when a message is received
void receiveEvent(int numBytesReceived) {

    if (newRxData == false) {
            // copy the data to rxData
        Wire.readBytes( (byte*) &rxData, numBytesReceived);
        newRxData = true;
    }
    else {
            // dump the data
        while(Wire.available() > 0) {
            byte c = Wire.read();
        }
    }
}

//===========

void requestEvent() {
    Wire.write((byte*) &txData, sizeof(txData));
    rqSent = true;
}

//===========

void i2c_setup(void)
{
    Serial.println("\nStarting I2C Slave Responder");
    Serial.print("I2C address: ");
    Serial.println(thisAddress);

        // set up I2C
    Wire.setSDA(I2C_SLAVE_SDA_PIN);// 0 > 4
    Wire.setSCL(I2C_SLAVE_SCL_PIN);// 1 > 5
    Wire.onReceive(receiveEvent); // register function to be called when a message arrives
    Wire.onRequest(requestEvent); // register function to be called when a request arrives
    Wire.begin(thisAddress); // join i2c bus

    Serial.println("\nTM1637 7 segment heading angle display");
    module.setupDisplay(true, 2);
    module.setDisplayToString("dJr");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, false);
}

// end
