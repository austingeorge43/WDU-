#include <Arduino.h>
#include "Ext_Var.h"

#define MAX_SAFETY_TEMP 200
#define MIN_SAFETY_TEMP 50
#define SAFETY_TEMP_STEP 1
#define PROBE_CALIBRATION_LIMIT 20

#define RED_LED 17 //HELLO
#define YELLOW_LED 39
#define GREEN_LED 38

#define OPERATING_TIME 4

float counter=0.0;
float calibration_value=3.0;
float speed=0.5;
float Max_liter=0.0;
float temp_error=0.0;
long start_tt=0;
// float speed=1.0;
bool but1=0;
bool mainscreenflag=0;
bool usersettings=0;
bool servicemenu=0;
bool uppointer=0;
bool downpointer=0;
bool inmenu=0;
bool secondaryyes=0;
bool solenoidoverride=1;
bool flowoverride=0;
bool speedup=0;
bool secondarytimerflag=0;

bool factoryresetflag=1;

bool leveloverride=0;
bool probeoverride=0;
bool time_skip=0;

uint8_t skip_count=0;
uint8_t longpress_count=0;
int Heatersafteytemp=100;


// bool in_usersettings=0;

//Button Pins
#define UP 35
#define DOWN 32
#define BACK 26
#define MODE 36


OneButton up_button(UP, true);
OneButton down_button(DOWN, true);
OneButton back_button(BACK, true);
OneButton mode_button(MODE, true);

//DEFINATION

buttonClass:: buttonClass()
{}

void buttonClass:: button_setup()
{
    pinMode(UP,INPUT_PULLUP);
    pinMode(DOWN,INPUT_PULLUP);
    pinMode(BACK,INPUT_PULLUP);
    pinMode(MODE,INPUT_PULLUP);
    
    up_button.attachClick(increment);
    up_button.attachDuringLongPress(long_press_up);
    down_button.attachClick(decrement);
    down_button.attachDuringLongPress(long_press_down);
    mode_button.attachDuringLongPress(user_settings);
    mode_button.attachClick(enter_function);
    back_button.attachClick(back_screen);
    back_button.setPressMs(1000);
    back_button.attachDuringLongPress(back_to_home);
    
//  
}

void buttonClass:: button_ticks()
{
    up_button.tick();
    down_button.tick();
    mode_button.tick();
    back_button.tick();
}

void buttonClass :: setPointer(uint8_t col,uint8_t row)
{
    lcd.clear();              // Clear screen
    lcd.setCursor(col, row);  // Set position
    lcd.print(">");           // Print pointer
}

void buttonClass:: but_check()//------------------------------UP DOWN Key Long Press detection
{
  if(digitalRead(UP)== LOW && digitalRead(DOWN) == LOW && but1 == 0)//----------Check for button press
  {
    start_tt = millis();//-------if both keys are pressed start the time calculation
    but1 = 1;
  }
  else if(digitalRead(UP)== LOW && digitalRead(DOWN) == LOW && but1 == 1)//--------Check for button is continously pressed
  {
    if((millis() - start_tt) >= 1000)//-------------------after 3 sec completion
    {
        lcd.clear();
        screen=ServiceMenuScreen1;//------------Go to service menu mode.
        servicemenu=1;
        mainscreenflag=0;
        uppointer=1;
        downpointer=0;
        setPointer(0,0);
        digitalWrite(BUZZER,HIGH);
        buzzerclass_object.Buzzer_beep(1000);
        buzzerclass_object.Buzzer_start();
        return;
    }
  }
  else if(but1 == 1)
  {
    if(digitalRead(UP)== HIGH || digitalRead(DOWN) == HIGH)
    {
      but1 = 0;
    }
  }
}

