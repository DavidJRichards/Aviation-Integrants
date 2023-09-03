//
//    Common to all sketches: necssary infrastructure identifiers
//

#define USE_I2C_MASTER   // 
#define USE_I2C_ADC      // ADS1115 ADC for Horizon Gyro
#define USE_I2C_DAC      // PCA9685 PWM output (analogue/digital)
#define USE_I2C_DIPSW     // MCP23017

#include "Wifi.h"
// has wifi credentials and mqtt broker ip addr
#define _WIFI_SSID "my_ssid"
#define _WIFI_PASSWORD "my_password"
#define _MQTT_HOST IPAddress(192, 168, mqtt, ipaddr)


// if using TLS, edit config.h and #define ASYNC_TCP_SSL_ENABLED 1
// do the same in async_config.h of the PATCHED ESPAsyncTCP library!! 
const uint8_t cert[20] = { 0x9a, 0xf1, 0x39, 0x79,0x95,0x26,0x78,0x61,0xad,0x1d,0xb1,0xa5,0x97,0xba,0x65,0x8c,0x20,0x5a,0x9c,0xfa };
// If using MQTT server authentication, fill in next two fields!
const char* mqAuth="";
const char* mqPass="";
//
//  Some sketches will require you to set START_WITH_CLEAN_SESSION to false
//  For THIS sketch, leave it at false
//
#define START_WITH_CLEAN_SESSION   true

#include "MenuRoutingTest_menu.h"
#include <Wire.h>

#ifdef USE_I2C_DIPSW
#include <MCP23017.h>
#define MCP_ADDRESS 0x20 // (A2/A1/A0 = LOW)
#define MCP_SDA 22 //26
#define MCP_SCL 27
#define RESET_PIN 99 
MCP23017 myMCP = MCP23017(&Wire, MCP_ADDRESS, RESET_PIN);
#endif

int pub_bits, sub_bits, bits;

int encoder_pos;

#define I2C_ADDRESS 0x48
#define GYRO_POT_FACTOR 56.0  // reading is degrees
#define GYRO_POT_FACTOR_PITCH GYRO_POT_FACTOR
#define GYRO_POT_FACTOR_ROLL GYRO_POT_FACTOR
float voltage_0_1;
float voltage_2_3;

/* Create an instance of the library */
#ifdef USE_I2C_ADC
  #include <ADS1115_WE.h>   // i2c ADC for Horizon Gyro
  ADS1115_WE adc = ADS1115_WE(I2C_ADDRESS);
#endif

#ifdef USE_I2C_DAC
  #include "PCA9685.h"      // i2c PWM DAC controller
  PCA9685 pwmController;
#endif

#define PIN_LED 4
long target1_time = 0;    // event timer value


#include "framework.h" // manages automatic connection / reconnection and runs setup() - put YOUR stuff in userSetup
#include <ArduinoJson.h>
StaticJsonDocument<500> doc;
const char *HyperIMUTopic =   "integrants/sub/HyperIMU";            // Topic to subscribe to
const char *DigitalTopic =    "integrants/sub/DACdata";             // Topic to subscribe to
const char *PWMdataTopic =    "integrants/sub/PWMdata";             // Topic to subscribe to
const char *LEDdataTopic =    "integrants/sub/LEDdata";             // Topic to subscribe to
const char *StatusTopic =     "integrants/pub/status";              // Topic to subscribe to
float pitch, roll, compass;
int values[16];
bool led_blink_green = false;
bool led_blink_blue = false;
// data to be sent and received
typedef struct  {
  int config;          // 2
  float pwm_chans[6];  // 24
  byte padding[6];     //  6
                       //------
                       // 32
} t_I2cTxStruct;

typedef struct  {
  int status;           //  2
  float gen_frequency;  //  4
  float syn_frequency;  //  4
  byte padding[22];     //  22
                        //------
                        // 32
} t_I2cRxStruct;
const byte otherAddress = 0x30;
t_I2cTxStruct txData;  // = {"xxx", 236, 0.0, 0.0, 0.0};
t_I2cRxStruct rxData = { 1, 2.2, 3.3 };
bool rqData = false;
bool newRxData = false;
bool newTxData = false;


const char* pload0="multi-line payload hex dumper which should split over several lines, with some left over";
const char* pload1="PAYLOAD QOS1";
const char* pload2="Save the Pangolin!";

