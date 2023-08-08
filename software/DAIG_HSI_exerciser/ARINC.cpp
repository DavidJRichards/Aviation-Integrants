#include <Arduino.h>
#include <stdint.h>
#include "Arinc.h"

extern SerialUSB Serial;

ARINC429 dAR429;


// function copied from https://stackoverflow.com/questions/13247647/convert-integer-from-pure-binary-to-bcd
long bin2BCD(long binary) { // double dabble: 8 decimal digits in 32 bits BCD
  if (!binary) return 0;
  long bit = 0x4000000; //  99999999 max binary
  while (!(binary & bit)) bit >>= 1;  // skip to MSB

  long bcd = 0;
  long carry = 0;
  while (1) {
    bcd <<= 1;
    bcd += carry; // carry 6s to next BCD digits (10 + 6 = 0x10 = LSB of next BCD digit)
    if (bit & binary) bcd |= 1;
    if (!(bit >>= 1)) return bcd;
    carry = ((bcd + 0x33333333) & 0x88888888) >> 1; // carrys: 8s -> 4s
    carry += carry >> 1; // carrys 6s  
  }
}

word ARINC429_Permute8Bits(word label8bits)
{
  int i;
  word mask, result;

  result = 0;
  for(i = 0, mask = 0x80; i<8; i += 1, mask >>= 1)
    result |= ( (label8bits & mask) ? (1 << i) : 0 );
  return result;
}

word ARINC429_ComputeMSBParity(unsigned long ar429_L)
{
  int bitnum;
  int parity=0;
  unsigned long bitmask = 1L;

  // check all bits except parity bit
  for(bitnum = 0, bitmask = 1L; bitnum<31; bitnum += 1, bitmask <<= 1L)
  {
    if(ar429_L & bitmask)
      parity++; 
  }
  return (parity%2) ? 0 : 1;
}

unsigned long ARINC429_BuildCommand( unsigned char label, unsigned char sdi, unsigned long data, unsigned char ssm)
{

  dAR429.data = bin2BCD(data); // 19bits data
  dAR429.label = ARINC429_Permute8Bits(label);
  dAR429.sdi = sdi & 0x03;  
  dAR429.ssm = ssm & 0x03;  
  dAR429.parity = ARINC429_ComputeMSBParity(dAR429.ar429_L); // parity: MSB  

  return dAR429.ar429_L;
 }

void printbits(unsigned long value, word start, word finish, char delim)
{
  unsigned long bitnum;
  unsigned long bitmask;
#if 0
  bitmask = 1L << finish;
  for(bitnum = finish; bitnum+1 > start; bitnum -- , bitmask >>= 1)
    Serial.print(value & bitmask ? "1" : "0");
  
  Serial.print(delim);
#endif  
}

void ARINC429_PrintCommand( unsigned long ar429_L)
{
  int bitnum;
  unsigned long bitmask = 1L;

  dAR429.ar429_L = ar429_L;
#if 0
  Serial.print("parity: ");
  Serial.println(dAR429.parity, HEX);
  Serial.print("ssm: ");
  Serial.println(dAR429.ssm, HEX);
  Serial.print("BCD data: ");
  Serial.println(dAR429.data, HEX);
  Serial.print("sdi: ");
  Serial.println(dAR429.sdi, HEX);
  Serial.print("Label (oct): ");
  Serial.println(ARINC429_Permute8Bits(dAR429.label), OCT);

  Serial.print("Bitstream: ");
  printbits(dAR429.parity,0,0,',');
  printbits(dAR429.ssm,1,2,',');
  Serial.print('[');
  printbits(dAR429.data,16,18,',');
  printbits(dAR429.data,12,15,',');
  printbits(dAR429.data,8,11,',');
  printbits(dAR429.data,4,7,',');
  printbits(dAR429.data,0,3,']');
  Serial.print(',');
  printbits(dAR429.sdi,0,1,',');
  printbits(dAR429.label,0,7,'\n');
#endif
}


void transmit_bit(char bitvalue)
{
  if(bitvalue)
  {
    digitalWrite(PIN_Hi_429, HIGH);
    digitalWrite(PIN_Lo_429, LOW);
  }
  else
  {
    digitalWrite(PIN_Hi_429, LOW);
    digitalWrite(PIN_Lo_429, HIGH);
  }
  delayMicroseconds(24);
  digitalWrite(PIN_Hi_429, LOW);
  digitalWrite(PIN_Lo_429, LOW);
  delayMicroseconds(40);
}

void ARINC429_SendCommand( unsigned long ar429_L)
{
  int bitnum;
  unsigned long bitmask = 1L;

  digitalWrite(PIN_Debug,HIGH);
  // for all bits
  for(bitnum = 0, bitmask = 1L; bitnum<32; bitnum ++, bitmask <<= 1)
  {
    transmit_bit(ar429_L & bitmask ? 1 : 0);
  }  
  digitalWrite(PIN_Debug,LOW);
}
