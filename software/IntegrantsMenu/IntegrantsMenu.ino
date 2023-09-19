#include <WiFi.h>
#include "Wifi.h"
#include "IntegrantsMenu_menu.h"


#include <XPT2046_Touchscreen.h>

#include <IoAbstractionWire.h>
const int encoderA = 14;
const int encoderB = 13;
const int encoderOK = 15;
const int ledA = 0;
const int ledB = 1;
//const int attachedInterruptPin = 35;
//MCP23017IoAbstraction mcp23017(0x20, ACTIVE_LOW_OPEN,  attachedInterruptPin, IO_PIN_NOT_DEFINED);
MCP23017IoAbstraction mcp23017(0x20, ACTIVE_LOW_OPEN, IO_PIN_NOT_DEFINED);

static uint16_t const SCREEN_WIDTH    = 320;
static uint16_t const SCREEN_HEIGHT   = 240;

extern XPT2046_Touchscreen touchDevice;

enum class PointID { NONE = -1, A, B, C, COUNT };

// source points used for calibration
static TS_Point _screenPoint[] = {
  TS_Point( 13,  11), // point A
  TS_Point(312, 113), // point B
  TS_Point(167, 214)  // point C
};


// touchscreen points used for calibration verification
static TS_Point _touchPoint[] = {
  TS_Point(3559, 3669), // point A
  TS_Point( 269, 2164), // point B
  TS_Point(1834,  598), // point C
};


static TS_Calibration cal(
    _screenPoint[(int)PointID::A], _touchPoint[(int)PointID::A],
    _screenPoint[(int)PointID::B], _touchPoint[(int)PointID::B],
    _screenPoint[(int)PointID::C], _touchPoint[(int)PointID::C],
    SCREEN_WIDTH,
    SCREEN_HEIGHT
);


void setup() {
    Wire.begin(22,27);
//    Wire.begin(18,23);
    Wire.setClock(400000);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    setupMenu();
//    touchDevice.calibrate(cal);


}

int encoder_pos;

LargeFixedNumber* large_absolute; //menuMapAbsolute.getLargeNumber();
double absolute;
#define DEFAULT_COARSE_OFFSET -0  //-90
#define DEFAULT_MEDIUM_OFFSET 0   //65
#define DEFAULT_FINE_OFFSET 0

int coarse_offset = DEFAULT_COARSE_OFFSET;  // film at left hand end of roll, abs 0
int medium_offset = DEFAULT_MEDIUM_OFFSET;  //
int fine_offset = DEFAULT_FINE_OFFSET;

// these constants represent the gearing between the resolvers
const float ratio0 = (32.2727272727 / 1.00979);              // medium to coarse
const float ratio2 = (1041.5289256198 / 1.00979 / 1.00333);  //ratio1*30; // fine to coarse
const float ratio1 = (ratio2 / ratio0);                      //30;        // fine to medium

void loop() {
    taskManager.runLoop();
    
//    large_absolute=menuMapAbsolute.getLargeNumber();
//    absolute=large_absolute->getAsFloat();

        menuMapFine.setFloatValue  (fmod((absolute         ) +   fine_offset, 360)); // fine
        menuMapMedium.setFloatValue(fmod((absolute / ratio1) + medium_offset, 360)); // medium
        menuMapCoarse.setFloatValue(fmod((absolute / ratio2) + coarse_offset, 360)); // coarse

}

void CALLBACK_FUNCTION eeprom_load(int id) {
    menuMgr.load();
}


void CALLBACK_FUNCTION eeprom_save(int id) {
    menuMgr.save();
}



void CALLBACK_FUNCTION cb_encoder(int id) {
  encoder_pos=menuEncoder.getCurrentValue()-2048;
}


void CALLBACK_FUNCTION cb_absolute(int id) {
//    large_absolute=menuMapAbsolute.getLargeNumber();
//    absolute=large_absolute->getAsFloat();
    absolute=menuMapAbsolute.getLargeNumber()->getAsFloat();
    menuMapFine.setFloatValue  (fmod((absolute         ) +   fine_offset, 360)); // fine
    menuMapMedium.setFloatValue(fmod((absolute / ratio1) + medium_offset, 360)); // medium
    menuMapCoarse.setFloatValue(fmod((absolute / ratio2) + coarse_offset, 360)); // coarse
}

//#define AMPLITUDE_FS 14.23
//#define DIV_FACT 560

//float div_fact, amplitude_nom;

void CALLBACK_FUNCTION cb_voltage(int id) {
    // TODO - your menu change code
  //div_fact = menuRmsFactor.getCurrentValue();
  //amplitude_nom = menuRmsNominal.getCurrentValue();
  float value = menuRmsNominal.getCurrentValue() * 100.0 / menuReferenceAmplitude.getCurrentValue();
}



void CALLBACK_FUNCTION cb_ratio(int id) {
    // TODO - your menu change code
    double ratio2;
    ratio2 = menuMapRatio0.getLargeNumber()->getAsFloat() * menuMapRatio1.getLargeNumber()->getAsFloat();
    menuCalRatio2.setFloatValue(ratio2);
}



void CALLBACK_FUNCTION cb_phase(int id) {
    // TODO - your menu change code
}
