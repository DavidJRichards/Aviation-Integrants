#define USE_I2C

extern void showNewData(void);
extern void updateDataToSend(void);
extern void i2c_setup(void);

typedef struct {
  uint8_t       channel;
  uint8_t       config;
  uint16_t      amplitude;
  float         value;
} Channel_t;

