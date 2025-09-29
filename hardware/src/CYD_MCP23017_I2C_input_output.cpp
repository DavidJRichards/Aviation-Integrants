/******************************************************

Example sketch for the MCP23017/MCP23S17/MCP23018/
MCP2318_WA library.
written by Wolfgang (Wolle) Ewald
https://wolles-elektronikkiste.de/en/port-expander-mcp23017-2

djrm: CYD version December 2023

*******************************************************/

#include <Arduino.h>
//#include <Wire.h>
#include <MCP23017.h>
#define MCP_ADDRESS 0x20 // (A2/A1/A0 = LOW)
#ifdef CYDCLONE
#define MCP_SDA 22
#define MCP_SCL 27
#else
#define MCP_SDA 18
#define MCP_SCL 17
#endif
/* A hardware reset is performed during init(). If you want to save a pin you can define a dummy 
 * reset pin >= 99 and connect the reset pin to HIGH. This will trigger a software reset instead 
 * of a hardware reset. 
 */
//#define RESET_PIN 28 
#define RESET_PIN 99 
MCP23017 myMCP = MCP23017(&Wire, MCP_ADDRESS, RESET_PIN);

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

#define PIN_LED 1

void setup(){ 
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,1);
  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  delay(100);
  Serial.println("init...");
  digitalWrite(PIN_LED,0);
  delay(100);

  Wire.begin(MCP_SDA, MCP_SCL);

  if(!myMCP.Init()){
    Serial.println("Not connected!: halt");
    while(1){} 
  }

  Serial.println("Connected");
  myMCP.setPortMode(0xff, A);    
  myMCP.setPortMode(0x00, B);

}

void loop(){ 
  int bits;
  bits=myMCP.getPort(B);
  // invert and bitreverse so switches and leds align on demo board
  bits ^= 0xff;
  bits=reverse(bits);
  Serial.println(bits, HEX);
  myMCP.setPort(bits,A);
  delay(500);
} 