void onMqttConnect(bool sessionPresent) {
  Serial.printf("Connected to MQTT session=%d max payload size=%d\n",sessionPresent,mqttClient.getMaxPayloadSize());
  Serial.println("Subscribing at QoS 2");
  mqttClient.subscribe("integrants/sub/test", 2);

  Serial.println("Subscribing at QoS 2");
  mqttClient.subscribe(HyperIMUTopic, 2);

  Serial.println("Subscribing at QoS 2");
  mqttClient.subscribe(PWMdataTopic, 2);

  Serial.println("Subscribing at QoS 2");
  mqttClient.subscribe(LEDdataTopic, 2);

  Serial.println("Subscribing at QoS 2");
  mqttClient.subscribe(DigitalTopic, 2);

//  Serial.println("Subscribing at QoS 2");
//  mqttClient.subscribe(StatusTopic, 2);

//  Serial.printf("T=%u Publishing at QoS 0\n",millis());
//  mqttClient.publish("integrants/pub/test1",pload0,strlen(pload0));
//  Serial.printf("T=%u Publishing at QoS 1\n",millis());
//  mqttClient.publish("integrants/pub/test2",pload1,strlen(pload1),1); 
//  Serial.printf("T=%u Publishing at QoS 2\n",millis());
//  mqttClient.publish("integrants/pub/test3",pload2,strlen(pload2),2);

  PT1.attach(1,[]{
    // simple way to publish int types  as strings using printf format
//    mqttClient.publish("integrants/pub/test4",ESP.getFreeHeap(),"%u"); 
//    mqttClient.publish("integrants/pub/test4",-33); 
    mqtt_publish();
  });
}

