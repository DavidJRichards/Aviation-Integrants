#include "IntegrantsMenu_menu.h"

void setup() {
    setupMenu();

}

void loop() {
    taskManager.runLoop();

}



void CALLBACK_FUNCTION eeprom_load(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION eeprom_save(int id) {
    // TODO - your menu change code
}
