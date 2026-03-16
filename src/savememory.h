#ifndef SAVEMEMORY_H
#define SAVEMEMORY_H

#include "Arduino.h"
 class eepromclass
 {
    public:
      eepromclass();
    void eeprom_dataread();
    void eeprom_datawrite();
    void eeprom_defaultvalue();

 };

 extern eepromclass eeprom_object;

#endif