void onMqttMessage(const char* topic, const uint8_t* payload, size_t len,uint8_t qos,bool retain,bool dup) {
  char message[len + 1];
  std::string t(topic);
  std::string sub = t.substr(t.find("/")+1);
//  Serial.printf("\nH=%u Message %s qos%d dup=%d retain=%d len=%d\n",ESP.getFreeHeap(),topic,qos,dup,retain,len);
//  PANGO::dumphex(payload,len,16);
//  Serial.printf("Unpack %s ",sub.c_str());

  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.printf("\nH=%u Message %s qos%d dup=%d retain=%d len=%d\n",ESP.getFreeHeap(),topic,qos,dup,retain,len);
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  if(0==strcmp(topic,LEDdataTopic))
  {

    sub_bits =  0
      | doc["LED1"].as<int>() << 7 
      | doc["LED2"].as<int>() << 6 
      | doc["LED3"].as<int>() << 5 
      | doc["LED4"].as<int>() << 4 
      | doc["LED5"].as<int>() << 3 
      | doc["LED6"].as<int>() << 2 
      | doc["LED7"].as<int>() << 1 
      | doc["LED8"].as<int>() << 0; 

    Serial.printf("[%s] ",topic);
    Serial.printf("LEDS: %d, %d, %d, %d, %d, %d, %d, %d\n\r",
    doc["LED1"].as<int>(), 
    doc["LED2"].as<int>(), 
    doc["LED3"].as<int>(), 
    doc["LED4"].as<int>(), 
    doc["LED5"].as<int>(), 
    doc["LED6"].as<int>(), 
    doc["LED7"].as<int>(), 
    doc["LED8"].as<int>());
  
  }
  else if(0==strcmp(topic,PWMdataTopic))
  {
    txData.pwm_chans[0]=doc["angles"][0].as<float>(); 
    txData.pwm_chans[1]=doc["angles"][1].as<float>(); 
    txData.pwm_chans[2]=doc["angles"][2].as<float>(); 
    txData.pwm_chans[3]=doc["angles"][3].as<float>(); 
    txData.pwm_chans[4]=doc["angles"][4].as<float>(); 

    Serial.printf("[%s] ",topic);
    Serial.printf("Angles: %d, %d, %d, %d, %d\n\r",
    doc["angles"][0].as<int>(), 
    doc["angles"][1].as<int>(), 
    doc["angles"][2].as<int>(), 
    doc["angles"][3].as<int>(), 
    doc["angles"][4].as<int>());

  }
  else if(0==strcmp(topic,StatusTopic))
  {
    Serial.printf("[%s] ",topic);
    Serial.printf(     "Encoder: %d, \t",doc["Encoder"].as<int>());
    Serial.printf("Voltage-ADC1: %d, \t",doc["Voltage-ADC1"].as<int>());
    Serial.printf("Voltage-ADC2: %d, \t",doc["Voltage-ADC2"].as<int>());
    Serial.printf(      "Hz-gen: %d, \t",doc["Hz-gen"].as<int>());
    Serial.printf(      "Hz-syn: %d, \t",doc["Hz-syn"].as<int>());
    Serial.printf(     "Angle-0: %d, \t",doc["Angle-0"].as<int>());
    Serial.printf(     "Angle-1: %d, \t",doc["Angle-1"].as<int>());
    Serial.printf(     "Angle-2: %d, \t",doc["Angle-2"].as<int>());
    Serial.printf(     "Angle-3: %d, \t",doc["Angle-3"].as<int>());
    Serial.printf(     "Angle-4: %d\n\r",doc["Angle-4"].as<int>());

  }
  else if(0==strcmp(topic,HyperIMUTopic))
  {
    compass=doc["orientation"][0].as<float>();
    pitch  =doc["orientation"][1].as<float>();
    roll   =doc["orientation"][2].as<float>();

    txData.pwm_chans[0]=pitch;
    txData.pwm_chans[1]=roll;
    txData.pwm_chans[2]=compass; // (+120)

#if 0
    Serial.printf("[%s] ",topic);
    Serial.print(F("Angles:"));
    Serial.print(compass, 1);
    Serial.print(F(", "));
    Serial.print(pitch, 1);
    Serial.print(F(","));
    Serial.println(roll, 1);
#endif

  }
  else if(0==strcmp(topic,DigitalTopic))
  {
    values[0]=doc["values"][0].as<int>();
    values[1]=doc["values"][1].as<int>();
    values[2]=doc["values"][2].as<int>();
    values[3]=doc["values"][3].as<int>();
    values[4]=doc["values"][4].as<int>();
    values[5]=doc["values"][5].as<int>();
    values[6]=doc["values"][6].as<int>();
    values[7]=doc["values"][7].as<int>();
    values[8]=doc["values"][8].as<int>();
    values[9]=doc["values"][9].as<int>();
    values[10]=doc["values"][10].as<int>();

    Serial.printf("[%s] ",topic);
    Serial.print(F("\nValues:"));
    Serial.print(values[0]);
    Serial.print(", ");
    Serial.print(values[1]);
    Serial.print(", ");
    Serial.print(values[2]);
    Serial.print(", ");
    Serial.print(values[3]);
    Serial.print(", ");
    Serial.print(values[4]);
    Serial.print(", ");
    Serial.print(values[5]);
    Serial.print(", ");
    Serial.print(values[6]);
    Serial.print(", ");
    Serial.print(values[7]);
    Serial.print(", ");
    Serial.print(values[8]);
    Serial.print(", ");
    Serial.print(values[9]);
    Serial.print(", ");
    Serial.print(values[10]);
    Serial.println();

  }
  else
  {
  //  Serial.printf("\nH=%u Message %s qos%d dup=%d retain=%d len=%d\n",ESP.getFreeHeap(),topic,qos,dup,retain,len);
    PANGO::dumphex(payload,len,16);
    Serial.printf("Unpack %s ",sub.c_str());

  }

} 