void buttonClass:: back_to_home()
{
    if(!mainscreenflag  && !closetap){
    lcd.clear();
    digitalWrite(BUZZER,HIGH);
    buzzerclass_object.Buzzer_beep(1000);
    buzzerclass_object.Buzzer_start();
    
    // usersettings=0;
    // servicemenu=0;
    mainscreenflag=1;
    process_flag=0;
    temp_drop_flag=0;
    inmenu=0;
    secondarytimerflag=0;
    process_object.heater1_stop();
    buzzerclass_object.heater_stop();
    digitalWrite(SOLENOID1,LOW);
    digitalWrite(SOLENOID2,LOW);
    eeprom_object.eeprom_dataread();
    screen=MainScreen;
    if(!solenoidoverride && !usersettings && !servicemenu  ) 
    {
         Probe1_Err=0;
        waterlevel_error_flag=0;
        flow_error_checkflag=0;
        closetap=1;
        error_check_flag=1;
        screen=ErrorScreen;
        // screen=MainScreen;
    }
    else
    {

        usersettings=0;
        servicemenu=0;
        screen=MainScreen;
        // Probe1_Err=0;
        // waterlevel_error_flag=0;
        // flow_error_checkflag=0;
        // closetap=1;
        // error_check_flag=1;
        // screen=ErrorScreen;

    }  
    
    }

}

void buttonClass::long_press_up()
{
    if(digitalRead(DOWN)==HIGH)
    {
        if(mainscreenflag)
        {
            Max_liter=(2*OPERATING_TIME)*(variant/10.0);
            if(counter<Max_liter)
            {
                longpress_count++;
                if(longpress_count==4)
                {
                    counter+=0.5;
                    longpress_count=0;
                }
            }
        }
    

        if(screen==CalibrationSettings)
        {
            Max_liter=(2*OPERATING_TIME)*(variant/10.0);
            if(calibration_value<Max_liter)
            {
                longpress_count++;
                if(longpress_count==3)
                {
                    calibration_value+=0.1;
                    longpress_count=0;
                }
            }
        }
    
        if(screen==SafteyTemperatureSettings)
        {
            if(Heatersafteytemp<MAX_SAFETY_TEMP)
            {
            longpress_count++;
            if(longpress_count==5)
            {
                Heatersafteytemp+=SAFETY_TEMP_STEP;
                longpress_count=0;
            }
            }
        }

        if(screen==ProbeCalibrationSettings)
        {
            if(temp_error<=PROBE_CALIBRATION_LIMIT)
            {
                longpress_count++;
                if(longpress_count==5)
                {
                    temp_error+=0.1;
                    longpress_count=0;
                }
            
            }
        }
    }

}

void buttonClass::long_press_down()
{
    if(digitalRead(UP)==HIGH)
    {
        if(counter>=0.5)
        {
            if(mainscreenflag)
            {
                longpress_count++;
                if(longpress_count==4)
                {
                    counter-=0.5;
                    longpress_count=0;
                }
            }
        }
        if(screen==CalibrationSettings)
        {
            if(calibration_value>0.1)
            {
                longpress_count++;
                if(longpress_count==3)
                {
                    calibration_value-=0.1;
                    longpress_count=0;
                }
            }
        }

        if(screen==SafteyTemperatureSettings)
        {
            if(Heatersafteytemp > MIN_SAFETY_TEMP)
            {
            longpress_count++;
            if(longpress_count==5)
            {
                Heatersafteytemp-= SAFETY_TEMP_STEP;
                longpress_count=0;
            }
            }
        }
       if(screen==ProbeCalibrationSettings)
       {
            if(temp_error>=-(PROBE_CALIBRATION_LIMIT))
            {
                longpress_count++;
                if(longpress_count==5)
                {
                    temp_error-=0.1;
                    longpress_count=0;
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
            Max_liter=(2*OPERATING_TIME)*(variant/10.0);
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
        if(calibration_value>0.1)
        {
        calibration_value-=0.1;
        }
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
            break;

            case FactoryResetScreen:
                // lcd.clear();
                uppointer=0;
                downpointer=1;
                // lcd.setCursor(0,1);
                // lcd.print(">");
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

    if(mainscreenflag)
    {
        process_object.variant_settings();
        
        Serial3.println("BUZZZZZZZZZZZER");
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
            process_object.heater1_start();
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
            process_object.heater1_start();
            if(dduflag){
            buzzerclass_object.heater_start();
            }
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
                EEPROM.write(PRODUCT_SELECTION, dduflag);
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