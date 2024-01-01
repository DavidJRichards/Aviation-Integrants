# djrm Aviation Integrants

### Dual processor Integrated master / client system

#### Designed to offer flexible mapping of inputs to outputs and monitoring

The configuration changes made in the editor can be saved in an eeprom for later use.

Additional i/o modules can be easily be added to the system with minor changes to the master software.

The configuration implements a cross-point matrix of inputs connected to outputs, with a storage cell to configure each crossing.

The main user interface is used to control of choice of outputs receiving data from an input. A menu 'Routing Table' consists of a list of each output and choice is made as to which input it uses. A single input can feed more than one output if neded. 

For the synchro simulation channels there is an additional configuration to select the transformation used, synchro, resolver, single sin(0), etc. The values are all normalised into floating point anthough some kinds are integer or even boolean.

Ongoing development will add function modules for more sophisticated control functions - e.g. for the moving map display module where one input will need to have calculations performed and to then drive multiple outputs. The function modules will appear in the menus in a similar way to the existing i/o devices.

#### Moving map

Menu and MQTT inputs are available to set the position of the moving map display, the horizontal movement needs to be sent to the map unit as three resolver values, these are calculated from the absolute map position variable.

#### System architecture

The software relies heavily on pre-built Arduino libraries, The main ones used are for the menu, wifi communications, and the i/o devices.

[MAX532 MDAC interface](./MAX532-MDAC.md)

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
|MAX532      |   |

Master using ESP32 CYD tft touch display

[Integrants I2C master](./IntegrantsMaster)

Client using RP2040 PWM & MAX532 generators

[Integrants I2C client](./IntegrantsClient)

A version of sensor and driver software integration using TcMenu

![TcMenu](../images/TcMenuExample.jpg)

[IntegrantsMenu](./IntegrantsMenu)

Menu structure tested in this project prior to copying into CYD_I2C_Master for normal use

![Alt text](../images/embedCONT.png)

MQTT allows remote control and monitor of system, example shown here has IMU and moving map values:

![NodeRed](../images/MQTT_NodeRed.png)

## Physical

The system is powered from 28 volts DC which is used by the DAC output and a 5 volt regulator to supply the microcontrollers and peripheral devices.

PWM outputs are low voltage AC outputs which are fed into an array of audio amplifiers which are seperately powered from a 15-0-15 VAC mains transformer

An optional 115 Volt 400 Hz input is used to generate a synchronizing signal to the system synchro outputs. 

Operation without synchronising input is possible if the built-in reference generator is used and its output fed to the target system.

Three ADC channels on the RP2040 are used to measure the angle of an attached Synchro transmitter.

All system values are visible on the TFT menu display, the information is also transmitted as MQTT data and can de displayed on a PC using, for example, using Node Red dashboard or MQTT explorer.

The configuration menu is editable using the system rotary encoder, the menu protocol is accesible over wifi and also controllable using a PC application.
 

### Outputs

10 400 Hz variable purpose drives (synchro, resolver, etc)

6 400 Hz MAX 523 resolver output drivers(better precision for moving map)

16 DC DAC drives 

8 LED Indicators

8 Digit 7 Segment display

Single Arinc429 transmitter

### Inputs

Rotary encoder manual with press and two external buttons

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
 * Function - includes data representation and output hardware selection
 * Data source - input channel data is aquired from
 * Rotation angle - the main dynamic data
 * Maximum amplitude - scaling value for data

### Menu Structure

The main display consists of a multi line menu system, most items are editable although are display only
The main system configuration is through ther 'Routing Table' sub-menu where each output is assigned it data input source.
the 400Hz PWM channels require additional configuration to select the type of output and channel used.


 * Root menu,       Frequent use items
 * EEPROM Data,     Non-volatile items, Settings load & save
 * Routing Table,   Select inputs for ouput channel
 * Snchro Config,   Select data conversion for 400Hz PWM output
 * Synchro Channel, Select data channel for 400Hz PWM output

Eleven sysnchro outputs are available, these output channels require additional configuration to select the output conversion and hardware. It is this channel information which is transfered across the I2C interface to the Pico PWM output board.

Each 400Hz PWM output channel usually requires two outputs to drive synchros or resolvers. The Synchro config menu selects the type of output required, alternative output types are available for single channel output onto either of the channel pairs usually used. An additional kind of resolver output using I2C MDAC board instead of Pico PWM can also be selected. 