void mqtt_publish()
{
  char buf[256];
  sprintf(buf, "{\"Encoder\":%d, \"ADC1\":%6.1f, \"ADC2\":%6.1f,\
   \"Hz-gen\":%6.0f, \"Hz-syn\":%6.0f,\
   \"CHAN0\":%6.1f, \"CHAN1\":%6.1f, \"CHAN2\":%6.1f, \"CHAN3\":%6.1f, \"CHAN4\":%6.1f}", 
    encoder_pos,
    voltage_0_1,
    voltage_2_3,
    rxData.gen_frequency, 
    rxData.syn_frequency==rxData.syn_frequency?rxData.syn_frequency:0.0, 
    txData.pwm_chans[0],
    txData.pwm_chans[1],
    txData.pwm_chans[2],
    txData.pwm_chans[3],
    txData.pwm_chans[4]
    );

  mqttClient.publish("integrants/pub/PWMstatus", buf, strlen(buf), 2);

  pub_bits=myMCP.getPort(B);
  sprintf(buf, "{\"DIP1\":%d, \"DIP2\":%d, \"DIP3\":%d, \"DIP4\":%d, \"DIP5\":%d, \"DIP6\":%d, \"DIP7\":%d, \"DIP8\":%d\}",
    (pub_bits&0x01)==0?1:0,
    (pub_bits&0x02)==0?1:0,
    (pub_bits&0x04)==0?1:0,
    (pub_bits&0x08)==0?1:0,
    (pub_bits&0x10)==0?1:0,
    (pub_bits&0x20)==0?1:0,
    (pub_bits&0x40)==0?1:0,
    (pub_bits&0x80)==0?1:0
    );

  mqttClient.publish("integrants/pub/DIPstatus", buf, strlen(buf), 2);

  sub_bits=myMCP.getPort(A);
  sprintf(buf, "{\"LED1\":%d, \"LED2\":%d, \"LED3\":%d, \"LED4\":%d, \"LED5\":%d, \"LED6\":%d, \"LED7\":%d, \"LED8\":%d\}",
                
    (sub_bits&0x80)?1:0,
    (sub_bits&0x40)?1:0,
    (sub_bits&0x20)?1:0,
    (sub_bits&0x10)?1:0,
    (sub_bits&0x08)?1:0,
    (sub_bits&0x04)?1:0,
    (sub_bits&0x02)?1:0,
    (sub_bits&0x01)?1:0
    );
  mqttClient.publish("integrants/pub/LEDstatus",buf, strlen(buf), 2);
}

void userSetup() {

    Wire.begin(22,27);
    setupMenu();

  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  delay(100);
  Serial.println("init...");
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED,1);
#ifdef USE_I2C_ADC
  if (!adc.init()) {
    Serial.println("ADS1115 !! not !! connected");
  }
  else
    Serial.println("ADS1115 connected");
#endif

#ifdef USE_I2C_DAC
//  Wire.begin(22,27);

  pwmController.resetDevices();  // Resets all PCA9685 devices on i2c line
  pwmController.init();          // Initializes module using default totem-pole driver mode, and default disabled phase balancer

  pwmController.setChannelPWM(0, 2048);  // centre bias output (no current limit resistor)
  Serial.print("PCA9685 ch0: ");
  Serial.println(pwmController.getChannelPWM(0)); // Should output 2048, which is 128 << 4

  pwmController.setChannelPWM(1, 2048);  // Set PWM to mid range  (galvo, 3k3 curent limit resistor)
  pwmController.setChannelPWM(2, 2048);  // Set PWM to mid range  (galvo, 3k3 curent limit resistor)
  pwmController.setChannelPWM(3, 2048);  // Set PWM to mid range  (galvo, 3k3 curent limit resistor)

  pwmController.setChannelPWM(5, 4095);  // Set PWM to full on  (flag)
  pwmController.setChannelPWM(6, 4095);  // Set PWM to full on  (flag)

  pwmController.setChannelPWM(7, 2048);  // Set PWM to half on  (half power 28 volt to 12 volt mains relay)

  pwmController.setChannelPWM(8, 4095);   // Set PWM to full on  (28 volt hi side driver to lamp)
  pwmController.setChannelPWM(9, 4095);   // Set PWM to full on  (28 volt hi side driver to solenoid)
  pwmController.setChannelPWM(10, 4095);  // Set PWM to full on  (28 volt hi side driver to solenoid)
#endif

#ifdef USE_I2C_DIPSW
  Wire.begin(MCP_SDA, MCP_SCL);
  if(!myMCP.Init()){
    Serial.println("Not connected! MCP23017 ");
//    while(1){} 
  }
  else
  {
    Serial.println("MCP23017 Connected");
  }
  myMCP.setPortMode(0xff, A);    
  myMCP.setPortMode(0x00, B);

#endif

}

#ifdef USE_I2C_ADC
float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
#ifdef USE_I2C_ADC  
  adc.setCompareChannels(channel);
  adc.setVoltageRange_mV(ADS1115_RANGE_4096);
  adc.startSingleMeasurement();
  while (adc.isBusy()) {}
  voltage = adc.getResult_V();
#endif  
  return voltage;
}
#endif

void transmitData() {

  {  //}    if (newTxData == true) {
    Wire.beginTransmission(otherAddress);
    Wire.write((byte*)&txData, sizeof(txData));
    Wire.endTransmission();  // this is what actually sends the data
    newTxData = false;
    rqData = true;
  }
}

