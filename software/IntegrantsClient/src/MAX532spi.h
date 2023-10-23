#include <Arduino.h>
#include <SPI.h>

extern uint16_t* channels[3][2]; // three dual DAC chips
extern byte spi_msg[9]; // three bytes per chip
extern void SPI_setup(void);
extern void SPI_write(void);
extern void SPI_update(byte* pmsg, int index, uint16_t chan1, uint16_t chan2);
extern bool newDACdata;
extern float resolver_DAC_angles[3];


