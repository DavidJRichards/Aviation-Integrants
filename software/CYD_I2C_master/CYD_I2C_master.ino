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

#include "IntegrantsMenu_menu.h"
#include <Wire.h>

#ifdef USE_I2C_DIPSW
#include <MCP23017.h>
#define MCP_ADDRESS 0x20 // (A2/A1/A0 = LOW)

#define CYD
#ifdef CYD
#define MCP_SDA 22
#define MCP_SCL 27
#else
?#define MCP_SDA 18
#define MCP_SCL 23
#endif

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
const char *MovingMapTopic =  "integrants/sub/MovingMap";           // Topic to subscribe to
const char *DigitalTopic =    "integrants/sub/DACdata";             // Topic to subscribe to
const char *PWMdataTopic =    "integrants/sub/PWMdata";             // Topic to subscribe to
const char *LEDdataTopic =    "integrants/sub/LEDdata";             // Topic to subscribe to
const char *StatusTopic =     "integrants/pub/status";              // Topic to subscribe to
float pitch, roll, compass;
int values[16];
bool led_blink_green = false;
bool led_blink_blue = false;


#define DEFAULT_COARSE_OFFSET -0  //-90
#define DEFAULT_MEDIUM_OFFSET 0   //65
#define DEFAULT_FINE_OFFSET 0

int coarse_offset = DEFAULT_COARSE_OFFSET;  // film at left hand end of roll, abs 0
int medium_offset = DEFAULT_MEDIUM_OFFSET;  //
int fine_offset = DEFAULT_FINE_OFFSET;
double absolute;
//LargeFixedNumber* large_absolute;

// these constants represent the gearing between the resolvers
const float ratio0 = (32.2727272727 / 1.00979);              // medium to coarse
const float ratio2 = (1041.5289256198 / 1.00979 / 1.00333);  //ratio1*30; // fine to coarse
const float ratio1 = (ratio2 / ratio0);                      //30;        // fine to medium




typedef struct {
  uint8_t       channel;        // 1
  uint8_t       config;         // 1
  uint16_t      amplitude;      // 2
  float         value;          // 4
} Channel_t;



// data to be sent and received
typedef struct  {
  uint32_t config;              // 4
  Channel_t channels[12];       // 12 * 8 = 96
} t_I2cTxStruct;                // = 100

typedef struct  {
  uint32_t status;      //  4
  float gen_frequency;  //  4
  float syn_frequency;  //  4
  float adc_angle;       //  4
                        //------
                        // 
} t_I2cRxStruct;
const byte otherAddress = 0x30;
t_I2cTxStruct txData; 
t_I2cRxStruct rxData;
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
  mqttClient.subscribe(MovingMapTopic, 2);

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
   
    txData.channels[0].value=doc["angles"][0].as<float>(); 
    txData.channels[1].value=doc["angles"][1].as<float>(); 
    txData.channels[2].value=doc["angles"][2].as<float>(); 
    txData.channels[3].value=doc["angles"][3].as<float>(); 
    txData.channels[4].value=doc["angles"][4].as<float>(); 
    txData.channels[5].value=doc["angles"][5].as<float>(); 

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
  else if(0==strcmp(topic,MovingMapTopic))
  {
    if(doc.containsKey("Absolute"))
    {
      menuMapAbsolute.getLargeNumber()->setFromFloat(doc["Absolute"].as<float>());
     cb_absolute(0);
    }
    if(doc.containsKey("Fine"))
      menuMapFine.setFloatValue(doc["Fine"].as<float>());    
    if(doc.containsKey("Medium"))
      menuMapMedium.setFloatValue(doc["Medium"].as<float>());    
    if(doc.containsKey("Coarse"))
      menuMapCoarse.setFloatValue(doc["Coarse"].as<float>());    

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

   sprintf(buf,
   "{\"CHAN0\":%6.1f, \"CHAN1\":%6.1f, \"CHAN2\":%6.1f, \"CHAN3\":%6.1f, \"CHAN4\":%6.1f, \"CHAN5\":%6.1f\
       \"CHAN6\":%6.1f, \"CHAN7\":%6.1f, \"CHAN8\":%6.1f, \"CHAN9\":%6.1f, \"CHAN10\":%6.1f, \"CHAN11\":%6.1f}", 
    txData.channels[0].value,
    txData.channels[1].value,
    txData.channels[2].value,
    txData.channels[3].value,
    txData.channels[4].value,
    txData.channels[5].value,
    txData.channels[6].value,
    txData.channels[7].value,
    txData.channels[8].value,
    txData.channels[9].value,
    txData.channels[10].value,
    txData.channels[11].value
    );

  mqttClient.publish("integrants/pub/PWMstatus", buf, strlen(buf), 2);
 
  sprintf(buf, "{\"Encoder\":%d, \"ADCAngle\":%6.1f, \"ADC1\":%6.1f, \"ADC2\":%6.1f,\
   \"Hz-gen\":%6.0f, \"Hz-syn\":%6.0f}", 
    encoder_pos,
    rxData.adc_angle,
    voltage_0_1,
    voltage_2_3,
    rxData.gen_frequency, 
    rxData.syn_frequency==rxData.syn_frequency?rxData.syn_frequency:0.0 
    );

  mqttClient.publish("integrants/pub/SYSstatus", buf, strlen(buf), 2);

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

  sprintf(buf, "{\"Absolute\":%6.3f, \"Fine\":%3.1f, \"Medium\":%3.1f, \"Coarse\":%3.1f,\
   \"Heading\":%3.0f, \"NtoS\":%3.0f}", 
    absolute,
    menuMapFine.getFloatValue(),
    menuMapMedium.getFloatValue(),
    menuMapCoarse.getFloatValue(),
    0.0,0.0);

  mqttClient.publish("integrants/pub/MAPstatus", buf, strlen(buf), 2);
}

