# djrm Aviation Integrants

### Dual processor Integrated master / client system

#### Designed to offer flexible mapping of inputs to outputs and monitoring

The configuration changes made in the editor can be saved in an eeprom for later use.

Additional i/o modules can be easily be added to the system with minor changes to the master software.

The main user interface is used to control of choice of outputs receiving data from an input. 

Ongoing development will add function modules for more sophisticated control functions - e.g. for the moving map display module where one input will need to have calculations performed and to then drive multiple outputs. The function modules will appear in the menus in a similar way to the existing i/o devices.

#### Moving map

Menu and MQTT inputs are available to set the position of the moving map display, the horizontal movement needs to be sent to the map unit as three resolver values, these are calculated from the absolute map position variable.

#### System architecture

The software relies heavily on pre-built Arduino libraries, The main ones used are for the menu, wifi communications, and the i/o devices.

##### Libraries used:

|Function   |     |
|-----------|-----|
|TcMenu     |     |
|TFT_eSPI   |     |
|Wifi       |     |
|Pangolin MQTT|   |
|ArduinoJSON |    |
|Ticker      |   |
|Wire        |   |
|MPC23017    |   |
|ADS1115_WE  |   |
|PCA9685     |   |
|SPI         |   |
|Seg7        |   |
|TM1637      |   |
|ADCInput    |   |
|Arinc429    |   |

Master using ESP32 CYD tft touch display

[Integrants I2C master](./CYD_I2C_master)

Client using RP2040 PWM generators

[Integrants I2C client](./MULTI_I2C_client)

Latest version of sensor and driver software integration using TcMenu 

![TcMenu](../images/TcMenuExample.jpg)

[IntegrantsMenu](./IntegrantsMenu)

Menu structure tested in this project prior to copying into CYD_I2C_Master for normal use

![Alt text](../images/embedCONT.png)

## Physical

The system is powered from 28 volts DC which is used by the DAC output and a 5 volt regulator to supply the microcontrollers and peripheral devices.

PWM outputs are low voltage AC outputs which are fed into an array of audio amplifiers which are seperately powered from a 15-0-15 VAC mains transformer

An optional 115 Volt 400 Hz input is used to generate a synchronizing signal to the system synchro outputs. 

Operation without synchronising input is possible if the built-in reference generator is used and its output fed to the target system.

Three ADC channels on the RP2040 are used to measure the angle of an attached Synchro transmitter.

All system values are visible on the TFT menu display, the information is also transmitted as MQTT data and can de displayed on a PC using, for example, using Node Red dashboard or MQTT explorer.

The configuration menu is editable using the system rotary encoder, the menu protocol is accesible over wifi and also controllable using a PC application.
 

### Outputs

16 400 Hz variable purpose drives

16 DC DAC drives

8 LED Indicators

8 Digit 7 Segment display

Single Arinc429 transmitter

### Inputs

Rotary encoder manual

Synchro Receiver ADC with angle decode

28V 400Hz synchronizing input

ADC for horizon gyro angle voltage

8 DIP switches

28 Volts DC Power for DAC outputs

### MQTT

#### Published

System status

PWM status

DIP switch status

LED status

Moving Map status

#### Subscribed

HyperIMU phone position sensor

MovingMap Absolute value & Fine, Medium, Coarse angles

Angle inputs (0 to 4096)

Analogue inputs ( -180 to 180 )

Digital inputs ( 0 or 1 )

### JSON topics

Sample MQTT messages in JSON format

```
integrants
    pub
        PWMstatus = {""CHAN0": 0.0, "CHAN1": 0.0, "CHAN2": 0.0, "CHAN3": -1.0, "CHAN4": 231.2, "CHAN5": 231.2 "CHAN6": 0.0, "CHAN7": 0.0, "CHAN8": 0.0, "CHAN9": 0.0, "CHAN10": 2.5, "CHAN11": 1.0}
        DIPstatus = {"DIP1":0, "DIP2":1, "DIP3":1, "DIP4":0, "DIP5":1, "DIP6":1, "DIP7":1, "DIP8":0}
        LEDstatus = {"LED1":0, "LED2":1, "LED3":1, "LED4":0, "LED5":1, "LED6":1, "LED7":1, "LED8":0}
        SYSstatus = {"Encoder":0, "ADCAngle": 231.2, "ADC1": 2.5, "ADC2": 1.0, "Hz-gen": 403, "Hz-syn": 0}
    sub
        HyperIMU = {"os":"hyperimu","orientation":[94.61526489257812,-37.457916259765625,-6.3306684494018555]}
```

