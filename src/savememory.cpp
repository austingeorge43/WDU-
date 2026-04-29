#include <Arduino.h>
#include "Ext_Var.h"

#include <EEPROM.h>


// #define PRODUCT_SELECTION 0
// #define SUBPRODUCT_SELECTION 1
// #define SAFETY_TEMP 3
// #define CALIBRATION_VALUE 5
// #define PROBE_ERROR 9

//----------DEFINATIONS

eepromclass::eepromclass()
{}

void eepromclass::eeprom_defaultvalue()                 // Factory Reset Condition
{
    counter=0.0;
    dduflag=0;
    prodtypecounter=0;
    secondaryyes = 1;          // Secondary fill option enable/disable
    calibration_value=1.4;    // Calibration value for 1.5L product, default product
    solenoidoverride=1;
    flowoverride=0;
    Heatersafteytemp=90;
    // temp_error=0.0;
    eeprom_object.eeprom_datawrite();

}

void eepromclass:: eeprom_update_sensor()
{
    EEPROM.update(SECONDARY_FILL, secondaryyes);

    EEPROM.update(FLOW_CONTROL, flowoverride);

    EEPROM.update(LEVEL_CONTROL, leveloverride);

    EEPROM.update(SOLENOID_CONTROL, solenoidoverride);

    EEPROM.update(PROBE_CONTROL, probeoverride);
}

void eepromclass:: eeprom_datawrite()
{
//   prodtypecounter=1;
//   calibration_value=0.1;
//   Heatersafteytemp=50;
//      temp_error=0.0;
    EEPROM.write(PRODUCT_SELECTION, dduflag);
    
    EEPROM.put(SUBPRODUCT_SELECTION,prodtypecounter);
    
    EEPROM.put(CALIBRATION_VALUE, calibration_value);
    
    EEPROM.put(SAFETY_TEMP, Heatersafteytemp);
 
    EEPROM.put(PROBE_ERROR, temp_error);



    EEPROM.put(SECONDARY_FILL, secondaryyes);

    EEPROM.put(FLOW_CONTROL, flowoverride);

    EEPROM.put(LEVEL_CONTROL, leveloverride);

    EEPROM.put(SOLENOID_CONTROL, solenoidoverride);

    EEPROM.put(PROBE_CONTROL, probeoverride);

    


}

void eepromclass:: eeprom_product_selection()
{
     EEPROM.write(PRODUCT_SELECTION, dduflag);
}

void eepromclass:: eeprom_subproduct_selection()
{
    EEPROM.put(SUBPRODUCT_SELECTION,prodtypecounter);
}

void eepromclass:: eeprom_calibration_value()
{
    EEPROM.put(CALIBRATION_VALUE, calibration_value);
}

void eepromclass:: eeprom_safety_temperature()
{
    EEPROM.put(SAFETY_TEMP, Heatersafteytemp);
}

void eepromclass:: eeprom_probe_error()
{
    EEPROM.put(PROBE_ERROR, temp_error);
}


void eepromclass :: eeprom_dataread()
{
    dduflag=EEPROM.read(PRODUCT_SELECTION);
    EEPROM.get(SUBPRODUCT_SELECTION,prodtypecounter);
    EEPROM.get(CALIBRATION_VALUE, calibration_value);
    EEPROM.get(SAFETY_TEMP, Heatersafteytemp);
    EEPROM.get(PROBE_ERROR, temp_error);

    EEPROM.get(SECONDARY_FILL, secondaryyes);

    EEPROM.get(FLOW_CONTROL, flowoverride);

    EEPROM.get(LEVEL_CONTROL, leveloverride);

    EEPROM.get(SOLENOID_CONTROL, solenoidoverride);

    EEPROM.get(PROBE_CONTROL, probeoverride);

    // Serial3.println(dduflag);
    // Serial3.println(calibration_value);

}

eepromclass eeprom_object= eepromclass();