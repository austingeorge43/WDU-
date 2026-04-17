#include <Arduino.h>
#include "Ext_Var.h"

// ---------------- SAFETY LIMIT DEFINITIONS ----------------
#define MAX_SAFETY_TEMP 200          // Maximum allowed heater safety temperature
#define MIN_SAFETY_TEMP 50           // Minimum allowed heater safety temperature
#define SAFETY_TEMP_STEP 1           // Step increment/decrement for safety temperature
#define PROBE_CALIBRATION_LIMIT 20   // Max +/- limit for probe calibration error

// ---------------- LED PIN DEFINITIONS ----------------
#define RED_LED 17       //Heater ON indication LED
#define YELLOW_LED 39    // Standby / Warning indication LED
#define GREEN_LED 38     // Settings indication LED 

// ---------------- PROCESS CONFIGURATION ----------------
#define OPERATING_TIME 4   // Base operating time factor used for max volume calculation

// ---------------- GLOBAL VARIABLES ----------------

// Volume control
float counter = 0.0;            // User-selected output volume (in liters)
float Max_liter = 0.0;          // Maximum allowable volume based on variant

// Calibration & tuning
float calibration_value = 3.0;  // Calibration offset for system
float temp_error = 0.0;         // Probe temperature error adjustment
float speed = 0.5;              // Reserved / adjustable speed parameter
bool zero_calib = 0;             // Flag to indicate zero calibration condition

// Timing
long start_tt = 0;              // Used for detecting long press (UP + DOWN combo)

// ---------------- BUTTON & UI STATE FLAGS ----------------
bool dualPressActive = 0;                  // Helper flag for dual button press detection

bool mainscreenflag = 0;        // Indicates Main Screen is active
bool usersettings = 0;          // Indicates User Settings menu is active
bool servicemenu = 0;           // Indicates Service Menu is active
bool inmenu = 0;                // Indicates user is inside a submenu

// Pointer control for menu navigation
bool uppointer = 0;             // Pointer on upper option
bool downpointer = 0;           // Pointer on lower option

// ---------------- FEATURE FLAGS ----------------
bool secondaryyes = 0;          // Secondary fill option enable/disable
bool solenoidoverride = 1;      // Solenoid override control
bool flowoverride = 0;          // Flow sensor override
bool leveloverride = 0;         // Level sensor override
bool probeoverride = 0;         // Temperature probe override

bool speedup = 0;               // Reserved for speed control feature
bool secondarytimerflag = 0;    // Indicates secondary fill timer is running
bool time_skip = 0;             // Used to skip time during secondary process

// ---------------- FACTORY RESET ----------------
bool factoryresetflag = 1;      // Confirmation flag for factory reset

// ---------------- COUNTERS ----------------
uint8_t skip_count = 0;         // Counter used for time skip logic
uint8_t longpress_count = 0;    // Counter for smooth long press increment/decrement

// ---------------- SAFETY PARAMETERS ----------------
int Heatersafteytemp = 90;     // Current heater safety temperature setting

// ---------------- BUTTON PIN DEFINITIONS ----------------
#define UP 35
#define DOWN 32
#define BACK 26
#define MODE 36

// ---------------- BUTTON OBJECTS ----------------
// Using OneButton library for handling click & long press events
OneButton up_button(UP, true);
OneButton down_button(DOWN, true);
OneButton back_button(BACK, true);
OneButton mode_button(MODE, true);

// ---------------- CLASS CONSTRUCTOR ----------------
buttonClass::buttonClass()
{
    // Constructor currently does nothing
}

// ---------------- BUTTON INITIALIZATION ----------------
void buttonClass::button_setup()
{
    // Configure button pins with internal pull-up resistors
    pinMode(UP, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);
    pinMode(BACK, INPUT_PULLUP);
    pinMode(MODE, INPUT_PULLUP);
    
    // -------- BUTTON EVENT ATTACHMENTS --------

    // UP button
    up_button.attachClick(increment);                 // Short press → Increment value / move up
    up_button.attachDuringLongPress(long_press_up);   // Long press → Continuous increment

    // DOWN button
    down_button.attachClick(decrement);               // Short press → Decrement value / move down
    down_button.attachDuringLongPress(long_press_down);// Long press → Continuous decrement

    // MODE button
    mode_button.attachDuringLongPress(user_settings); // Long press → Enter User Settings
    mode_button.attachClick(enter_function);          // Short press → Enter / Confirm action

    // BACK button
    back_button.attachClick(back_screen);             // Short press → Go back one screen
    back_button.setPressMs(1000);                     // Long press threshold (1 second)
    back_button.attachDuringLongPress(back_to_home);  // Long press → Return to main screen

    // Note:
    // BACK long press acts as a "hard reset" to main UI state
}