void userSetup() {

//    Wire.begin(22,27);
    Wire.begin(MCP_SDA,MCP_SCL);
    Wire.setClock(400000);
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
// ch4 unused
  pwmController.setChannelPWM(5, 4095);  // Set PWM to full on  (flag)
  pwmController.setChannelPWM(6, 4095);  // Set PWM to full on  (flag)

  pwmController.setChannelPWM(7, 2048);  // Set PWM to half on  (half power 28 volt to 12 volt mains relay)

  pwmController.setChannelPWM(8, 4095);   // Set PWM to full on  (28 volt hi side driver to lamp)
  pwmController.setChannelPWM(9, 4095);   // Set PWM to full on  (28 volt hi side driver to solenoid)
  pwmController.setChannelPWM(10, 4095);  // Set PWM to full on  (28 volt hi side driver to solenoid)
// ch11 unused
  pwmController.setChannelPWM(12, 0);   // Set PWM to full on  (28 volt hi side driver to DC1)
  pwmController.setChannelPWM(13, 0);   // Set PWM to full on  (28 volt hi side driver to DC2)
  pwmController.setChannelPWM(14, 0);   // Set PWM to full on  (28 volt hi side driver to DC3)
  pwmController.setChannelPWM(15, 0   );   // Set PWM to off      (28 volt hi side driver to Amp Mains relay)
// ch15  
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
  //  Serial.printf("sizeoftx %d\n\r",sizeof(txData));
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

 enum menu_pwm_config {
  pwm_disable,
  pwm_synchro,
  pwm_resolver,
  pwm_sineCha,
  pwm_sineChb
 };



 enum menu_destination {
  Default,
  Encoder,
  FREQ_Gen,
  FREQ_Syn,
  ADC1_Volts,
  ADC2_Volts,
  IMU_Pitch,
  IMU_Roll,
  IMU_Heading,
  ADC_Angle,
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
  EEP_Angle5,
  MAP_Absolute,
  MAP_Coarse,
  MAP_Medium,
  MAP_Fine
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

      case FREQ_Gen:
        value = rxData.gen_frequency;
        break;

      case FREQ_Syn:
        value = rxData.syn_frequency;
        break;

      case ADC1_Volts:
        value = voltage_0_1;//readChannel(ADS1115_COMP_0_1)*GYRO_POT_FACTOR_PITCH;
        break;

      case ADC2_Volts:
        value = voltage_2_3;//readChannel(ADS1115_COMP_2_3)*GYRO_POT_FACTOR_ROLL;
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

      case ADC_Angle:
        value = rxData.adc_angle;
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

      case MAP_Coarse:
        value = menuMapCoarse.getFloatValue(); // coarse
        break;

      case MAP_Medium:
        value = menuMapMedium.getFloatValue(); // medium
        break;

      case MAP_Fine:
        value = menuMapFine.getFloatValue(); // fine
        break;

      case MAP_Absolute:
        value = absolute;
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

    txData.channels[0].value=get_menuindex(menuPWMChan0.getCurrentValue() );
    txData.channels[1].value=get_menuindex(menuPWMChan1.getCurrentValue() );
    txData.channels[2].value=get_menuindex(menuPWMChan2.getCurrentValue() );
    txData.channels[3].value=get_menuindex(menuPWMChan3.getCurrentValue() );
    txData.channels[4].value=get_menuindex(menuPWMChan4.getCurrentValue() );
    txData.channels[5].value=get_menuindex(menuPWMChan5.getCurrentValue() );
    txData.channels[6].value=get_menuindex(menuPWMChan6.getCurrentValue() );
    txData.channels[7].value=get_menuindex(menuPWMChan7.getCurrentValue() );
    txData.channels[8].value=get_menuindex(menuPWMChan8.getCurrentValue() );
    txData.channels[9].value=get_menuindex(menuPWMChan9.getCurrentValue() );
    txData.channels[10].value=get_menuindex(menuPWMChan10.getCurrentValue() );
    txData.channels[11].value=get_menuindex(menuPWMChan11.getCurrentValue() );

    txData.channels[0].config=menuPWMConfigCH0.getCurrentValue();
    txData.channels[1].config=menuPWMConfigCH1.getCurrentValue();
    txData.channels[2].config=menuPWMConfigCH2.getCurrentValue();
    txData.channels[3].config=menuPWMConfigCH3.getCurrentValue();
    txData.channels[4].config=menuPWMConfigCH4.getCurrentValue();
    txData.channels[5].config=menuPWMConfigCH5.getCurrentValue();
    txData.channels[6].config=menuPWMConfigCH6.getCurrentValue();
    txData.channels[7].config=menuPWMConfigCH7.getCurrentValue();
    txData.channels[8].config=menuPWMConfigCH8.getCurrentValue();
    txData.channels[9].config=menuPWMConfigCH9.getCurrentValue();
    txData.channels[10].config=menuPWMConfigCH10.getCurrentValue();
    txData.channels[11].config=menuPWMConfigCH11.getCurrentValue();
      
    txData.channels[0].channel=menuPWMChannelCH0.getCurrentValue();
    txData.channels[1].channel=menuPWMChannelCH1.getCurrentValue();
    txData.channels[2].channel=menuPWMChannelCH2.getCurrentValue();
    txData.channels[3].channel=menuPWMChannelCH3.getCurrentValue();
    txData.channels[4].channel=menuPWMChannelCH4.getCurrentValue();
    txData.channels[5].channel=menuPWMChannelCH5.getCurrentValue();
    txData.channels[6].channel=menuPWMChannelCH6.getCurrentValue();
    txData.channels[7].channel=menuPWMChannelCH7.getCurrentValue();
    txData.channels[8].channel=menuPWMChannelCH8.getCurrentValue();
    txData.channels[9].channel=menuPWMChannelCH9.getCurrentValue();
    txData.channels[10].channel=menuPWMChannelCH10.getCurrentValue();
    txData.channels[11].channel=menuPWMChannelCH11.getCurrentValue();

#define REF_CONST 608       // divisor for 13 volt reference output (26 v phase to phase)
#define DIV_FACT 560        // multiplier, for menu voltage out calculation - found by experiment (needs updating)

    txData.channels[0].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[1].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[2].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[3].amplitude=menuReferenceAmplitude.getCurrentValue();
    txData.channels[4].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[5].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[6].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[7].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[8].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[9].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[10].amplitude=menuSynchroAmplitude.getCurrentValue();
    txData.channels[11].amplitude=menuSynchroAmplitude.getCurrentValue();


    pwmController.setChannelPWM( 1, get_menuindex(menuDACGalv1.getCurrentValue() )); // galv
    pwmController.setChannelPWM( 2, get_menuindex(menuDACGalv2.getCurrentValue() ));
    pwmController.setChannelPWM( 3, get_menuindex(menuDACGalv3.getCurrentValue() ));
    pwmController.setChannelPWM( 5, get_menuindex(menuDACFlag1.getCurrentValue() )>0?4095:0);  // flag
    pwmController.setChannelPWM( 6, get_menuindex(menuDACFlag2.getCurrentValue() )>0?4095:0);
    pwmController.setChannelPWM( 7, get_menuindex(menuDACRelay.getCurrentValue() ));
    pwmController.setChannelPWM( 8, get_menuindex(menuDACLamp.getCurrentValue() ));
    pwmController.setChannelPWM( 9, get_menuindex(menuDACSolenoid1.getCurrentValue() )>0?4095:0); // solenoid
    pwmController.setChannelPWM(10, get_menuindex(menuDACSolenoid2.getCurrentValue() )>0?4095:0);

    pwmController.setChannelPWM(12, get_menuindex(menuDACDC1.getCurrentValue() )>0?4095:0);
    pwmController.setChannelPWM(13, get_menuindex(menuDACDC2.getCurrentValue() )>0?4095:0);
    pwmController.setChannelPWM(14, get_menuindex(menuDACDC3.getCurrentValue() )>0?4095:0);
    pwmController.setChannelPWM(15, get_menuindex(menuDACAMP.getCurrentValue() )>0?4095:0);

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
/*
    encoder_pos=menuEncoder.getCurrentValue()-2048;

    large_absolute=menuMapAbsolute.getLargeNumber();
    absolute=large_absolute->getAsFloat();
    menuMapFine.setFloatValue  (fmod((absolute         ) +   fine_offset, 360)); // fine
    menuMapMedium.setFloatValue(fmod((absolute / ratio1) + medium_offset, 360)); // medium
    menuMapCoarse.setFloatValue(fmod((absolute / ratio2) + coarse_offset, 360)); // coarse
*/
    menuSynchroAngle.setFloatValue(rxData.adc_angle);
    
    voltage_0_1=readChannel(ADS1115_COMP_0_1)*GYRO_POT_FACTOR_PITCH;
    menuADC1Voltage.setFloatValue(voltage_0_1);

    voltage_2_3=readChannel(ADS1115_COMP_2_3)*GYRO_POT_FACTOR_ROLL;
    menuADC2Voltage.setFloatValue(voltage_2_3);

    menuPWM0.setFloatValue(txData.channels[0].value);
    menuPWM1.setFloatValue(txData.channels[1].value);
    menuPWM2.setFloatValue(txData.channels[2].value);
    menuPWM3.setFloatValue(txData.channels[3].value);
    menuPWM4.setFloatValue(txData.channels[4].value);
    menuPWM5.setFloatValue(txData.channels[5].value);
    menuPWM6.setFloatValue(txData.channels[6].value);
    menuPWM7.setFloatValue(txData.channels[7].value);
    menuPWM8.setFloatValue(txData.channels[8].value);
    menuPWM9.setFloatValue(txData.channels[9].value);
    menuPWM10.setFloatValue(txData.channels[10].value);
    menuPWM11.setFloatValue(txData.channels[11].value);

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



void CALLBACK_FUNCTION cb_encoder(int id) {
  encoder_pos=menuEncoder.getCurrentValue()-2048;
}


void CALLBACK_FUNCTION cb_absolute(int id) {
//    large_absolute=menuMapAbsolute.getLargeNumber();
//    absolute=large_absolute->getAsFloat();
    absolute=menuMapAbsolute.getLargeNumber()->getAsFloat();
    menuMapFine.setFloatValue  (fmod((absolute         ) +   fine_offset, 360)); // fine
    menuMapMedium.setFloatValue(fmod((absolute / ratio1) + medium_offset, 360)); // medium
    menuMapCoarse.setFloatValue(fmod((absolute / ratio2) + coarse_offset, 360)); // coarse
}

// get indexed value from menu
// 