void requestData() {
  if (rqData == true) {  // just one request following every Tx
    byte stop = true;
    byte numBytes = 32;
    Wire.requestFrom(otherAddress, numBytes, stop);
    // the request is immediately followed by the read for the response
    Wire.readBytes((byte*)&rxData, numBytes);
    newRxData = true;
    rqData = false;
  }
}

 enum menu_destination {
  Default,
  Encoder,
  ADC1,
  ADC2,
  IMU_Pitch,
  IMU_Roll,
  IMU_Heading,
  DIP_SW1,
  DIP_SW2,
  DIP_SW3,
  DIP_SW4,
  DIP_SW5,
  DIP_SW6,
  DIP_SW7,
  DIP_SW8,
  MQTT_Value1,
  MQTT_Value2,
  MQTT_Value3,
  MQTT_Value4,
  MQTT_Value5,
  MQTT_Digit1,
  MQTT_Digit2,
  MQTT_Digit3,
  MQTT_Digit4,
  MQTT_Digit5,
  MQTT_Digit6,
  MQTT_Digit7,
  MQTT_Digit8,
  EEP_Digital1,
  EEP_Digital2,
  EEP_Digital3,
  EEP_Digital4,
  EEP_Digital5,
  EEP_Value1,
  EEP_Value2,
  EEP_Value3,
  EEP_Value4,
  EEP_Value5,
  EEP_Angle1,
  EEP_Angle2,
  EEP_Angle3,
  EEP_Angle4,
  EEP_Angle5
 };


// use source index from destination menu
// to lookup appropriate source value
// destination enum above matches order in menu

float get_menuindex(int idx)
{   
  float value=-1;

    switch(idx){
      case Default:
      break;

      case Encoder:
        value = encoder_pos;
        break;

      case ADC1:
        value = readChannel(ADS1115_COMP_0_1)*GYRO_POT_FACTOR_PITCH;
        break;

      case ADC2:
        value = readChannel(ADS1115_COMP_2_3)*GYRO_POT_FACTOR_ROLL;
        break;

      case IMU_Pitch:
        value = pitch;
        break;

      case IMU_Roll:
        value = roll;
        break;

      case IMU_Heading:
        value = compass;
        break;

      case DIP_SW1:
        value = !myMCP.getPin(0, B);
        break;

      case DIP_SW2:
        value = !myMCP.getPin(1, B);
        break;

      case DIP_SW3:
        value = !myMCP.getPin(2, B);
        break;

      case DIP_SW4:
        value = !myMCP.getPin(3, B);
        break;

      case DIP_SW5:
        value = !myMCP.getPin(4, B);
        break;

      case DIP_SW6:
        value = !myMCP.getPin(5, B);
        break;

      case DIP_SW7:
        value = !myMCP.getPin(6, B);
        break;

      case DIP_SW8:
        value = !myMCP.getPin(7, B);
        break;

      case MQTT_Value1:
        value = doc["values"][0].as<int>();
        break;

      case MQTT_Value2:
        value = doc["values"][1].as<int>();
        break;

      case MQTT_Value3:
        value = doc["values"][2].as<int>();
        break;

      case MQTT_Value4:
        value = doc["values"][3].as<int>();
        break;

      case MQTT_Value5:
        value = doc["values"][4].as<int>();
        break;

      case MQTT_Digit1:
        value = doc["LED1"].as<int>();
        break;

      case MQTT_Digit2:
        value = doc["LED2"].as<int>();
        break;

      case MQTT_Digit3:
        value = doc["LED3"].as<int>();
        break;

      case MQTT_Digit4:
        value = doc["LED4"].as<int>();
        break;

      case MQTT_Digit5:
        value = doc["LED5"].as<int>();
        break;

      case MQTT_Digit6:
        value = doc["LED6"].as<int>();
        break;

      case MQTT_Digit7:
        value = doc["LED7"].as<int>();
        break;

      case MQTT_Digit8:
        value = doc["LED8"].as<int>();
        break;

      case EEP_Digital1:
        value = menuEEPDigital1.getCurrentValue();
        break;

      case EEP_Digital2:
        value = menuEEPDigital2.getCurrentValue();
        break;

      case EEP_Digital3:
        value = menuEEPDigital3.getCurrentValue();
        break;

      case EEP_Digital4:
        value = menuEEPDigital4.getCurrentValue();
        break;

      case EEP_Digital5:
        value = menuEEPDigital5.getCurrentValue();
        break;

      case EEP_Value1:
        value = menuEEPValue1.getCurrentValue();
        break;

      case EEP_Value2:
        value = menuEEPValue2.getCurrentValue();
        break;

      case EEP_Value3:
        value = menuEEPValue3.getCurrentValue();
        break;

      case EEP_Value4:
        value = menuEEPValue4.getCurrentValue();
        break;

      case EEP_Value5:
        value = menuEEPValue5.getCurrentValue();
        break;

      case EEP_Angle1:
        value = menuEEPAngle1.getCurrentValue();
        break;

      case EEP_Angle2:
        value = menuEEPAngle2.getCurrentValue();
        break;

      case EEP_Angle3:
        value = menuEEPAngle3.getCurrentValue();
        break;

      case EEP_Angle4:
        value = menuEEPAngle4.getCurrentValue();
        break;

      case EEP_Angle5:
        value = menuEEPAngle5.getCurrentValue();
        break;


    }
    return value;
}


