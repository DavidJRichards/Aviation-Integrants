# Aviation-Integrants
Modular microprocessor circuits for exercising aviation instruments


## [Photos](./images)

## [Software](./software)

### Dual processor Integrated master / client system

Master using ESP32 CYD tft touch display

[Integrants I2C master](./software/CYD_I2C_master)

Client using RP2040 PWM generators

[Integrants I2C client](./software/MULTI_I2C_client)

### H6 Horizon driver - initial versions

[HSI Exerciser](./software/DIAG_HSI_Exerciser)

[HyperIMU plotter](./software/picow_hyperimu_plotter)

[Wifi server](./software/DAIG_i2c_slave)

## Videos

[HSI Exercisor](https://youtube.com/shorts/Rt51kpNWBBE)

[H6 HyperIMU](https://youtube.com/shorts/gqmefr7U4pM)

## [Documents](./documents)

## Driver modules

### PWM

Synthesise servo transmitter signals to drive audio amplifiers

### ARINC

ARINC transmitter software to suit MAX3232 driver

### Synchro

Measure synchro rotor voltage and convert to angle

### Compass

TFT_eSPI compass rose display with two needle

s and numbers

### Encoder

Reads rotary encoder with acceleration

### Differential ADC

Reads output potentiometers on horizon gyro

### PWM DAC

Provides 16 channels analogue/digital output with 3 volt and 24 volt hi-side driver

### 400 Hz Zero crossing detector

Synchronise internal generators with external power supply

### 400 Hz frequency measurement

Measure external or internal frequency

### 7 segment LED displays

Show measured values

### pico-w json server

process Wifi messages received in Json format



