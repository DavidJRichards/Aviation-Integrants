/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#ifndef MENU_GENERATED_CODE_H
#define MENU_GENERATED_CODE_H

#include <Arduino.h>
#include <tcMenu.h>
#include "tcMenuTfteSpi.h"
#include "EthernetTransport.h"
#include <RemoteConnector.h>
#include <RuntimeMenuItem.h>
#include <EditableLargeNumberMenuItem.h>
#include <IoAbstraction.h>
#include <IoAbstractionWire.h>
#include <EepromItemStorage.h>
#include <EepromAbstractionWire.h>

// variables we declare that you may need to access
extern const PROGMEM ConnectorLocalInfo applicationInfo;
extern TcMenuRemoteServer remoteServer;
extern TFT_eSPI gfx;
extern TfteSpiDrawable gfxDrawable;
extern GraphicsDeviceRenderer renderer;
extern WiFiServer server;
extern EthernetInitialisation ethernetInitialisation;

// Any externals needed by IO expanders, EEPROMs etc
extern IoAbstractionRef ioexp_iox;

// Global Menu Item exports
extern FloatMenuItem menuFREQGen;
extern FloatMenuItem menuFREQSyn;
extern FloatMenuItem menuPWM10;
extern FloatMenuItem menuPWM9;
extern FloatMenuItem menuPWM8;
extern FloatMenuItem menuPWM11;
extern FloatMenuItem menuPWM7;
extern FloatMenuItem menuPWM6;
extern FloatMenuItem menuPWM5;
extern FloatMenuItem menuPWM4;
extern FloatMenuItem menuPWM3;
extern FloatMenuItem menuPWM2;
extern FloatMenuItem menuPWM1;
extern FloatMenuItem menuPWM0;
extern FloatMenuItem menuADC2Voltage;
extern FloatMenuItem menuADC1Voltage;
extern FloatMenuItem menuSynchroAngle;
extern AnalogMenuItem menuPWMChannelCH11;
extern AnalogMenuItem menuPWMChannelCH10;
extern AnalogMenuItem menuPWMChannelCH9;
extern AnalogMenuItem menuPWMChannelCH8;
extern AnalogMenuItem menuPWMChannelCH7;
extern AnalogMenuItem menuPWMChannelCH6;
extern AnalogMenuItem menuPWMChannelCH5;
extern AnalogMenuItem menuPWMChannelCH4;
extern AnalogMenuItem menuPWMChannelCH3;
extern AnalogMenuItem menuPWMChannelCH2;
extern AnalogMenuItem menuPWMChannelCH1;
extern AnalogMenuItem menuPWMChannelCH0;
extern BackMenuItem menuBackSynchroChannel;
extern SubMenuItem menuSynchroChannel;
extern EnumMenuItem menuPWMConfigCH11;
extern EnumMenuItem menuPWMConfigCH10;
extern EnumMenuItem menuPWMConfigCH9;
extern EnumMenuItem menuPWMConfigCH8;
extern EnumMenuItem menuPWMConfigCH7;
extern EnumMenuItem menuPWMConfigCH6;
extern EnumMenuItem menuPWMConfigCH5;
extern EnumMenuItem menuPWMConfigCH4;
extern EnumMenuItem menuPWMConfigCH3;
extern EnumMenuItem menuPWMConfigCH2;
extern EnumMenuItem menuPWMConfigCH1;
extern EnumMenuItem menuPWMConfigCH0;
extern BackMenuItem menuBackSynchroConfig;
extern SubMenuItem menuSynchroConfig;
extern EnumMenuInfo minfoLED8;
extern EnumMenuItem menuLED8;
extern EnumMenuInfo minfoLED7;
extern EnumMenuItem menuLED7;
extern EnumMenuInfo minfoLED6;
extern EnumMenuItem menuLED6;
extern EnumMenuInfo minfoLED5;
extern EnumMenuItem menuLED5;
extern EnumMenuInfo minfoLED4;
extern EnumMenuItem menuLED4;
extern EnumMenuInfo minfoLED3;
extern EnumMenuItem menuLED3;
extern EnumMenuInfo minfoLED2;
extern EnumMenuItem menuLED2;
extern EnumMenuInfo minfoLED1;
extern EnumMenuItem menuLED1;
extern EnumMenuInfo minfoDACDC3;
extern EnumMenuItem menuDACDC3;
extern EnumMenuInfo minfoDACDC2;
extern EnumMenuItem menuDACDC2;
extern EnumMenuInfo minfoDACDC1;
extern EnumMenuItem menuDACDC1;
extern EnumMenuInfo minfoDACAMP;
extern EnumMenuItem menuDACAMP;
extern EnumMenuInfo minfoDACSolenoid2;
extern EnumMenuItem menuDACSolenoid2;
extern EnumMenuInfo minfoDACSolenoid1;
extern EnumMenuItem menuDACSolenoid1;
extern EnumMenuInfo minfoDACLamp;
extern EnumMenuItem menuDACLamp;
extern EnumMenuInfo minfoDACRelay;
extern EnumMenuItem menuDACRelay;
extern EnumMenuInfo minfoDACFlag2;
extern EnumMenuItem menuDACFlag2;
extern EnumMenuInfo minfoDACFlag1;
extern EnumMenuItem menuDACFlag1;
extern EnumMenuInfo minfoDACGalv3;
extern EnumMenuItem menuDACGalv3;
extern EnumMenuInfo minfoDACGalv2;
extern EnumMenuItem menuDACGalv2;
extern EnumMenuInfo minfoDACGalv1;
extern EnumMenuItem menuDACGalv1;
extern EnumMenuInfo minfoMapAbsoluteSet;
extern EnumMenuItem menuMapAbsoluteSet;
extern EnumMenuInfo minfoPWMChan11;
extern EnumMenuItem menuPWMChan11;
extern EnumMenuInfo minfoPWMChan10;
extern EnumMenuItem menuPWMChan10;
extern EnumMenuInfo minfoPWMChan9;
extern EnumMenuItem menuPWMChan9;
extern EnumMenuInfo minfoPWMChan8;
extern EnumMenuItem menuPWMChan8;
extern EnumMenuInfo minfoPWMChan7;
extern EnumMenuItem menuPWMChan7;
extern EnumMenuInfo minfoPWMChan6;
extern EnumMenuItem menuPWMChan6;
extern EnumMenuInfo minfoPWMChan5;
extern EnumMenuItem menuPWMChan5;
extern EnumMenuInfo minfoPWMChan4;
extern EnumMenuItem menuPWMChan4;
extern EnumMenuInfo minfoPWMChan3;
extern EnumMenuItem menuPWMChan3;
extern EnumMenuInfo minfoPWMChan2;
extern EnumMenuItem menuPWMChan2;
extern EnumMenuInfo minfoPWMChan1;
extern EnumMenuItem menuPWMChan1;
extern EnumMenuInfo minfoPWMChan0;
extern EnumMenuItem menuPWMChan0;
extern BackMenuItem menuBackRoutingTable;
extern SubMenuItem menuRoutingTable;
extern ActionMenuItem menuEepromSave;
extern AnalogMenuItem menuPhaseOffset;
extern AnalogMenuItem menuRmsNominal;
extern AnalogMenuItem menuReferenceAmplitude;
extern AnalogMenuItem menuSynchroAmplitude;
extern AnalogMenuItem menuEEPAngle5;
extern AnalogMenuItem menuEEPAngle4;
extern AnalogMenuItem menuEEPAngle3;
extern AnalogMenuItem menuEEPAngle2;
extern AnalogMenuItem menuEEPAngle1;
extern AnalogMenuItem menuEEPValue5;
extern AnalogMenuItem menuEEPValue4;
extern AnalogMenuItem menuEEPValue3;
extern AnalogMenuItem menuEEPValue2;
extern AnalogMenuItem menuEEPValue1;
extern BooleanMenuItem menuEEPDigital5;
extern BooleanMenuItem menuEEPDigital4;
extern BooleanMenuItem menuEEPDigital3;
extern BooleanMenuItem menuEEPDigital2;
extern BooleanMenuItem menuEEPDigital1;
extern BackMenuItem menuBackEEPROMData;
extern SubMenuItem menuEEPROMData;
extern FloatMenuItem menuCalRatio2;
extern AnalogMenuItem menuScale1;
extern AnalogMenuItem menuScale0;
extern EditableLargeNumberMenuItem menuMapRatio1;
extern EditableLargeNumberMenuItem menuMapRatio0;
extern FloatMenuItem menuMapCoarse;
extern AnalogMenuItem menuMAPCoarse;
extern FloatMenuItem menuMapMedium;
extern AnalogMenuItem menuMAPMedium;
extern FloatMenuItem menuMapFine;
extern AnalogMenuItem menuMAPFine;
extern EditableLargeNumberMenuItem menuMapAbsolute;
extern AnalogMenuItem menuMapHeading;
extern AnalogMenuItem menuMapNtoS;
extern AnalogMenuItem menuMapPosition;
extern BackMenuItem menuBackMovingMap;
extern SubMenuItem menuMovingMap;
extern ActionMenuItem menuEepromLoad;
extern AnalogMenuItem menuEncoder;

// Provide a wrapper to get hold of the root menu item and export setupMenu
inline MenuItem& rootMenuItem() { return menuEncoder; }
void setupMenu();

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

void CALLBACK_FUNCTION cb_absolute(int id);
void CALLBACK_FUNCTION cb_coarse(int id);
void CALLBACK_FUNCTION cb_encoder(int id);
void CALLBACK_FUNCTION cb_fine(int id);
void CALLBACK_FUNCTION cb_heading(int id);
void CALLBACK_FUNCTION cb_medium(int id);
void CALLBACK_FUNCTION cb_ntos(int id);
void CALLBACK_FUNCTION cb_phase(int id);
void CALLBACK_FUNCTION cb_position(int id);
void CALLBACK_FUNCTION cb_ratio(int id);
void CALLBACK_FUNCTION cb_scale(int id);
void CALLBACK_FUNCTION cb_voltage(int id);
void CALLBACK_FUNCTION eeprom_load(int id);
void CALLBACK_FUNCTION eeprom_save(int id);

#endif // MENU_GENERATED_CODE_H