// ---------------- BUTTON EVENT PROCESSING ----------------
// This function must be called repeatedly inside loop()
// It updates the state of all buttons (click / long press detection)
void buttonClass::button_ticks()
{
    up_button.tick();     // Process UP button events
    down_button.tick();   // Process DOWN button events
    mode_button.tick();   // Process MODE button events
    back_button.tick();   // Process BACK button events
}

// ---------------- POINTER DISPLAY FUNCTION ----------------
// Displays '>' symbol at given LCD position
void buttonClass::setPointer(uint8_t col, uint8_t row)
{
    lcd.clear();              // Clear entire LCD 
    lcd.setCursor(col, row);  // Move cursor to specified position
    lcd.print(">");           // Print selection pointer
}

// ---------------- DUAL BUTTON LONG PRESS (UP + DOWN) ----------------
// Detects simultaneous press of UP and DOWN buttons
// If held for defined duration → enters Service Menu
void buttonClass::but_check()
{
    // -------- STEP 1: Detect initial simultaneous press --------
    if (digitalRead(UP) == LOW && digitalRead(DOWN) == LOW && dualPressActive == 0)
    {
        start_tt = millis();  // Record start time of press
        dualPressActive = 1;             // Mark that dual press has started
    }

    // -------- STEP 2: Check if buttons are still held --------
    else if (digitalRead(UP) == LOW && digitalRead(DOWN) == LOW && dualPressActive == 1)
    {
        // Check if hold duration exceeded threshold
        if ((millis() - start_tt) >= 1000)   // 1000 ms = 1 second
        {
            // -------- ENTER SERVICE MENU --------
            lcd.clear();

            screen = ServiceMenuScreen1; // Switch to first service menu screen
            servicemenu = 1;             // Activate service menu mode
            mainscreenflag = 0;          // Exit main screen

            // Set pointer to first option
            uppointer = 1;
            downpointer = 0;
            setPointer(0, 0);

            // Provide user feedback via buzzer
            digitalWrite(BUZZER, HIGH);
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();

            return; // Exit after triggering action
        }
    }

    // -------- STEP 3: Reset if buttons released --------
    else if (dualPressActive == 1)
    {
        // If either button is released → cancel detection
        if (digitalRead(UP) == HIGH || digitalRead(DOWN) == HIGH)
        {
            dualPressActive = 0; // Reset dual press flag
        }
    }
}

// ---------------- BACK TO HOME (LONG PRESS BACK BUTTON) ----------------
// This function performs a "hard reset" of the UI and process state.
// It stops all operations, resets flags, and returns to Main Screen.
// Also handles error condition routing based on override settings.

void buttonClass::back_to_home()
{
    // Execute only if NOT already on main screen AND tap is not closed
    if (!mainscreenflag && !closetap)
    {
        lcd.clear();  // Clear display

        // -------- USER FEEDBACK (BUZZER) --------
        digitalWrite(BUZZER, HIGH);
        buzzerclass_object.Buzzer_beep(1000);
        buzzerclass_object.Buzzer_start();

        // -------- RESET SYSTEM STATES --------
        mainscreenflag = 1;      // Return to main screen
        process_flag = 0;        // Stop process execution
        temp_drop_flag = 0;      // Reset temperature drop logic
        inmenu = 0;              // Exit any submenu
        secondarytimerflag = 0;  // Stop secondary timer

        // -------- STOP HARDWARE OPERATIONS --------
        process_object.heater1_stop();
        // process_object.heater1_stop();     // Stop heater
        // buzzerclass_object.heater_stop();  // Stop buzzer-related heater logic

        digitalWrite(SOLENOID1, LOW);  // Close solenoid 1
        digitalWrite(SOLENOID2, LOW);  // Close solenoid 2

        // -------- RESTORE SETTINGS FROM EEPROM --------
        eeprom_object.eeprom_dataread();

        // Default screen → Main Screen
        screen = MainScreen;

        waterlevel_error_flag=0;
        flow_error_checkflag=0;
        primary_filling_flag=0;
        error_check_flag=0;
        pauseflag=0;
        //probeerrorflag=0;

        // -------- ERROR HANDLING LOGIC --------
        // If solenoid override is OFF and not in settings/menu,
        // force system into error screen for safety validation
        if (!solenoidoverride && !usersettings && !servicemenu)
        {
            Probe1_Err = 0;
            waterlevel_error_flag = 0;
            flow_error_checkflag = 0;

            closetap = 1;          // Mark tap as closed
            error_check_flag = 1;  // Trigger error check

            screen = ErrorScreen;  // Redirect to error screen
        }
        else
        {
            // Normal return to main screen
            usersettings = 0;
            servicemenu = 0;
            screen = MainScreen;
        }
    }
}

// ---------------- LONG PRESS UP BUTTON ----------------
// Handles continuous increment when UP button is held
// Uses longpress_count to control increment speed (non-blocking)