void loop() {

  taskManager.runLoop();

  if (target1_time < millis() ) {
    target1_time += 500;
  
// I2C send and receive to slave Pico2040 PWM generator  
    transmitData();
    requestData();

    txData.pwm_chans[0]=get_menuindex(menuPWMChan0.getCurrentValue() );
    txData.pwm_chans[1]=get_menuindex(menuPWMChan1.getCurrentValue() );
    txData.pwm_chans[2]=get_menuindex(menuPWMChan2.getCurrentValue() );
    txData.pwm_chans[3]=get_menuindex(menuPWMChan3.getCurrentValue() );
    txData.pwm_chans[4]=get_menuindex(menuPWMChan4.getCurrentValue() );
    
    pwmController.setChannelPWM( 1, get_menuindex(menuDACGalv1.getCurrentValue() )); // galv
    pwmController.setChannelPWM( 2, get_menuindex(menuDACGalv2.getCurrentValue() ));
    pwmController.setChannelPWM( 3, get_menuindex(menuDACGalv3.getCurrentValue() ));
    pwmController.setChannelPWM( 5, get_menuindex(menuDACFlag1.getCurrentValue() )>0?4095:0);  // flag
    pwmController.setChannelPWM( 6, get_menuindex(menuDACFlag2.getCurrentValue() )>0?4095:0);
    pwmController.setChannelPWM( 7, get_menuindex(menuDACRelay.getCurrentValue() ));
    pwmController.setChannelPWM( 8, get_menuindex(menuDACLamp.getCurrentValue() ));
    pwmController.setChannelPWM( 9, get_menuindex(menuDACSolenoid1.getCurrentValue() )>0?4095:0); // solenoid
    pwmController.setChannelPWM(10, get_menuindex(menuDACSolenoid2.getCurrentValue() )>0?4095:0);

    myMCP.setPin(7, A, get_menuindex(menuLED1.getCurrentValue() ));
    myMCP.setPin(6, A, get_menuindex(menuLED2.getCurrentValue() ));
    myMCP.setPin(5, A, get_menuindex(menuLED3.getCurrentValue() ));
    myMCP.setPin(4, A, get_menuindex(menuLED4.getCurrentValue() ));
    myMCP.setPin(3, A, get_menuindex(menuLED5.getCurrentValue() ));
    myMCP.setPin(2, A, get_menuindex(menuLED6.getCurrentValue() ));
    myMCP.setPin(1, A, get_menuindex(menuLED7.getCurrentValue() ));
    myMCP.setPin(0, A, get_menuindex(menuLED8.getCurrentValue() ));

    menuFREQGen.setFloatValue(rxData.gen_frequency);
    menuFREQSyn.setFloatValue(rxData.syn_frequency==rxData.syn_frequency?rxData.syn_frequency:0.0);
    encoder_pos=menuEncoder.getCurrentValue()-2048;
    menuPWM1.setFloatValue(txData.pwm_chans[0]);
    menuPWM2.setFloatValue(txData.pwm_chans[1]);
    menuPWM3.setFloatValue(txData.pwm_chans[2]);
    menuPWM4.setFloatValue(txData.pwm_chans[3]);
    menuPWM5.setFloatValue(txData.pwm_chans[4]);

  } // end if target1 time

}

void CALLBACK_FUNCTION eeprom_load(int id) {
    // TODO - your menu change code
    menuMgr.load();
}


void CALLBACK_FUNCTION eeprom_save(int id) {
    // TODO - your menu change code
    menuMgr.save();
}


// get indexed value from menu
// 