The 400 Hz PWM channels are fed to audio amplifiers capable of driving synchros and resolvers or other similar devices. The ten Pico PWM outputs are connected to six stereo LM amplifier pairs. The six MAX532 outputs are connected to six mono TDA amplifiers. The amplifiers are main powered via suitable transformers.

### 400Hz PWM Functions available

|Function    |Hardware|Outputs|
|------------|--------|-------|
|Disabled    |None    |0      |
|Synchro     |Pico    |0 to 5 |
|Resolver    |Pico    |0 to 5 |
|Sine_ChA    |Pico    |0 to 5 |
|Sine_ChB    |Pico    |0 to 5 |
|Reference   |Pico    |0 to 5 |
|Resolver DAC|MDAC    |0 to 2 |


#### Pico 400 Hz PWM outputs - 5 pairs available

| | |
|-|-|
|0 ChA|GPIO0|
|0 ChB|GPIO1|
|1 ChA|GPIO2|
|1 ChB|GPIO3|
|2 ChA|GPIO4|
|2 ChB|GPIO5|
|3 ChA|GPIO6|
|3 ChB|GPIO7|
|4 ChA|GPIO8|
|4 ChB|GPIO9|
|5 ChA|unimplemented|
|5 ChB|unimplemented|

### Resolver DAC 400Hz outputs - 3 pairs available

| | |
|-|-|
|0 ChA|MAX532-1a|
|0 ChB|MAX532-1b|
|1 ChA|MAX532-2a|
|1 ChB|MAX532-2b|
|2 ChA|MAX532-3a|
|2 ChB|MAX532-3b|



## Input sources

* Access float get_menuindex(int idx)
* uses menuXXX.getCurrentValue() for menu items
* or doc["XXX"][N].as<TYPE>(); for MQTT JSON value

|Index|Source |                    |Access|
|-----|-------|--------------------|-|
|0|Default|                    |
|1|ENCODER_Pos|                |encoder_pos=menuEncoder.getCurrentValue()-2048;|
|2|FERQ_Gen|Internal frequency |value = rxData.gen_frequency;
|3|FREQ_Syn|External frequency |value = rxData.syn_frequency;
|4|ADC_Value1|Horizon Pitch    |
|5|ADC_Value2|Horizon Roll     |
|6|IMU_Pitch|HyperIMU          |
|7|IMU_Roll|HyperIMU           |
|8|IMU_Heading|HyperIMU        |
|9|ADC_Angle  |Synchro receiver|
|10|DIP_SW1|                   |
|11|DIP_SW2|                   |
|12|DIP_SW3|                   |
|13|DIP_SW4|                   |
|14|DIP_SW5|                   |
|15|DIP_SW6|                   |
|16|DIP_SW7|                   |
|17|DIP_SW8|                   |
|18|MQTT_Value1|0 to 4096       |
|19|MQTT_Value2|                |
|20|MQTT_Value3|                |
|21|MQTT_Value4|                |
|22|MQTT_Value5|                |
|23|MQTT_Digit1|0 to 1          |
|24|MQTT_Digit2|                |
|25|MQTT_Digit3|                |
|26|MQTT_Digit4|                |
|27|MQTT_Digit5|                |
|28|MQTT_Digit6|                |
|29|MQTT_Digit7|                |
|30|MQTT_Digit8|                |
|31|EEP_Digit1|0 to 1 (0)       |
|32|EEP_Digit2|                 |
|33|EEP_Digit3|                 |
|34|EEP_Digit4|                 |
|35|EEP_Digit5|                 |
|36|EEP_Value1|0 to 4096 (2048) |
|37|EEP_Value2|                 |
|38|EEP_Value3|                 |
|39|EEP_Value4|                 |
|40|EEP_Value5|                 |
|41|EEP_Angle1|-180 to 180 (0)  |
|42|EEP_Angle2|                 |
|43|EEP_Angle3|                 |
|44|EEP_Angle4|                 |
|45|EEP_Angle5|                 |
|46|MAP Absolute|value input         |
|47|MAP Fine    |Calculated from abs.|
|48|MAP Medium  |Calculated from abs.|
|49|MAP Coarse  |Calculated from abs.|
|50|MAP Position|                    |
|51|MAP Heading |                    |
|52|MAP NtoS    |                    |

 
## Output sinks

todo: create general purpose write data function