void buttonClass::long_press_up()
{
    // Prevent conflict: only act if DOWN button is NOT pressed
    if (digitalRead(DOWN) == HIGH)
    {
        // -------- MAIN SCREEN: INCREASE VOLUME --------
        if (mainscreenflag)
        {
            // Calculate maximum allowed volume
            Max_liter = (2 * OPERATING_TIME) * (variant / 10.0);

            if (counter < Max_liter)
            {
                longpress_count++;  // Increment counter for smooth stepping

                // Increase value every 4 cycles
                if (longpress_count == 4)
                {
                    counter += 0.5;
                    longpress_count = 0;  // Reset counter
                }
            }
        }

        // -------- CALIBRATION SETTINGS --------
        if (screen == CalibrationSettings)
        {
            // Max_liter = (2 * OPERATING_TIME) * (variant / 10.0);

            if (calibration_value < Max_liter +5)
            {
                longpress_count++;

                // Faster increment (every 3 cycles)
                if (longpress_count == 3)
                {
                    calibration_value += 0.1;
                    longpress_count = 0;
                }
            }
        }

        // -------- SAFETY TEMPERATURE SETTINGS --------
        if (screen == SafteyTemperatureSettings)
        {
            if (Heatersafteytemp < MAX_SAFETY_TEMP)
            {
                longpress_count++;

                // Slower increment (every 5 cycles for safety)
                if (longpress_count == 5)
                {
                    Heatersafteytemp += SAFETY_TEMP_STEP;
                    longpress_count = 0;
                }
            }
        }

        // -------- PROBE CALIBRATION SETTINGS --------
        if (screen == ProbeCalibrationSettings)
        {
            if (temp_error <= PROBE_CALIBRATION_LIMIT)
            {
                longpress_count++;

                // Controlled increment (every 5 cycles)
                if (longpress_count == 5)
                {
                    temp_error += 0.1;
                    longpress_count = 0;
                }
            }
        }
    }
}

// ---------------- LONG PRESS DOWN BUTTON ----------------
// Handles continuous decrement when DOWN button is held
// Mirror logic of long_press_up()

void buttonClass::long_press_down()
{
    // Prevent conflict: only act if UP button is NOT pressed
    if (digitalRead(UP) == HIGH)
    {
        // -------- MAIN SCREEN: DECREASE VOLUME --------
        if (counter >= 0.5)
        {
            if (mainscreenflag)
            {
                longpress_count++;

                // Decrease every 4 cycles
                if (longpress_count == 4)
                {
                    counter -= 0.5;
                    longpress_count = 0;
                }
            }
        }

        // -------- CALIBRATION SETTINGS --------
        if (screen == CalibrationSettings)
        {
            if (calibration_value > 0.0)
            {
                longpress_count++;

                if (longpress_count == 3)
                {
                    calibration_value -= 0.1;
                    longpress_count = 0;
                }
            }
            if(calibration_value < 0.0)
                calibration_value = 0.0;
        }

        // -------- SAFETY TEMPERATURE SETTINGS --------
        if (screen == SafteyTemperatureSettings)
        {
            if (Heatersafteytemp > MIN_SAFETY_TEMP)
            {
                longpress_count++;

                if (longpress_count == 5)
                {
                    Heatersafteytemp -= SAFETY_TEMP_STEP;
                    longpress_count = 0;
                }
            }
        }

        // -------- PROBE CALIBRATION SETTINGS --------
        if (screen == ProbeCalibrationSettings)
        {
            if (temp_error >= -(PROBE_CALIBRATION_LIMIT))
            {
                longpress_count++;

                if (longpress_count == 5)
                {
                    temp_error -= 0.1;
                    longpress_count = 0;
                }
            }
        }
    }
}

