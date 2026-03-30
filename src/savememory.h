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
    void eeprom_update_sensor();

    void eeprom_product_selection();
    void eeprom_subproduct_selection();
    void eeprom_calibration_value();
    void eeprom_safety_temperature();
    void eeprom_probe_error();

 };

 extern eepromclass eeprom_object;

#endif