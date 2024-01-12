: postinc ( a -- x ) \ increment a, return its original value
  dup @ 1 rot +!
;

: enum
  create 0 ,
  does> postinc constant
;

enum    source

source  Default
source  Encoder
source  FREQ_Gen
source  FREQ_Syn
source  ADC1_Volts
source  ADC2_Volts
source  IMU_Pitch
source  IMU_Roll
source  IMU_Heading
source  ADC_Angle
source  DIP_SW1
source  DIP_SW2
source  DIP_SW3
source  DIP_SW4
source  DIP_SW5
source  DIP_SW6
source  DIP_SW7
source  DIP_SW8
source  MQTT_Value1
source  MQTT_Value2
source  MQTT_Value3
source  MQTT_Value4
source  MQTT_Value5
source  MQTT_Digit1
source  MQTT_Digit2
source  MQTT_Digit3
source  MQTT_Digit4
source  MQTT_Digit5
source  MQTT_Digit6
source  MQTT_Digit7
source  MQTT_Digit8
source  EEP_Digital1
source  EEP_Digital2
source  EEP_Digital3
source  EEP_Digital4
source  EEP_Digital5
source  EEP_Value1
source  EEP_Value2
source  EEP_Value3
source  EEP_Value4
source  EEP_Value5
source  EEP_Angle1
source  EEP_Angle2
source  EEP_Angle3
source  EEP_Angle4
source  EEP_Angle5
source  MAP_Absolute
source  MAP_Coarse
source  MAP_Medium
source  MAP_Fine
source  MAP_Heading
source  MAP_NtoS


