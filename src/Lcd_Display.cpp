#include <Arduino.h>
#include "Ext_Var.h"

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

Ticker error_blink(toggle_screen,500,0,MILLIS);
 
//DEFINATIONS
lcdclass::lcdclass()
{}

void toggle_screen()
{
    toggle=!toggle;
}

void lcdclass::lcd_setup()
{
    Wire.begin();              // Initialize the I2C bus
    Wire.setClock(100000);     // Set I2C clock speed to 100 kHz (standard speed)
    lcd.backlight();
    lcd.init();
    lcd.clear();
    lcd.begin(16,2);
    lcd.noAutoscroll();        // Disable auto-scrolling
    error_blink.start();
    pinMode(RED_LED,OUTPUT);
    pinMode(GREEN_LED,OUTPUT);
    pinMode(YELLOW_LED,OUTPUT);

    variant=((prodtype[prodtypecounter])/10);
    screen=VersionScreen;
}

void lcdclass:: lcd_blink_update()
{
    error_blink.update();

}

void lcdclass::lcd_display()
{
    // Serial3.print("Screen value: ");
    // Serial3.println(screen);
    // if(!dduflag)
    // {
    //     if()
    // }

    
    switch(screen)
    {
        case VersionScreen:
            lcd.setCursor(0,0);
            // lcd.print("Hi");
            lcd.print("LABQUEST BOROSIL");
            lcd.setCursor(3,1);
            lcd.print("WDU V3.00");
            digitalWrite(BUZZER,HIGH);
            // delay(1000);
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();
            delay(2000);
            screen++;
            lcd.clear();
        
        break;

        case MainScreen:
            mainscreenflag=1;
            usersettings=0;
            downpointer=0;
            // buttonClass_object.but_check();
        
            digitalWrite(YELLOW_LED,HIGH);
            digitalWrite(GREEN_LED,LOW);
            digitalWrite(RED_LED,LOW);
            lcd.setCursor(0,0);
            lcd.print(" SELECT VOLUME");     
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

            
            
            // lcd.clear();
            buttonClass_object.but_check();
            break;
        case UserSettingsScreen1:
            mainscreenflag=0;
            digitalWrite(GREEN_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);
            //lcd.clear();
            lcd.setCursor(1,0);
            lcd.print("SECONDARY FILL   ");
            // lcd.print("SOLENOID CONTRL   ");     
            lcd.setCursor(1,1);
            lcd.print("FLOW CONTROL    "); 
            // usersettings=0;
        break;

        case UserSettingsScreen2:
            lcd.setCursor(1,0);
            lcd.print("FLOW CONTROL   "); 
            // lcd.print("Operating Time");     
            lcd.setCursor(1,1);
            lcd.print("LEVEL CONTROL  ");
            // lcd.print("Product Type");
        break;
        
        case UserSettingsScreen3:
            lcd.setCursor(1,0);
            lcd.print("LEVEL CONTROL  ");
            // lcd.print("Solenoid Contrl");     
            lcd.setCursor(1,1);
            // lcd.print("Flow Control");
            lcd.print("SOLENOID CONTRL   ");
            // lcd.print("PROBE CONTROL   "); 
        break;

        case UserSettingsScreen4:
            lcd.setCursor(1,0);
            // lcd.print("Operating Time");
            lcd.print("SOLENOID CONTRL   ");
            // lcd.print("PROBE CONTROL   "); 
              
            // lcd.print("Solenoid Contrl");     
            lcd.setCursor(1,1);
            // lcd.print("Flow Control");
            lcd.print("PROBE CONTROL   ");
            // lcd.print("SOLENOID CONTRL   "); 
            
            // lcd.print("SOLENOID CONTRL"); 

        break;

         case ServiceMenuScreen1:
            digitalWrite(GREEN_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);
            lcd.setCursor(1,0);
            lcd.print("PRODUCT TYPE  ");
            // lcd.print("Solenoid Contrl");     
            lcd.setCursor(1,1);
            // lcd.print("Flow Control");
            lcd.print("SUBPRODUCT TYPE   "); 
        break;

        case ServiceMenuScreen2:
            lcd.setCursor(1,0);
            lcd.print("SUBPRODUCT TYPE  ");
            // lcd.print("Solenoid Contrl");     
            lcd.setCursor(1,1);
            // lcd.print("Flow Control");
            lcd.print("CALIBRATION   "); 
        break;

        case ServiceMenuScreen3:
            lcd.setCursor(1,0);
            lcd.print("CALIBRATION   ");
            // lcd.print("Solenoid Contrl");     
            lcd.setCursor(1,1);
            // lcd.print("Flow Control");
            lcd.print("SAFETY TEMP.   "); 
        break;

        case ServiceMenuScreen4:
            lcd.setCursor(1,0);
            lcd.print("SAFETY TEMP.   ");
            // lcd.print("Solenoid Contrl");     
            lcd.setCursor(1,1);
            // lcd.print("Flow Control");
            lcd.print("PROBE CALB "); 
        break;

        case ServiceMenuScreen5:
            lcd.setCursor(1,0);
            lcd.print("PROBE CALB   ");
            // lcd.print("Solenoid Contrl");     
            lcd.setCursor(1,1);
            // lcd.print("Flow Control");
            lcd.print("FACTORY RESET "); 
        break;

        case SDUServiceMenuScreen3:
            lcd.setCursor(1,0);
            lcd.print("CALIBRATION   ");    
            lcd.setCursor(1,1);
            lcd.print("FACTORY RESET   ");
        break;


        // case UserSettingsScreen5:
        //     lcd.setCursor(1,0);
        //     // lcd.print("Operating Time");
        //     //  lcd.print("Product Type"); 
        //     lcd.print("FLOW CONTROL   ");
        //     ;     
        //     lcd.setCursor(1,1);
        //     // lcd.print("Flow Control");
        //     // lcd.print("FLOW CONTROL   ");
        //     lcd.print("SECONDARY FILL   ");

        // break;

        // case SDUUserSettingsScreen5:
        //     lcd.setCursor(1,0);
        //     lcd.print("FLOW CONTROL   ");    
        //     lcd.setCursor(1,1);
        //     lcd.print("FACTORY RESET   ");
        // break;

        // case UserSettingsScreen6:
        //     lcd.setCursor(1,0);
        //     // lcd.print("Operating Time");
        //     //  lcd.print("Product Type"); 
        //     // lcd.print("PRODUCT TYPE   ");
        //      lcd.print("SECONDARY FILL   ");     
        //     lcd.setCursor(1,1);
        //     // lcd.print("Flow Control");
        //     // lcd.print("FLOW CONTROL   ");
        //     lcd.print("SAFETY TEMP.   ");
        // break;

        // case UserSettingsScreen7:
        //     lcd.setCursor(1,0);
        //     // lcd.print("Operating Time");
        //     //  lcd.print("Product Type"); 
        //     // lcd.print("PRODUCT TYPE   ");
        //      lcd.print("SAFETY TEMP.   ");     
        //     lcd.setCursor(1,1);
        //     // lcd.print("Flow Control");
        //     // lcd.print("FLOW CONTROL   ");
        //     lcd.print("PROBE CALB   ");
        // break;  

        // case UserSettingsScreen8:
        //     lcd.setCursor(1,0);
        //     lcd.print("PROBE CALB   ");     
        //     lcd.setCursor(1,1);
        //     lcd.print("FACTORY RESET");
        // break;


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
        // Serial3.println("1234");
            lcd.setCursor(0,0);
            digitalWrite(RED_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);
            lcd.print("PROCESS STARTED");
            // remaining_time = end_time - one_second_counter;
            // lcd.setCursor(5,1);
            // lcd.print(remaining_time / 60);
            // lcd.print("m ");

            if ((one_second_counter - previous_time) >= time_per_step)
            {
                previous_time = one_second_counter;
                if (remaining_volume > 0)
                {
                    remaining_volume--;
                      // Decrement 0.1L
                }
            }

            lcd.setCursor(3,1);
            lcd.print(remaining_volume/10);
            lcd.print(".");
            lcd.print(remaining_volume%10);
            lcd.print(" LITERS         ");


            process_object.error_check();
            // process_object.heater1_start();
            // process_object.heater2_start();

            if(!error_check_flag && !temp_drop_flag)
            {
            process_flag=1;
            process_object.process_start();
            }
        break;

        case SecondaryFillTimer:
            // digitalWrite(BUZZER,LOW); 
            process_object.error_check();
            lcd.setCursor(0,0);
            digitalWrite(RED_LED,HIGH); 
            lcd.print("SECONDARY BOILER");
            lcd.setCursor(0,1);
            lcd.print("FILLING...      ");
            // lcd.setCursor(11,1);
            if(!pauseflag)
            {
                Sec_time = pre_end_time - one_second_counter;
                // lcd.print(Sec_time);
            }
            // Sec_time = pre_end_time - one_second_counter;
            // lcd.print(Sec_time);
            // lcd.print("m ");

            // process_object.heater1_start();

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
            lcd.print(" C  ");
        break;

        case ProbeCalibrationSettings:
            
            lcd.setCursor(0,0);
            lcd.print("PR CAL ");
            lcd.print(Heater_temp,1 );
            lcd.print(" ");
            lcd.print(calib_Heater1,1);
            lcd.setCursor(0,1);
            lcd.print("Error   ");
            lcd.print(temp_error,1);
            // lcd.print(" C");

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
        
        // case SolenoidErrorScreen:
        //     lcd.setCursor(0,0);
        //     lcd.print("PROCESS COMPLETE ");
        //     lcd.setCursor(0,1);
        //     lcd.print("CLOSE WATER TAP");
        // break;
        case ErrorScreen:
            if(error_check_flag)
            {
                digitalWrite(RED_LED,LOW);
                if(toggle)
                {
                    if(closetap)
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("PROCESS COMPLETE    ");
                        lcd.setCursor(0,1);
                        lcd.print("CLOSE WATER TAP    ");
                        
                    }

                    if(flow_error_checkflag)
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("NO WATER SUPPLY  ");
                        lcd.setCursor(0,1);
                        lcd.print("   CHECK TAP");
                        
                        if(solenoid_stop == 0)
                        {
                            solenoid_stop=millis();
                        }
                        if(millis()- solenoid_stop >= 5000)
                        {
                            digitalWrite(SOLENOID1,HIGH);
                            digitalWrite(SOLENOID2,HIGH);

                        }
                    }

                    if(waterlevel_error_flag  && !flow_error_checkflag)
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("PRI.BOILER EMPTY");
                        lcd.setCursor(0,1);
                        lcd.print("CHECK FEED WATER");
                    }
                    if(Probe1_Err && !flow_error_checkflag && !waterlevel_error_flag)
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
                digitalWrite(BUZZER,LOW);
                digitalWrite(YELLOW_LED,LOW);
                if(secondarytimerflag)
                {
                    screen=SecondaryFillTimer;
                    process_object.heater1_start();
                }
                else if(process_flag)
                {
                    screen=ProcessScreen;
                    process_object.heater1_start();
                    if(dduflag)
                    {
                        buzzerclass_object.heater_start();
                    }
                }
                else
                {
                    screen=MainScreen;
                    mainscreenflag=1;
                    Serial3.println("1");
                }
            }
            if(!closetap  && (process_flag || secondarytimerflag))
            {
            process_object.error_check();
            }
        break;

        // case ServiceMenuScreen:
        //     lcd.clear();
        //     lcd.setCursor(0,0);
        //     lcd.print("HIIII");
            

        // break;

        default:

        break;
          
            
            


            // lcd.setCursor(0,1);
            // lcd.print(optime[optimecounter]);

            



            

    }
    

}

lcdclass lcd_object = lcdclass();         