### Virtual I/O

#### Inputs

Items selectable in configuration menus

Angle inputs

Value inputs

Digital inputs

Encoder position

Absolute map position


## PWM Configuration

There are 16 PWM channels normally arranged in pairs to drive amplifiers to simulate synchro or resolvers or other 400 Hz signals.

Each channel is configured using one of 12 parameter sets, giving access to:
 * Function
 * Data source
 * Rotation angle
 * Maximum amplitude
 
### Functions available

|Function |     |     |
|---------|-----|-----|
|Disabled |     |     |
|Synchro  |     |     |
|Resolver |     |     |
|Sine_ChA |     |     |
|Sine_ChB |     |     |
|Reference|     |     |

## Input sources

|Source |                    |
|-------|--------------------|
|Default|                    |
|ENCODER_Pos|                |
|FERQ_Gen|Internal frequency |
|FREQ_Syn|External frequency |
|ADC_Value1|Horizon Pitch    |
|ADC_Value2|Horizon Roll     |
|IMU_Pitch|HyperIMU          |
|IMU_Roll|HyperIMU           |
|IMU_Heading|HyperIMU        |
|ADC_Angle  |Synchro receiver|
|DIP_SW1||
|DIP_SW2||
|DIP_SW3||
|DIP_SW4||
|DIP_SW5||
|DIP_SW6||
|DIP_SW7||
|DIP_SW8||
|MQTT_Value1|0 to 4096       |
|MQTT_Value2|                |
|MQTT_Value3|                |
|MQTT_Value4|                |
|MQTT_Value5|                |
|MQTT_Digit1|0 to 1          |
|MQTT_Digit2|                |
|MQTT_Digit3|                |
|MQTT_Digit4|                |
|MQTT_Digit5|                |
|MQTT_Digit6|                |
|MQTT_Digit7|                |
|MQTT_Digit8|                |
|EEP_Digit1|0 to 1 (0)       |
|EEP_Digit2|                 |
|EEP_Digit3|                 |
|EEP_Digit4|                 |
|EEP_Digit5|                 |
|EEP_Value1|0 to 4096 (2048) |
|EEP_Value2|                 |
|EEP_Value3|                 |
|EEP_Value4|                 |
|EEP_Value5|                 |
|EEP_Angle1|-180 to 180 (0)  |
|EEP_Angle2|                 |
|EEP_Angle3|                 |
|EEP_Angle4|                 |
|EEP_Angle5|                 |
|MAP Absolute|value input         |
|MAP Fine    |Calculated from abs.|
|MAP Medium  |Calculated from abs.|
|MAP Coarse  |Calculated from abs.|

 
## Output sinks

|Sink         |               |
|-------------|---------------|
|PWM Chan0    |Pitch          |
|PWM1 Chan    |Roll           |
|PWM2 Chan    |Heading        |
|PWM3 Chan    |Reference      |
|PWM4 Chan    |               |
|PWM5 Chan    |               |
|PWM6 Chan    |spare          |
|PWM7 Chan    |spare          |
|PWM8 Chan    |spare          |
|PWM9 Chan    |also ARINC data|
|PWM10 Chan   |also 7 Seg LHS |
|PWM11 Chan   |also 7 Seg RHS |
|DAC Galv1    |250 uA Analogue|
|DAC Galv2    |250 uA Analogue|
|DAC Galv3    |250 uA Analogue|
|DAC Flag1    |5 volt Switch  |
|DAC Flag2    |5 volt Switch  |
|DAC Relay    |28 V PWM       |
|DAC Lamp     |28 V PWM       |
|DAC Solenoid1|28 V Switch    |
|DAC Solenoid2|28 V Switch    |
|DAC DC1      |28 V PWM       |
|DAC DC2      |28 V PWM       |
|DAC DC3      |28 V PWM       |
|DAC Amp      |28 V on/off    |
|LED1         |               |
|LED2         |               |
|LED3         |               |
|LED4         |               |
|LED5         |               |
|LED6         |               |
|LED7         |               |
|LED8         |               |

