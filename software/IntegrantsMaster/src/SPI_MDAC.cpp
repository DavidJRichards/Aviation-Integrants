#include "SPI_MDAC.h"
#include <math.h>

#define LED_BUILTIN 4

bool newDACdata;

byte spi_msg[BYTES_MDAC_SPI_BUFFER];

uint16_t channel_B;
uint16_t channel_A;
uint16_t channel_D;
uint16_t channel_C;
uint16_t channel_F;
uint16_t channel_E;

uint16_t* channels[NUM_MDACS][CHANS_PER_MDAC]=
{
    {&channel_A, &channel_B},
    {&channel_C, &channel_D},
    {&channel_E, &channel_F},
};

float resolver_DAC_angles[NUM_MDACS];

SPIClass mySpi = SPIClass(HSPI);
SPISettings setMAX532(2000000, MSBFIRST, SPI_MODE0);

const int slaveSelectPin = 5;
const int SPI_SCK        = 18;
const int SPI_MOSI       = 23;
const int SPI_MISO       = 19;

void SPI_setup(void)
{
   pinMode(slaveSelectPin, OUTPUT);
   mySpi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, slaveSelectPin);
}


void SPI_write(void) 
{
  if(newDACdata)
  {
    static bool led_blink=false;
    int i,channel,index;
    led_blink = !led_blink;
    //digitalWrite(LED_BUILTIN, led_blink);
    mySpi.beginTransaction(setMAX532);
    digitalWrite(slaveSelectPin, LOW);
    mySpi.transferBytes(spi_msg, NULL, sizeof(spi_msg));
    mySpi.endTransaction();
    digitalWrite(slaveSelectPin, HIGH);
    newDACdata=false;
  }
}


void SPI_process(void)
{
  int i;
  int index;
  float target;
  for(i=0; i<12; i++)
  {
      int channel = txData.channels[i].channel;
      byte config = txData.channels[i].config;
      float value = txData.channels[i].value;
      switch(config){
          
        case dac_resolver:
          // output to MDAC resolver 
          if(channel < NUM_MDACS )
          {
            value = fmod(value, 360);
            if(value != resolver_DAC_angles[channel])
            {
              resolver_DAC_angles[channel] = value;
              target = fmod(resolver_DAC_angles[channel], 360) * M_PI / 180.0;
              *channels[channel][0] = 2048 + sin(target) * 2047.0;
              *channels[channel][1] = 2048 + cos(target) * 2047.0;
              index = channel * BYTES_PER_MDAC;
              spi_msg[index+0] = (byte) (   *channels[channel][1] >> 4);
              spi_msg[index+1] = (byte) ( ( *channels[channel][1] << 4 ) | ( *channels[channel][0] >> 8 ) );
              spi_msg[index+2] = (byte) (   *channels[channel][0] & 0xFF);
              newDACdata = true;
            }
          }
        break;
      
        // process only MDAC channels
        default: 
        break;
    }
  }
}
