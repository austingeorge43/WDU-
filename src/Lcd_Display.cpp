#include <Arduino.h>
#include "Ext_Var.h"

// LED and Buzzer Pins
#define RED_LED 17 //HELLO
#define YELLOW_LED 39
#define GREEN_LED 38
#define BUZZER 18



void toggle_screen();
LiquidCrystal_I2C lcd(0X27,16,2);
int screen=0;
int optimecounter=0;
int prodtypecounter=0;
int Sec_time=0;
int remaining_time=0;   

bool toggle=0;
bool dduflag=0;

unsigned long solenoid_stop=0;

int optime[4]={6,8,10,12};
int prodtype[3]={150,250,400};
float base_calibration[3]={1.4,2.4,3.9};

// Error blinking ticker for main screen
Ticker error_blink(toggle_screen,500,0,MILLIS);
 
//DEFINATIONS
lcdclass::lcdclass()
{}

void toggle_screen()
{
    toggle=!toggle;
}
// -------- LCD INTIALIZATION --------
void lcdclass::lcd_setup()
{
    Wire.begin();
    // Wire.setClock(100000);
    lcd.backlight();
    // lcd.clear();
    // delay(5000);
    
    // lcd.clear();
    lcd.init();
    // lcd.clear();
    // lcd.backlight();
    // lcd.clear();
    
    delay(100);
    // lcd.noBacklight();
    lcd.print(" SELECT VOLUME   ");
    lcd.noAutoscroll();


    error_blink.start();

    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);

    variant = ((prodtype[prodtypecounter]) / 10);
    screen = VersionScreen;
}

void lcdclass:: lcd_blink_update()
{
    error_blink.update();

}

