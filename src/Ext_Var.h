#ifndef EXT_VAR_H
#define EXT_VAR_H


#include <LiquidCrystal_I2C.h>

#include "Lcd_Display.h"
#include <OneButton.h>
#include <EEPROM.h>
#include "Ticker.h"
#include <Wire.h>

#include "Button.h"
#include "Lcd_Display.h"
#include "Buzzer.h"
#include "process.h"
#include "RTD.h"
#include "savememory.h"

#define BUZZER 18
#define SOLENOID1 13
#define SOLENOID2 12

#define PRODUCT_SELECTION 0
#define SUBPRODUCT_SELECTION 1
#define SAFETY_TEMP 3
#define CALIBRATION_VALUE 5
#define PROBE_ERROR 9

#define SECONDARY_FILL 15
#define FLOW_CONTROL 17
#define LEVEL_CONTROL 19
#define SOLENOID_CONTROL 21
#define PROBE_CONTROL 23


extern int prodtype[3];
extern int  optime[4];



extern bool toggle;
extern bool usersettings;
extern bool servicemenu;
extern bool mainscreenflag;
extern bool uppointer;
extern bool downpointer;
extern bool inmenu;
extern bool secondaryyes;
extern bool solenoidoverride;
extern bool flowoverride;
extern int one_second_counter;
extern bool process_flag;
extern bool flow_error_checkflag;
extern bool error_check_flag;
extern bool secondarytimerflag;
extern bool pauseflag;
extern bool waterlevel_error_flag;
extern bool dduflag;
extern bool factoryresetflag;
extern bool leveloverride;
extern bool probeoverride;
extern bool closetap;
extern bool  temp_drop_flag;
extern bool time_skip;
extern bool primary_filling_flag;
extern bool zero_calib;

extern float calib_Heater1;
extern float temp_error;
extern float Max_liter;
extern int optimecounter;
extern int prodtypecounter;
extern float calibration_value;
extern float counter;
extern int screen;
extern float Heater_temp;
extern int variant;
extern int pre_end_time;
extern int end_time;
extern uint16_t mint;
extern uint8_t Probe1_Err;
extern int remaining_volume;
extern int u_input;
extern float time_per_step;
extern int previous_time;
extern int Heatersafteytemp;
extern unsigned long solenoid_stop;

extern uint8_t skip_count;
extern uint16_t time_counter2;
extern int one_second_counter2;


enum menus
{
    VersionScreen,
    MainScreen,

    ServiceMenuScreen1,
    ServiceMenuScreen2,
    ServiceMenuScreen3,
    ServiceMenuScreen4,
    ServiceMenuScreen5,
    SDUServiceMenuScreen3,

    UserSettingsScreen1,
    UserSettingsScreen2,
    UserSettingsScreen3,
    UserSettingsScreen4,
    // UserSettingsScreen5,
    // UserSettingsScreen6,
    // UserSettingsScreen7,
    // UserSettingsScreen8,

    LevelSensorSettings,
    TempSensorSettings,

    
    OperatingTimeSettings,
    ProductTypeSettings,
    SubProductTypeSettings,
    CalibrationSettings,
    SolenoidControlSettings,
    FlowControlSettings,

    SecondaryFillSettings,
    SafteyTemperatureSettings,
    ProbeCalibrationSettings,

    FactoryResetScreen,

    ProcessScreen,
    PrimaryFillScreen,
    SecondaryFillTimer,
    
    // SolenoidErrorScreen,
    ErrorScreen
};

#endif