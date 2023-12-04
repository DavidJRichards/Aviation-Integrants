#include <Arduino.h>
#include <SPI.h>

#define NUM_MDACS 3
#define CHANS_PER_MDAC 2
#define BYTES_PER_MDAC 3
#define BYTES_MDAC_SPI_BUFFER ( NUM_MDACS * BYTES_PER_MDAC )

extern uint16_t* channels[NUM_MDACS][CHANS_PER_MDAC]; // three dual DAC chips
extern byte spi_msg[BYTES_MDAC_SPI_BUFFER]; // three bytes per chip
extern bool newDACdata;
extern float resolver_DAC_angles[NUM_MDACS];

void angle2res(int channel, byte config, float value);

extern SPIClass mySpi;

enum menu_pwm_config_ {
    pwm_disable,
    pwm_synchro,
    pwm_resolver,
    pwm_sineCha,
    pwm_sineChb,
    pwm_reference,
    dac_resolver
};

// single PWM channel config
typedef struct {
  uint8_t       channel;        // 1
  uint8_t       config;         // 1
  uint16_t      amplitude;      // 2
  float         value;          // 4
} Channel_t;

// data to send to client using I2C 
typedef struct  {
  uint32_t config;              // 4
  Channel_t channels[12];       // 12 * 8 = 96
  uint16_t  phase_offset;       // 2
} t_I2cTxStruct;                // = 102

// data to receive from client using I2C
typedef struct  {
  uint32_t status;      //  4
  float gen_frequency;  //  4
  float syn_frequency;  //  4
  float adc_angle;      //  4
                        //------
                        // 16
  byte padding[32];
} t_I2cRxStruct;

extern t_I2cTxStruct txData; 
extern bool newDACdata;

extern void SPI_setup(void);
extern void SPI_write(void);
extern void SPI_update(byte* pmsg, int index, uint16_t chan1, uint16_t chan2);
extern void SPI_process(void);