// to remote synchros
 * pwm type config |    txData.channels[0].config = menuPWMConfigCH0.getCurrentValue();
 * pwm channel channel | txData.channels[0].channel = menuPWMChannelCH0.getCurrentValue();
 * pwm voltage config | txData.channels[0].amplitude =menuRmsNominal.getCurrentValue() * 100.0 / menuSynchroAmplitude.getCurrentValue();

// to local hardware
todo create readonly menu items for these
 * adc output setting | pwmController.setChannelPWM( 1, get_menuindex(menuDACGalv1.getCurrentValue() )); 
 * LED output setting | mcp23017.digitalWriteS(led1, get_menuindex(menuLED1.getCurrentValue() ));


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


## Confifuration example

400 Hz synchro / resolver outputs have three confiruration settings each, Output channel number, output channel config, output channel device index.

remaining outputs have only input configuration to set.


### Routing table (400Hz PWM)

|Channel No.|Routing Table|Synchro Config|Synchro Output|Notes|
|-----------|-------------|--------------|--------------|-----|
|PWM-0      |Map Fine     |MDAC Resolver |0             |Moving map|
|PWM-1      |Map Medium   |MDAC Resolver |1             |Moving map|
|PWM-2      |Map Coarse   |MDAC Resolver |2             |Moving map|
|PWM-3      |Map Heading  |Resolver      |3             |Moving map|
|PWM-4      |Map NtoS     |Resolver      |4             |Moving map|
|PWM-5      |ADC Angle    |Synchro       |0             |General purpose|
|PWM-6      |ADC Value 1  |Synchro       |1             |H6 Horizon from Gyro ADC|
|PWM-7      |ADC Vakue 2  |Synchro       |2             |H6 Horizon from Gryo ADC|
|PWM-8      |             |              |              ||
|9-Arinc    |Encoder Pos  |Disabled      |n/a           ||
|10-LED LHS |Map Heading  |Disabled      |n/a           ||
|11-LED RHS |Map NtoS     |Disabled      |n/a           ||


### Routing Table (remainder)

|Output        |Input      |setting|Notes       |
|--------------|-----------|-------|------------|
|MapAbsoluteSet|MapPosition|Value  |Notes       |
|DAC Galv1     |EEP Value5 |2048   |centre scale|
|DAC Galv2     |EEP Value5 |2048   |centre scale|
|DAC Galv3     |EEP Value5 |2048   |centre scale|
|DAC Flag      |EEP Digit5 |TRUE   |Flag On|
|DAC Relay     |EEP Value4 |4095   |Full power|
|DAC Lamp      |EEP Value4 |4095   |Full power|
|DAC Solenoid1 |EEP Value5 |2048   |half power|
|DAC Solenoid2 |EEP Value5 |2048   |half power|
|DAC Amp       |DIP SW1    |True   |On to enable amp|
|DAC DC1       |DIP SW2    |True   |On to enable map|
|DAC DC2       |DIP SW3    |True   |On to enable day|
|DAC DC3       |Default    |       |Demo Bulb|
|LED 1         |DIP SW1    |       |Lit when Amp enabled|
|LED 2         |DIP SW2    |       |Lit when Map enabled|
|LED 3         |DIP SW3    |       |Lit when Day enabled|
|LED 4         |Default    |||
|LED 5         |Default    |||
|LED 6         |Default    |||
|LED 7         |Default    |||
|LED 8         |Default    |||


## Special functions

### System frequency, FREQ_SYN, FREQ_SYN

 * Frequency of sampled waveform
 * Frequency of synchro simulation

The frequency to be sampled is processed and opto-coupled before use as edge triggered interrupt input.
The sampled frequency is also output as a pulse on a spare micro pin for scope sync use.

Synchro simulation frequency is locked to sampled input frequency when it is present

### Synchro angle read, ADC_Angle

Three wires fed into two ASC inputs to read position angle 0 to 360 degrees

### IMU_Pitch, IMU_Roll, IMU_Heading

Values received using MQTT from HyperIMU orientation protocol running on mobile phone

### Moving map horizontal position calculations

Absolute position converted into three seperate resolver angles 

#### MAP_FIne, MAP_Medium, MAP_Coarse

 *    absolute=menuMapAbsolute.getLargeNumber()->getAsFloat();
 *   menuMapFine.setFloatValue  (fmod((absolute         ) +   fine_offset, 360)); // fine
 *   menuMapMedium.setFloatValue(fmod((absolute / ratio1) + medium_offset, 360)); // medium
 *   menuMapCoarse.setFloatValue(fmod((absolute / ratio2) + coarse_offset, 360)); // coarse