void lcdclass::lcd_display()
{

    switch(screen)
    {
        case VersionScreen:
            lcd.setCursor(0,0);
            lcd.print("LABQUEST BOROSIL");
            lcd.setCursor(1,1);
            if(dduflag)
            {
                lcd.print("DDU ");// V3.00");
                lcd.print(prodtype[prodtypecounter]);
                lcd.print(" V3.00");
            }
            else
            {
                lcd.print("SDU ");// V3.00");
                lcd.print(prodtype[prodtypecounter]);
                lcd.print(" V3.00");

            }
            // lcd.print("WDU V3.00");
            digitalWrite(BUZZER,HIGH);
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();
            delay(2000);
            screen++;
            lcd.clear();
            mainscreenflag=1;
        
        break;

        case MainScreen:
            downpointer=0;
            digitalWrite(YELLOW_LED,HIGH);
            digitalWrite(GREEN_LED,LOW);
            digitalWrite(RED_LED,LOW);
            lcd.setCursor(0,0);
            lcd.print(" SELECT VOLUME   ");     
            lcd.setCursor(3,1);
            lcd.print(counter,1);
            if(counter<10.0){
                lcd.setCursor(6,1);
                lcd.print(" ");
                lcd.print("LITERS ");
            }
            else
            {
                lcd.setCursor(7,1);
                lcd.print("");
                lcd.print(" LITERS");
            }
            buttonClass_object.but_check();
        break;

        case UserSettingsScreen1:
            mainscreenflag=0;
            digitalWrite(GREEN_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);
            lcd.setCursor(1,0);
            lcd.print("SECONDARY FILL   ");    
            lcd.setCursor(1,1);
            lcd.print("FLOW CONTROL    "); 
        break;

        case UserSettingsScreen2:
            lcd.setCursor(1,0);
            lcd.print("FLOW CONTROL   ");     
            lcd.setCursor(1,1);
            lcd.print("LEVEL CONTROL  ");
        break;
        
        case UserSettingsScreen3:
            lcd.setCursor(1,0);
            lcd.print("LEVEL CONTROL  ");
            lcd.setCursor(1,1);
            lcd.print("SOLENOID CONTRL   ");
        break;

        case UserSettingsScreen4:
            lcd.setCursor(1,0);
            lcd.print("SOLENOID CONTRL   ");   
            lcd.setCursor(1,1);
            lcd.print("PROBE CONTROL   ");
        break;

         case ServiceMenuScreen1:
            digitalWrite(GREEN_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);
            lcd.setCursor(1,0);
            lcd.print("PRODUCT TYPE  ");
            lcd.setCursor(1,1);
            lcd.print("SUBPRODUCT TYPE   "); 
        break;

        case ServiceMenuScreen2:
            lcd.setCursor(1,0);
            lcd.print("SUBPRODUCT TYPE  ");  
            lcd.setCursor(1,1);
            lcd.print("CALIBRATION   "); 
        break;

        case ServiceMenuScreen3:
            lcd.setCursor(1,0);
            lcd.print("CALIBRATION   ");    
            lcd.setCursor(1,1);
            lcd.print("SAFETY TEMP.   "); 
        break;

        case ServiceMenuScreen4:
            lcd.setCursor(1,0);
            lcd.print("SAFETY TEMP.   ");  
            lcd.setCursor(1,1);
            lcd.print("PROBE CALB "); 
        break;

        case ServiceMenuScreen5:
            lcd.setCursor(1,0);
            lcd.print("PROBE CALB   "); 
            lcd.setCursor(1,1);
            lcd.print("FACTORY RESET "); 
        break;

        case SDUServiceMenuScreen3:
            lcd.setCursor(1,0);
            lcd.print("CALIBRATION   ");    
            lcd.setCursor(1,1);
            lcd.print("FACTORY RESET   ");
        break;

        case CalibrationSettings:
            lcd.setCursor(0,0);
            lcd.print("SET CALIBRATION");
            lcd.setCursor(0,1);
            lcd.print(calibration_value,1);
            if(calibration_value<10.0){
                lcd.setCursor(3,1);
                lcd.print(" ");
                lcd.print("LITERS   ");
            }
            else
            {
                lcd.setCursor(4,1);
                lcd.print(" ");
                lcd.print("LITERS    ");

            }
        break;

        case SecondaryFillSettings:
            lcd.setCursor(0,0);
            lcd.print("SECONDARY FILL");
            lcd.setCursor(1,1);
            lcd.print("YES");
            lcd.setCursor(12,1);
            lcd.print("NO");
            break;

        case OperatingTimeSettings:
            lcd.setCursor(0,0);
            lcd.print("SET TIME");
            lcd.setCursor(0,1);
            lcd.print(optime[optimecounter]);
            if(optime[optimecounter]<10.0){
                lcd.setCursor(1,1);
                lcd.print(" ");
                lcd.print("HOURS ");

            }
            else
            {
                lcd.setCursor(2,1);
                lcd.print(" ");
                lcd.print("HOURS");

            }
        break;

        case ProductTypeSettings:
            lcd.setCursor(0,0);
            lcd.print("PRODUCT TYPE");
            lcd.setCursor(1,1);
            lcd.print("SDU");
            lcd.setCursor(12,1);
            lcd.print("DDU");
        break;

        case SubProductTypeSettings:
            lcd.setCursor(0,0);
            lcd.print("SUBPRODUCT TYPE   ");
            lcd.setCursor(0,1);
            if(dduflag){
            lcd.print("DDU");
            lcd.print(prodtype[prodtypecounter]);
            variant=((prodtype[prodtypecounter])/10);
            }
            else
            {
                lcd.print("SDU");
                lcd.print(prodtype[prodtypecounter]);
                variant=((prodtype[prodtypecounter])/10);

            }
        break;

        case SolenoidControlSettings:
            lcd.setCursor(0,0);
            lcd.print("SOLENOID CONTROL");
        break;

        case FlowControlSettings:
            lcd.setCursor(0,0);
            lcd.print("FLOW CONTROL");
        break;

        

        case ProcessScreen:
            digitalWrite(YELLOW_LED,HIGH);
            digitalWrite(RED_LED,LOW);
            if(process_flag)
            {
                lcd.setCursor(0,0);
                // digitalWrite(RED_LED,HIGH);
                // digitalWrite(YELLOW_LED,LOW);
                lcd.print("PROCESS STARTED");
            
                if ((one_second_counter - previous_time) >= time_per_step)
                {
                    previous_time = one_second_counter;
                    if (remaining_volume > 0)
                    {
                        remaining_volume--;
                        // Decrement 0.1L
                    }
                }
                lcd.setCursor(0,1);
                lcd.print("   ");
                lcd.setCursor(3,1);
                lcd.print(remaining_volume/10);
                lcd.print(".");
                lcd.print(remaining_volume%10);
                lcd.print(" LITERS         ");
                process_object.process_start();
            }
            else if(preheat_flag)
            {
                lcd.setCursor(0,0);
                // digitalWrite(YELLOW_LED,HIGH);
                // digitalWrite(RED_LED,LOW);
                lcd.print("PROCESS START  ");
                lcd.setCursor(0,1);
                lcd.print("PREHEATING...   ");
                process_object.boiler_preheat();
            }

            process_object.error_check();
            // if(!error_check_flag && !temp_drop_flag)
            // {
            //     process_flag=1;
            //     process_object.process_start();
            // }
        break;

        case PrimaryFillScreen:
            lcd.setCursor(0,0);
            lcd.print("PRIMARY BOILER   ");
            lcd.setCursor(0,1);
            lcd.print("FILLING...      ");
            process_object.error_check();
        break;

        case SecondaryFillTimer:
            process_object.error_check();
            lcd.setCursor(0,0);
            digitalWrite(RED_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);  
            
            lcd.print("SECONDARY BOILER");
            lcd.setCursor(0,1);
            lcd.print("FILLING...      ");
            if(!pauseflag)
            {
                Sec_time = pre_end_time - one_second_counter;
                // lcd.print(Sec_time);
            }
            // Sec_time = pre_end_time - one_second_counter;
            // lcd.print(Sec_time);
            // lcd.print("m ");


            if(!error_check_flag)
            {
            secondarytimerflag=1;
            process_object.secondary_fill();
            }
        break;

        case SafteyTemperatureSettings:
            lcd.setCursor(0,0);
            lcd.print("SET SAFTEY TEMP");
            lcd.setCursor(0,1);
            lcd.print("TEMP: ");
            lcd.print(Heatersafteytemp);
            lcd.print(" ");
            lcd.print((char)223);
            lcd.print("C  ");
        break;

        case ProbeCalibrationSettings:
            
            lcd.setCursor(0,0);
            lcd.print("PROBE CALB ");
            // lcd.print(Heater_temp,1 );
            // lcd.print(" ");
            lcd.print(calib_Heater1,1);
            lcd.print(" ");
            lcd.setCursor(0,1);
            lcd.print("Error  ");
            lcd.print(temp_error,1);
            lcd.print("  ");
        break;

        case TempSensorSettings:
            lcd.setCursor(0,0);
            lcd.print("PROBE OVERRIDE");
        break;

        case LevelSensorSettings:
            lcd.setCursor(0,0);
            lcd.print("LEVEL OVERRIDE");
        break;

        case FactoryResetScreen:
            lcd.setCursor(0,0);
            lcd.print("FACTORY RESET ");
            lcd.setCursor(1,1);
            lcd.print("YES");
            lcd.setCursor(12,1);
            lcd.print("NO");
        break;

        case ErrorScreen:
            if(error_check_flag)
            {
                digitalWrite(RED_LED,LOW);
                if(toggle)
                {
                    // if(zero_calib)                               // If error due to zero calibration
                    // {
                    //     digitalWrite(BUZZER,HIGH);
                    //     digitalWrite(YELLOW_LED,HIGH);
                    //     lcd.setCursor(0,0);
                    //     lcd.print(" CALIB MISSING!     ");
                    //     lcd.setCursor(0,1);
                    //     lcd.print("SET CALIBRATION   ");
                        
                    // }

                    if(closetap)                   // Solenoid Error
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("PROCESS COMPLETE    ");
                        lcd.setCursor(0,1);
                        lcd.print("CLOSE WATER TAP    ");
                        
                    }

                    if(flow_error_checkflag)      // If error due to flow issue
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("NO WATER SUPPLY  ");
                        lcd.setCursor(0,1);
                        lcd.print("CHECK WATER TAP");
                        
                        if(solenoid_stop == 0)
                        {
                            solenoid_stop=millis();
                        }
                        if(millis()- solenoid_stop >= 5000)
                        {
                            digitalWrite(SOLENOID1,HIGH);
                            if(dduflag && process_flag)
                            {
                            digitalWrite(SOLENOID2,HIGH);
                            }

                        }
                    }

                    if(waterlevel_error_flag  && !flow_error_checkflag)            // If error due to water level issue
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("  CHECK WATER");
                        lcd.setCursor(0,1);
                        lcd.print("  LEVEL SENSOR");
                    }
                    if(Probe1_Err && !flow_error_checkflag && !waterlevel_error_flag)             // If error due to temperature probe issue
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("  PROBE  ERROR    ");
                        lcd.setCursor(0,1);
                        lcd.print("CHECK TEMP PROBE    ");
                    }

                    

                }
                else
                {
                    digitalWrite(BUZZER,LOW);
                    digitalWrite(YELLOW_LED,LOW);
                    lcd.setCursor(0,0);
                    lcd.print("                 ");
                    lcd.setCursor(0,1);
                    lcd.print("                 ");
                }
            }
            else
            {
                lcd.clear();
                pauseflag=0;
                // primary_filling_flag=0;
                digitalWrite(BUZZER,LOW);
                digitalWrite(YELLOW_LED,LOW);
                if(secondarytimerflag)
                {
                    digitalWrite(BUZZER,LOW);
                    screen=SecondaryFillTimer;
                    process_object.Contactor1_start();
                    heater_start=1;
                }
                else if(process_flag || preheat_flag)
                {
                    digitalWrite(BUZZER,LOW);
                    screen=ProcessScreen;
                    process_object.Contactor1_start();
                    heater_start=1;
                    // if(dduflag)
                    // {
                    //     buzzerclass_object.heater_start();
                    // }
                }
                else
                {
                    digitalWrite(BUZZER,LOW);
                    screen=MainScreen;
                    mainscreenflag=1;
                    heater_start=0;
                    // Serial3.println("1");
                }
                digitalWrite(BUZZER,LOW);
            }

            if( !closetap  && (process_flag || secondarytimerflag || preheat_flag))           // If error is not due to zero calibration or solenoid issue and process is running or secondary timer is running → check for errors continuously
            {
            process_object.error_check();
            }
        break;

        default:

        break;
    }
    

}

lcdclass lcd_object = lcdclass();         