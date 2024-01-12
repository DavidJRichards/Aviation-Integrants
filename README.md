# Aviation-Integrants
Modular microprocessor circuits for exercising aviation instruments

## Modules

[Control Menu](./software/readme.md)

[MCP23017-Digital-I/O](./hardware/MCP23017-Digital-IO.md)

[MAX532-MDAC](./hardware/MAX532-MDAC.md)

[PCA9685-PWM-Power](./hardware/PCA9685-PWM-Power.md)

[ADS1115-ADC](./hardware/ADS1115-ADC.md)

[TDA7294-Amp](./hardware/TDA7294-Amp.md)

[LM1875-Amp](./hardware/LM1875-Amp.md)

## Videos

[#1 Aviation Integrants Introduction](https://youtu.be/Xt561YlcXEw)

[#2 Aviation Integrants 400 Hz PWM](https://youtu.be/oUfFnTuM4sA)

[#3 Aviation Integrants flight demo](https://youtu.be/FEqSL34pKfE)

## [Photos](./images/README.md)

## [Integrants Software](./software)

### Dual processor Integrated master / client system

projects now build in VSCode/PlatformIO

Master using ESP32 CYD tft touch display

[Integrants I2C master](./software/IntegrantsMaster)

Client using RP2040 PWM generators

[Integrants I2C client](./software/IntegrantsCient)

### H6 Horizon driver - initial versions

[HSI Exerciser](./software/backup/DIAG_HSI_Exerciser)

[HyperIMU plotter](./software/backup/picow_hyperimu_plotter)

[Wifi server](./software/backup/DAIG_i2c_slave)

## Videos

[HSI Exercisor](https://youtube.com/shorts/Rt51kpNWBBE)

[H6 HyperIMU](https://youtube.com/shorts/gqmefr7U4pM)

## [Documents](./documents)

[Node-Red MQTT](./documents/node-red-mqtt.md)

[Forth system](./documents/Forth-system.md)

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

## I2C device test

Output from arduino I2C device scanner sketch with device name annotation

```
Scanning for I2C devices ...
I2C device found at address 0x20    MCP2013 encoder etc
I2C device found at address 0x30    RP2040 client  
I2C device found at address 0x40    PCA9685 PWM DAC
I2C device found at address 0x48    ADS1115 ADC
I2C device found at address 0x50    eeprom
I2C device found at address 0x51    "   "
I2C device found at address 0x52    "   "
I2C device found at address 0x53    "   "
I2C device found at address 0x54    "   "
I2C device found at address 0x55    "   "
I2C device found at address 0x56    "   "
I2C device found at address 0x57    "   "
I2C device found at address 0x70    pca9685 all
```



