#define PIN_Hi_429   14 //13 //7
#define PIN_Lo_429   22 //10
#define PIN_Debug   LED_BUILTIN

typedef union {
//    byte ar249_B[4];
//    word ar429_W[2];
    unsigned long ar429_L;
    struct 
    {
      unsigned long label:  8;  // 2,3,3 LSB
      unsigned long sdi:    2;  // source / destination identifier
      unsigned long data:  19;  // 
      unsigned long ssm:    2;  // sign / status matrix
      unsigned long parity: 1;  // odd parity MSB
    };
} ARINC429;

extern unsigned long ARINC_data;
extern long ARINC_value;     // value sent to ARINC display
extern void ARINC429_SendCommand( unsigned long ar429_L);
extern unsigned long ARINC429_BuildCommand( unsigned char label, unsigned char sdi, unsigned long data, unsigned char ssm);
