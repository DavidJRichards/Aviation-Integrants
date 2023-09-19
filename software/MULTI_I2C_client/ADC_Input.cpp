

#include <ADCInput.h>         // buffered ADC read stream

#define ADC_READ_FREQ    8000   // frequency of adc reading (per each channel)
#define ADC_READ_BUFFERS 2500
#define ADC_OFFSET       2064   // value with no input present

ADCInput ADCInputs(A0, A1, A2);

volatile float theta=0.0;
float adc_angle;                // value sent to display

//setup:
void adc_setup(void)
{
  ADCInputs.setFrequency(ADC_READ_FREQ);
  ADCInputs.setBuffers(3, ADC_READ_BUFFERS);
  ADCInputs.begin();
}


//loop:
void adc_loop(void)
{

  int sinVal;
  int cosVal;
  int refVal;

  float r2mr1;
  float s1ms3;
  float s3ms2;
  //float s2ms1;

  float sinin, cosin, delta, demod;
  int refsqwv;

  // run repeatedly:  read the ADC. ( 0 to 4095 )
  // make value signed +- 2048 by applying offset
  refVal=ADCInputs.read()-ADC_OFFSET; 
  sinVal=ADCInputs.read()-ADC_OFFSET;
  cosVal=ADCInputs.read()-ADC_OFFSET;

  // note that s2ms1 isn't needed
  // convert to float in range -2PI to +2PI
//r2mr1 =  refVal / (2048.0 / 2 * M_PI); // Vr2mr1 = -Vr1mr2 = -(Vredwht - Vblkwht)
  s1ms3 =  sinVal / (2048.0 / 2 * M_PI); // Vs1ms3 = Vylw    - Vblu
  s3ms2 =  cosVal / (2048.0 / 2 * M_PI); // Vs3ms2 = Vblu    - Vblk
//s2ms1 =                                   Vs2ms1 = Vblk    - Vylw  


  if (refVal > 0)
      refsqwv = 1;
  else
      refsqwv = -1;


#if 1 
  // scott t transform the synchro inputs
  sinin = s1ms3;
  // cosin = 2/sqrt(3) * (s3ms2 + 0.5 * s1ms3)
  cosin = 1.1547 * (s3ms2 - 0.5 * s1ms3);
#else  
  // or do nothing here if using resolver inputs
  sinin = s1ms3;
  cosin = s3ms2;
#endif  


  // compute error
  delta = sinin*cos(theta) - cosin*sin(theta);

  // demodulate AC error term
  demod = refsqwv * delta;

  // apply gain term to demodulated error term and integrate
  // theta = theta + 1/64*demod
  theta = theta + 0.015625*demod;

  // wrap from -pi to +pi
  theta = fmod((theta),(M_PI));
 // interrupt_process();

}

//access:
float adc_process(void)
{
  float angle;
    angle = theta*180/M_PI ;
    adc_angle = fmod(angle+180,360);
    return adc_angle;
}