void buttonClass::increment()
{
    // -------- MAIN SCREEN: INCREASE OUTPUT VOLUME --------

    if(mainscreenflag)
    {
        Max_liter=(2*OPERATING_TIME)*(variant/10.0);    // Calculate max allowed liters
        if(counter<Max_liter)
        {
        counter+=0.5;   // Increase volume in steps of 0.5
        }
        return;
    }

    // -------- HANDLE SCREEN NAVIGATION & SETTINGS --------

    if(usersettings)
    {
    switch(screen)
    {
//  **************** User Settings *******************


        case UserSettingsScreen1:

                if(uppointer)
                {
                    screen=UserSettingsScreen4;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;

                }
                else
                {
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
            
            break;

            case UserSettingsScreen2:
            if(uppointer)
            {
                if(dduflag)
                {
                    screen=UserSettingsScreen1;
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
                else
                {
                    screen=UserSettingsScreen3;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
            }
            else 
            {
            buttonClass_object.setPointer(0,0);
              uppointer=1;
              downpointer=0;
            }
            break;

            case UserSettingsScreen3:
          if(uppointer)
            {
                screen=UserSettingsScreen2;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break; 

        case UserSettingsScreen4:
          if(uppointer)
            {
                screen=UserSettingsScreen3;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break;

         case SecondaryFillSettings:
            if(!secondaryyes)
            {
                buttonClass_object.setPointer(0,1);
                secondaryyes=1;
            }
            else
            {
                buttonClass_object.setPointer(11,1);
                secondaryyes=0;
            }
        break;

         case FlowControlSettings:
          if(!flowoverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                flowoverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                flowoverride=0;
            }
        break;

        case LevelSensorSettings:
            if(!leveloverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                leveloverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                leveloverride=0;
            }
        break;

        case SolenoidControlSettings:
            if(!solenoidoverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                solenoidoverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                solenoidoverride=0;
            }
        break;

         case TempSensorSettings:
            if(!probeoverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                probeoverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                probeoverride=0;
            }
        break;

        
        }
    return;
    }

//  **************** Service menu Settings *******************        

        if(servicemenu)
        {
            switch(screen)
            {


        case ServiceMenuScreen1:
            if(dduflag)
            {
                if(uppointer)
                {
                    screen=ServiceMenuScreen5;
                    buttonClass_object.setPointer(0,1);
                     uppointer=0;
                    downpointer=1;
                }
                else
                {
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
            }
            else
            {
                if(uppointer)
                {
                    screen=SDUServiceMenuScreen3;
                    buttonClass_object.setPointer(0,1);
                     uppointer=0;
                    downpointer=1;
                }
                else
                {
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }

            }
        break;

        case ServiceMenuScreen2:
          if(uppointer)
            {
                screen=ServiceMenuScreen1;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break;

        case ServiceMenuScreen3:
          if(uppointer)
            {
                screen=ServiceMenuScreen2;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break;

        case ServiceMenuScreen4:
          if(uppointer)
            {
                screen=ServiceMenuScreen3;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break;

        case ServiceMenuScreen5:
          if(uppointer)
            {
                screen=ServiceMenuScreen4;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break;

        case SDUServiceMenuScreen3:
          if(uppointer)
            {
                screen=ServiceMenuScreen2;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break;

        case ProductTypeSettings:
            if(!dduflag)
            {
                lcd.clear();
                lcd.setCursor(11,1);
                lcd.print(">DDU");
                dduflag=1;
            }
            else
            {
                lcd.clear();
                lcd.setCursor(0,1);
                lcd.print(">SDU     ");
                dduflag=0;
            }
        break;

        case SubProductTypeSettings:
            if(prodtypecounter<2){
            prodtypecounter++;
            }
            else
            {
                prodtypecounter=0;
            }
        break;

        case CalibrationSettings:
            // Max_liter=(2*OPERATING_TIME)*(variant/10.0);
            if(calibration_value<Max_liter+5.0)
            {
                calibration_value+=0.1;
            }
        break;

        case SafteyTemperatureSettings:
            if(Heatersafteytemp<MAX_SAFETY_TEMP)
            {
            Heatersafteytemp+=SAFETY_TEMP_STEP;
            }
        break;

        case ProbeCalibrationSettings:
            if(temp_error<=PROBE_CALIBRATION_LIMIT)
            {
            temp_error+=0.1;
            }

        break;

        case FactoryResetScreen:
            if(!factoryresetflag)
            {
                buttonClass_object.setPointer(0,1);
                factoryresetflag=1;
            }
            else
            {
                buttonClass_object.setPointer(11,1);
                factoryresetflag=0;
            }
        break;  
         }
        return;
        } 
}



void buttonClass::decrement()
{

    if(mainscreenflag)
        {
            if(counter>0.0)
            {
                counter-=0.5;
            }
            return;
        }
//  **************** User Settings *******************
    if(usersettings)
    {
        switch(screen)
        {
            case UserSettingsScreen1:
                if(downpointer)
                {
                    screen=UserSettingsScreen2;
                    buttonClass_object.setPointer(0,1);
                    downpointer=1;
                    uppointer=0;   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);
                    downpointer=1;
                    uppointer=0;
                }
        
            break;

            case UserSettingsScreen2:
                
                if(downpointer)
                {
                    screen=UserSettingsScreen3;
                    buttonClass_object.setPointer(0,1);
                    downpointer=1;
                    uppointer=0;   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);
                    downpointer=1;
                    uppointer=0;
                }
            break;

    case UserSettingsScreen3:
        if(downpointer)
        {
            if(dduflag)
            {
                screen=UserSettingsScreen4;
                buttonClass_object.setPointer(0,1);
                downpointer=1;
                uppointer=0;
            }
            else
            {
                screen=UserSettingsScreen2;
                buttonClass_object.setPointer(0,0);
                downpointer=0;
                uppointer=1;
            }   
        }
        else
        {
        buttonClass_object.setPointer(0,1);
        downpointer=1;
        uppointer=0;
        }
        
    break;

case UserSettingsScreen4:
        if(downpointer)
        {
            if(dduflag)
            {
                screen=UserSettingsScreen1;
                buttonClass_object.setPointer(0,0);
                downpointer=0;
                uppointer=1;
            }
            else
            {
                screen=UserSettingsScreen2;
                buttonClass_object.setPointer(0,0);
                downpointer=0;
                uppointer=1;
            } 
       
        }
        else
        {
        buttonClass_object.setPointer(0,1);
        downpointer=1;
        uppointer=0;
        }
        
    break;

    case SecondaryFillSettings:
       if(!secondaryyes)
            {
                buttonClass_object.setPointer(0,1);
                secondaryyes=1;
            }
            else
            {
                // lcd.clear();
                // lcd.setCursor(11,1);
                // lcd.print(">");
                buttonClass_object.setPointer(11,1);
                secondaryyes=0;
            }
    break;

    case FlowControlSettings:
        if(!flowoverride)
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                flowoverride=1;
            }
            else
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                flowoverride=0;
            }
    break;

    case LevelSensorSettings:
            if(!leveloverride)
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                leveloverride=1;
            }
            else
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                leveloverride=0;
            }
        break;

         case SolenoidControlSettings:
            if(!solenoidoverride)
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                solenoidoverride=1;
            }
            else
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                solenoidoverride=0;
            }
        break;

    case TempSensorSettings:
            if(!probeoverride)
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                probeoverride=1;
            }
            else
            {
                // lcd.clear();
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                probeoverride=0;
            }
        break;   
        }
        return;
    }

        if(servicemenu)
        {
            switch(screen)
        {

        case ServiceMenuScreen1:
        if(downpointer)
        {
            screen=ServiceMenuScreen2;
            buttonClass_object.setPointer(0,1);
            downpointer=1;
            uppointer=0;   
        }
        else
        {
        buttonClass_object.setPointer(0,1);
        downpointer=1;
        uppointer=0;
        }
        
        break;

    case ServiceMenuScreen2:
        if(dduflag)
        {
            if(downpointer)
            {
                screen=ServiceMenuScreen3;
                // lcd.clear();
                // lcd.setCursor(0,1);
                // lcd.print(">");
                buttonClass_object.setPointer(0,1);
                downpointer=1;
                uppointer=0;   
            }
            else
            {
                // lcd.clear();
                // lcd.setCursor(0,1);
                // lcd.print(">");
                buttonClass_object.setPointer(0,1);
                downpointer=1;
                uppointer=0;
            }
        }
        else
        {
            if(downpointer)
            {
                screen=SDUServiceMenuScreen3;
                // lcd.clear();
                // lcd.setCursor(0,1);
                // lcd.print(">");
                buttonClass_object.setPointer(0,1);
                downpointer=1;
                uppointer=0;   
            }
            else
            {
                buttonClass_object.setPointer(0,1);
                downpointer=1;
                uppointer=0;
            }
        }
    break;
    case ServiceMenuScreen3:
        if(downpointer)
        {
            screen=ServiceMenuScreen4;
            buttonClass_object.setPointer(0,1);
            downpointer=1;
            uppointer=0;   
        }
        else
        {
        buttonClass_object.setPointer(0,1);
        downpointer=1;
        uppointer=0;
        }
        
    break;

    case ServiceMenuScreen4:
        if(downpointer)
        {
            screen=ServiceMenuScreen5;
            buttonClass_object.setPointer(0,1);
            downpointer=1;
            uppointer=0;   
        }
        else
        {
        buttonClass_object.setPointer(0,1);
        downpointer=1;
        uppointer=0;
        }
        
    break;

    case ServiceMenuScreen5:
        if(downpointer)
        {
            screen=ServiceMenuScreen1;
            buttonClass_object.setPointer(0,0);
            downpointer=0;
            uppointer=1;   
        }
        else
        {
        buttonClass_object.setPointer(0,1);
        downpointer=1;
        uppointer=0;
        }
        
    break;

    case SDUServiceMenuScreen3:
        if(downpointer)
        {
            screen=ServiceMenuScreen1;
            buttonClass_object.setPointer(0,0);
            downpointer=0;
            uppointer=1;    
        }
        else
        {
        buttonClass_object.setPointer(0,1);
        downpointer=1;
        uppointer=0;
        }
        
    break;

    case ProductTypeSettings:
            if(!dduflag)
            {
                lcd.clear();
                lcd.setCursor(11,1);
                lcd.print(">DDU");
                dduflag=1;
            }
            else
            {
                lcd.clear();
                lcd.setCursor(0,1);
                lcd.print(">SDU     ");
                dduflag=0;
            }
    break;

    case SubProductTypeSettings:
            if(prodtypecounter>0){
            prodtypecounter--;
            }
            else{
                prodtypecounter=2;
            }
    break;

     case CalibrationSettings:
        if(calibration_value > 0.0)
        {
        calibration_value-=0.1;
        }
        if(calibration_value < 0.0)
            calibration_value = 0.0;
    break;

    case SafteyTemperatureSettings:
        if(Heatersafteytemp > MIN_SAFETY_TEMP)
        {
            Heatersafteytemp-=SAFETY_TEMP_STEP;
        }    
    break;



    case ProbeCalibrationSettings:
        if(temp_error>=-(PROBE_CALIBRATION_LIMIT))
        {
            temp_error-=0.1;
        }
    break;

    

    

    case FactoryResetScreen:
       if(!factoryresetflag)
            {
                buttonClass_object.setPointer(0,1);
                factoryresetflag=1;
            }
            else
            {
                buttonClass_object.setPointer(11,1);
                factoryresetflag=0;
            }
    break;

        }
    
  
        return;
    }
}

void buttonClass:: user_settings()
{
   
    if(!usersettings && !process_flag  && !servicemenu && !secondarytimerflag)
    {
      lcd.clear();
      if(dduflag)
      {
        screen=UserSettingsScreen1;
        // Serial3.println("ddu");
      }
      else
      {
        screen=UserSettingsScreen2;
        // Serial3.println("sdu");
      }
      usersettings=1;
      mainscreenflag=0;
      digitalWrite(YELLOW_LED,LOW);
      digitalWrite(GREEN_LED,HIGH);
      digitalWrite(BUZZER,HIGH);
      buzzerclass_object.Buzzer_beep(1000);
      buzzerclass_object.Buzzer_start();
        buttonClass_object.setPointer(0,0);
      uppointer=1;
      return;
    }

    if(secondarytimerflag && !time_skip)
    {
        skip_count++;
        if (skip_count>=50)
        {
            one_second_counter=pre_end_time-5;
            skip_count=0;
            time_skip=1;
            digitalWrite(BUZZER,HIGH);
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();
        }
        return;
    }
    else
    {
        skip_count=0;
        return;
    }

   
}

void buttonClass:: back_screen()
{
    if((usersettings || servicemenu) && !inmenu){
        lcd.clear();
        // eeprom_object.eeprom_datawrite();
        screen=MainScreen;
        mainscreenflag=1;
        usersettings=0;
        servicemenu=0;
        uppointer=0;
        downpointer=0;
        return;
    }

    if(inmenu)
    {
        // lcd.clear();
        // usersettings=1;
        // servicemenu=1;
        uppointer=1;
        downpointer=0;
        // lcd.setCursor(0,0);
        // lcd.print(">");
        buttonClass_object.setPointer(0,0);
        inmenu=0;

        switch(screen)
        {
            
            case FlowControlSettings:
                EEPROM.get(FLOW_CONTROL, flowoverride);
                screen=UserSettingsScreen2;
                usersettings=1;
            break;

            case LevelSensorSettings:
                EEPROM.get(LEVEL_CONTROL, leveloverride); 
                screen=UserSettingsScreen3;
                usersettings=1;
                
            break;

            case TempSensorSettings:
                EEPROM.get(PROBE_CONTROL, probeoverride);

                // lcd.clear();
                uppointer=0;
                downpointer=1;
                // lcd.setCursor(0,1);
                // lcd.print(">");
                buttonClass_object.setPointer(0,1);
                screen=UserSettingsScreen4;
                usersettings=1;

            break;

            case SolenoidControlSettings:
                EEPROM.get(SOLENOID_CONTROL, solenoidoverride);
                if(dduflag)
                {
                    screen=UserSettingsScreen4;
                    usersettings=1;
                }
                else
                {
                    // lcd.clear();
                    uppointer=0;
                    downpointer=1;
                    // lcd.setCursor(0,1);
                    // lcd.print(">");
                    buttonClass_object.setPointer(0,1);
                    screen=UserSettingsScreen3;
                    usersettings=1;
                }
            break;

            case ProductTypeSettings:
                dduflag=EEPROM.read(PRODUCT_SELECTION);
                screen=ServiceMenuScreen1;
                servicemenu=1;
            break;

            case CalibrationSettings:
                EEPROM.get(CALIBRATION_VALUE, calibration_value);
                servicemenu=1;
                if(dduflag)
                {
                    screen=ServiceMenuScreen3;
                }
                else
                {
                    screen=SDUServiceMenuScreen3;
                }
                
            break;

            case SecondaryFillSettings:
                EEPROM.get(SECONDARY_FILL, secondaryyes);
                screen=UserSettingsScreen1;
                usersettings=1;
            break;

            case SafteyTemperatureSettings:
                EEPROM.get(SAFETY_TEMP, Heatersafteytemp);
                screen=ServiceMenuScreen4;
                servicemenu=1;
            break;

            case ProbeCalibrationSettings:
                EEPROM.get(PROBE_ERROR, temp_error);
                screen=ServiceMenuScreen5;
                servicemenu=1;
            break;

            case SubProductTypeSettings:
                EEPROM.get(SUBPRODUCT_SELECTION,prodtypecounter);
                screen=ServiceMenuScreen2;
                servicemenu=1;
                Max_liter = (2 * OPERATING_TIME) * (variant / 10.0);
            break;

            case FactoryResetScreen:

                uppointer=0;
                downpointer=1;
                servicemenu=1;
                buttonClass_object.setPointer(0,1);
               if(dduflag)
               {
                screen=ServiceMenuScreen5;
               }
               else
               {
                screen=SDUServiceMenuScreen3;
               }
            break;
        }
        return;
    }
}

void buttonClass::enter_function()
{
    if(error_check_flag && closetap)
    {
        lcd.clear();
        error_check_flag=0;
        closetap=0;
        return;
    }

    if(error_check_flag && zero_calib)
    {
        lcd.clear();
       
        error_check_flag=0;
        zero_calib=0;
        screen=CalibrationSettings;
        servicemenu = 1;
        inmenu=1;
        digitalWrite(BUZZER,LOW);
        return;
    }

    if(mainscreenflag && counter>0.0)
    {
        if(calibration_value == 0.0)
        {
            zero_calib=1;
            mainscreenflag=0;
            error_check_flag=1;
            screen=ErrorScreen;
            return;
        }

        process_object.variant_settings();
        
        // Serial3.println("BUZZZZZZZZZZZER");
        digitalWrite(BUZZER,HIGH);
        
        buzzerclass_object.Buzzer_beep(1000);
        buzzerclass_object.Buzzer_start();

        if(secondaryyes && dduflag)
        {
            
           
            screen=SecondaryFillTimer;
            mainscreenflag=0;
            one_second_counter=0;
            secondarytimerflag=1;
            pauseflag=0;
            error_check_flag=0;
            process_object.Contactor1_start();
        }
        else
        {
            screen=ProcessScreen;
            process_flag=1;
            mainscreenflag=0;
            one_second_counter=0;
            pauseflag=0;
            error_check_flag=0;
            process_object.error_check();
            if(error_check_flag)
            {
                return;
            }
            process_object.Contactor1_start();

            // process_object.heater1_start();
            // if(dduflag){
            // buzzerclass_object.heater_start();
            // }
        }
        return;
    }

        if(inmenu)
        {
        
        // usersettings=1;
        // servicemenu=1;
        uppointer=1;
        downpointer=0;
        buttonClass_object.setPointer(0,0);
        inmenu=0;

        switch(screen)
        {
            case FlowControlSettings:
                EEPROM.put(FLOW_CONTROL, flowoverride);
                screen=UserSettingsScreen2;
                usersettings=1;
            break;

            case LevelSensorSettings:
                EEPROM.put(LEVEL_CONTROL, leveloverride);
                screen=UserSettingsScreen3;
                usersettings=1; 
            break;

            case TempSensorSettings:
                EEPROM.put(PROBE_CONTROL, probeoverride);
                screen=UserSettingsScreen4;
                usersettings=1;
                buttonClass_object.setPointer(0,1);
                uppointer=0;
                downpointer=1;
                
            break;

            case SolenoidControlSettings:
                EEPROM.put(SOLENOID_CONTROL, solenoidoverride);
                usersettings=1;
                if(dduflag)
                {
                    screen=UserSettingsScreen4;
                }
                else
                {
                    screen=UserSettingsScreen3;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
                
            break;

            case ProductTypeSettings:
                screen=ServiceMenuScreen1;
                servicemenu=1;
                calibration_value=0.0;
                EEPROM.write(PRODUCT_SELECTION, dduflag);
                delay(50);
                EEPROM.put(CALIBRATION_VALUE, calibration_value);
                delay(50);
            break;

            case CalibrationSettings:
                EEPROM.put(CALIBRATION_VALUE, calibration_value);
                servicemenu=1;
                if(dduflag)
                {
                    screen=ServiceMenuScreen3;
                }
                else
                {
                    screen=SDUServiceMenuScreen3;
                   
                }
                
            break;

            case SecondaryFillSettings:
                 EEPROM.put(SECONDARY_FILL, secondaryyes);
                screen=UserSettingsScreen1;
                usersettings=1;

            break;

            case SafteyTemperatureSettings:
                EEPROM.put(SAFETY_TEMP, Heatersafteytemp);
                screen=ServiceMenuScreen4;
                servicemenu=1;
            break;

            case ProbeCalibrationSettings:
                EEPROM.put(PROBE_ERROR, temp_error);
                screen=ServiceMenuScreen5;
                servicemenu=1;
            break;

            case SubProductTypeSettings:
                EEPROM.put(SUBPRODUCT_SELECTION,prodtypecounter);
                screen=ServiceMenuScreen2;
                servicemenu=1;
                calibration_value=0.0;
                delay(50);
                EEPROM.put(CALIBRATION_VALUE, calibration_value);
                delay(50);
                Max_liter = (2 * OPERATING_TIME) * (variant / 10.0);
            break;

            case FactoryResetScreen:
                if(factoryresetflag)
                {
                    lcd.setCursor(0,0);
                    lcd.print(" FACTORY RESET");
                    lcd.setCursor(5,1);
                    lcd.print(" DONE"); 
                    digitalWrite(BUZZER,HIGH);
                    eeprom_object.eeprom_defaultvalue();

                    delay(2000);
                    digitalWrite(BUZZER,LOW);
                    uppointer=1;
                    downpointer=0;
                    mainscreenflag=1;
                    usersettings=0;
                    servicemenu=0;
                    lcd.clear();
                    screen=MainScreen;

                }
                else
                {
                    uppointer=0;
                    downpointer=1;
                    servicemenu=1;
                    if(dduflag)
                    {
                        buttonClass_object.setPointer(0,1);
                        screen=ServiceMenuScreen5;

                    }
                    else
                    {
                        buttonClass_object.setPointer(0,1);
                        screen=SDUServiceMenuScreen3;

                    }
                   
                }
            break;
        }
        return;
    }


    if(servicemenu && !inmenu)
    {
        switch(screen)
        {
            case ServiceMenuScreen1:
                if(uppointer)
                {
                    // lcd.clear();
                    screen=ProductTypeSettings;
                    inmenu=1;
                    if(dduflag)
                    {
                        // lcd.setCursor(11,1);
                        // lcd.print(">");
                        buttonClass_object.setPointer(11,1);
                    }
                    else
                    {
                        // lcd.setCursor(0,1);
                        // lcd.print(">");  
                        buttonClass_object.setPointer(0,1);
                    }
                }
                else
                {
                    lcd.clear();
                    screen=SubProductTypeSettings;
                    inmenu=1;
                }

            break;

            case ServiceMenuScreen2:
                if(downpointer)
                {
                    screen=CalibrationSettings;
                    lcd.clear();
                    inmenu=1;
                }
                else
                {
                    lcd.clear();
                    screen=SubProductTypeSettings;
                    inmenu=1;
                }
            break;

            case ServiceMenuScreen3:
                if(downpointer)
                {
                    lcd.clear();
                    screen=SafteyTemperatureSettings;
                    inmenu=1;
                }
                else
                {
                    screen=CalibrationSettings;
                    lcd.clear();
                    inmenu=1;
                }

            break;

            case ServiceMenuScreen4:
                if(downpointer)
                {
                    lcd.clear();
                    screen=ProbeCalibrationSettings;
                    inmenu=1;
                }
                else
                {
                    lcd.clear();
                    screen=SafteyTemperatureSettings;
                    inmenu=1;
                }

            break;

            case ServiceMenuScreen5:
                if(downpointer)
                {
                    // lcd.clear();
                    factoryresetflag=1;
                    screen=FactoryResetScreen;
                    inmenu=1;
                    // lcd.setCursor(0,1);
                    // lcd.print(">"); 
                    buttonClass_object.setPointer(0,1);
                }
                else
                {
                    lcd.clear();
                    screen=ProbeCalibrationSettings;
                    inmenu=1;
                }

            break;

            case SDUServiceMenuScreen3:
                if(downpointer)
                {
                    // lcd.clear();
                    screen=FactoryResetScreen;
                    factoryresetflag=1;
                    inmenu=1;
                    // lcd.setCursor(0,1);
                    // lcd.print(">"); ;
                    buttonClass_object.setPointer(0,1);
                }
                else
                {
                    screen=CalibrationSettings;
                    lcd.clear();
                    inmenu=1;
                }

            break;
        }
        return;
    } 

    if(usersettings && !inmenu)
    {
        switch(screen)
        {
            case UserSettingsScreen1:
                if(uppointer)
                {
                    // lcd.clear();
                    screen=SecondaryFillSettings;
                    inmenu=1;
                    if(secondaryyes)
                    {
                        buttonClass_object.setPointer(0,1);
                    }
                    else
                    { 
                        buttonClass_object.setPointer(11,1);
                    }
                }
                else
                {
                    lcd.clear();
                    screen=FlowControlSettings;
                    if(flowoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;
                }
            break;

            case UserSettingsScreen2:
                if(downpointer)
                {
                    screen=LevelSensorSettings;
                    lcd.clear();
                     if(leveloverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;
                }
                else
                {
                    lcd.clear();
                    screen=FlowControlSettings;
                    if(flowoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
            break;

             case UserSettingsScreen3:
                if(downpointer)
                {
                    lcd.clear();
                    screen=SolenoidControlSettings;
                    if(solenoidoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
                else
                {
                   screen=LevelSensorSettings;
                    lcd.clear();
                     if(leveloverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
            break;

            case UserSettingsScreen4:
                if(downpointer)
                {
                    screen=TempSensorSettings;
                    lcd.clear();
                     if(probeoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;
                }
                else
                {
                    lcd.clear();
                    screen=SolenoidControlSettings;
                    if(solenoidoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
            break;

            }
            return;
        }
}


buttonClass buttonClass_object = buttonClass();