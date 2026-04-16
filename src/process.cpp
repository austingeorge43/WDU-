#include <Arduino.h>
#include "Ext_Var.h"

uint16_t mint=0;
bool process_flag=0;
bool flow_error_checkflag=0;
bool error_check_flag=0;
bool waterlevel_error_flag=0;
bool temp_drop_flag=0;
bool probeerrorflag=0;
bool closetap=0;
bool primary_filling_flag=0;

unsigned long pf_start=0;
unsigned long Noflow_start_time = 0;
bool flow_timing = false;
// bool pauseflag=0;

// int Heatersafteytemp=30;
int resume_value=0;
int refill_tym=0;
int pre_end_time=0;
int end_time=0;
int variant=0;
int remaining_volume=0;
int u_input=0;
float time_per_step=0;
int previous_time=0;
int temp_error_count=0;

#define FLOW_SWITCH 24

#define SOLENOID1 13
#define SOLENOID2 12
#define HEATER1 15
#define HEATER2 14
#define CONTACTOR1 10
#define CONTACTOR2 11

#define POWER_DETECTION 5
#define WATER_LEVEL_SENSOR 4

#define STARTING_DELAY 2000

void heat1_start();
void contact2_start();
void heat2_start();

void contact1_stop();
void heat2_stop();
void contact2_stop();

Ticker T_heater1_start(heat1_start,STARTING_DELAY,1,MILLIS);
Ticker T_contactor2_start(contact2_start,(STARTING_DELAY),1,MILLIS);
Ticker T_heater2_start(heat2_start,(STARTING_DELAY),1,MILLIS);

Ticker T_contactor1_stop(contact1_stop,STARTING_DELAY,1,MILLIS);
Ticker T_heater2_stop(heat2_stop,(STARTING_DELAY),1,MILLIS);
Ticker T_contactor2_stop(contact2_stop,(STARTING_DELAY),1,MILLIS);




void heat1_start()
{
    if((process_flag || secondarytimerflag) && !error_check_flag )
        process_object.heater1_start();
}

void heat2_start()
{
    if((process_flag || secondarytimerflag) && !error_check_flag )
        process_object.heater2_start();
}


void contact2_start()
{
    if((process_flag || secondarytimerflag) && !error_check_flag )
        process_object.Contactor2_start();
}
//--------------------------------------------------------------------------

void contact1_stop()
{
    process_object.Contactor1_stop();
}

void heat2_stop()
{
    process_object.heater2_stop();
}

void contact2_stop()
{
    process_object.Contactor2_stop();
}

// #define POWER_DETECTION2 7

// Definations

process::process()
{}

void process:: process_setup()           // Initialising the Heaters,Solenoid,Contactors and Water level sensor
{
    pinMode(FLOW_SWITCH,INPUT_PULLUP);
    pinMode(SOLENOID1,OUTPUT);
    pinMode(SOLENOID2,OUTPUT); 
    pinMode(HEATER1,OUTPUT);
    pinMode(HEATER2,OUTPUT);
    pinMode(CONTACTOR1,OUTPUT);
    pinMode(CONTACTOR2,OUTPUT);
    pinMode(POWER_DETECTION,OUTPUT);
    pinMode(WATER_LEVEL_SENSOR,INPUT_PULLUP);
    // pinMode(POWER_DETECTION2,OUTPUT);
    digitalWrite(POWER_DETECTION,HIGH);
    // digitalWrite(POWER_DETECTION2,HIGH);

}

void process:: ticker_update(){
    T_heater1_start.update();
    T_heater2_start.update();
    T_contactor2_start.update();

    T_contactor1_stop.update();
    T_heater2_stop.update();
    T_contactor2_stop.update();

}  

void process:: variant_settings()      // Calculations for Process time and Secondary Fill time for different variant
{
    variant=((prodtype[prodtypecounter])/10);
    int final_calb=process_object.calculate_calibration();

    switch(prodtype[prodtypecounter])
    {
        case 150:                        // For 1.5 Liters
            // pre_end_time = 15;
            pre_end_time = 10*240;  //40 minutes (to pre-heat/fill the secondary boiler before starting normal operation.)
            end_time = (final_calb*4)*60;//---------------end_time in seconds
            // end_time =15;
        break;

        case 250:                        // For 2.5 Liters
            // pre_end_time = 25;
            pre_end_time = (11*144)+60;  //26.4 minutes
            end_time = ((final_calb*2.4)*60) + 60;//----end_time in seconds
            // end_time = 25;
        break;

        case 400:                          // FOr 4.0 Liters
        //  pre_end_time = 40;
            pre_end_time = 13*90;   //19.5 minutes
            end_time = (final_calb*1.5)*60 ;//-----------end_time in seconds
            // end_time = 40;
        break;

    }

    remaining_volume = u_input;

    // Calculate seconds required for 0.1L
    time_per_step = end_time / u_input;
    // Protection against division result 0
    if (time_per_step == 0)
        time_per_step = 1;

    previous_time = one_second_counter;

}

int process:: calculate_calibration()                    // Calibration for the product 
{
    u_input=counter*10;
   int actual_time = (variant * 60) / (calibration_value*10);
   int Calibrated_volume = (variant * actual_time) / 60;
   int Calb_vol_act = round((u_input * Calibrated_volume) / variant);
   return(Calb_vol_act); 
}

void process:: error_check()                  // Checks the Error,1)Flow Switch 2)Water Detection 3)Probe Error
{
    if(!flowoverride){
        if(digitalRead(FLOW_SWITCH)==HIGH)
        {
            // pauseflag=1; 
            // flow_error_checkflag=1;
            // error_check_flag=1;
             if (!flow_timing)   // first time HIGH detected
            {
                Noflow_start_time = millis();
                flow_timing = true;
            }

            if (millis() - Noflow_start_time >= 15000)
            {
                if(!flow_error_checkflag)
                {
                    solenoid_stop=0;
                    digitalWrite(SOLENOID1,LOW);
                    digitalWrite(SOLENOID2,LOW);
                }
                flow_error_checkflag = 1;
                
            }
        }
        else
        {
            // pauseflag=0;
            flow_error_checkflag=0; 
            flow_timing = false;
        // flow_error_checkflag = 0;
        }
    }
    else
    {
        flow_error_checkflag=0;
    }
    
    if(!leveloverride)
    {
        process_object.water_level_detection();
    }
    else
    {
        waterlevel_error_flag=0;
    }

    if(flow_error_checkflag || waterlevel_error_flag)
    {
        // error_check_flag=1;
        pauseflag=1;
        screen=ErrorScreen;
        if(!error_check_flag)
        {
            error_check_flag=1;
        // process_object.process_stop();
        process_object.heater1_stop();
        buzzerclass_object.heater_stop();
        }
        return;
    }
    else
    {
        digitalWrite(BUZZER,LOW);
        pauseflag=0;
        error_check_flag=0;
    }

    if(!probeoverride)
    {
        if(dduflag)                                 //  Probe Error should be active only if DDU flag is active
        {
            if(!probeoverride)
            {
                PT100_object.PT100_error_check();
                if(Probe1_Err)
                {
                    error_check_flag=1;
                    pauseflag=1;
                    temp_error_count=0;
                    screen=ErrorScreen;
                    if(!probeerrorflag)
                    {
                        // Serial3.println("hi");
                        error_check_flag=1;
                        probeerrorflag=1;
                        process_object.heater1_stop();
                        buzzerclass_object.heater_stop();
                    }
                    return;
                }
                else
                {
                    error_check_flag=0;
                    pauseflag=0;
                    probeerrorflag=0;   
                }
            }
            // PT100_object.read_temperature();
            // PT100_object.PT100_error_check();
            // Serial3.println("HI");
            if(process_flag)                            
            {
                if(calib_Heater1>Heatersafteytemp)       // If Secondary Boiler gets empty
                {
                    temp_error_count++;
                    // Serial3.print("Temperature Error Count: ");
                    // Serial3.println(temp_error_count);
                    if(temp_error_count>=100)
                    {
                        process_flag=0;
                        // process_object.heater1_stop();
                        process_object.heater1_start();
                        buzzerclass_object.heater_stop();
                        resume_value=one_second_counter-5; 
                        // error_check_flag=1;
                        // pauseflag=1;
                        lcd.clear();
                        screen=SecondaryFillTimer;
                        temp_drop_flag=1;
                        one_second_counter=0;
                        secondarytimerflag=1;
                    
                        temp_error_count=0; 
                    }
                    
                }
            }
            
            
            // if(Probe1_Err)
            // {
            //     error_check_flag=1;
            //     pauseflag=1;
            //     screen=ErrorScreen;
            //     if(!probeerrorflag)
            //     {
            //         Serial3.println("hi");
            //         error_check_flag=1;
            //         probeerrorflag=1;
            //         process_object.heater1_stop();
            //         buzzerclass_object.heater_stop();
            //     }
            // }
            // else
            // {
            //     error_check_flag=0;
            //     pauseflag=0;
            //     probeerrorflag=0;   

            // }
        }
    }

}

void process::water_level_detection()       // Water level Detection Fuction, It is closed for 6 sec and open for 1 sec
{
    uint8_t cycle_position = one_second_counter2 % 15;

    if (cycle_position >= 10)   // From second 10 to 14 (5 seconds ON)
    {
        digitalWrite(POWER_DETECTION, LOW);  // Sensor ON
        delay(100);

        if (digitalRead(WATER_LEVEL_SENSOR) == HIGH && !primary_filling_flag)  // Active low sensor
        {
            pf_start=millis();
            primary_filling_flag=1;
            lcd.clear();
            screen=PrimaryFillScreen;
            pauseflag=1;
            process_object.heater1_stop();
            // waterlevel_error_flag = 1;
        }

        else if(digitalRead(WATER_LEVEL_SENSOR) == LOW && primary_filling_flag)
        {
            primary_filling_flag=0;
            waterlevel_error_flag = 0;\
            pauseflag=0;
            lcd.clear();
            process_object.Contactor1_start();
            if(secondarytimerflag)
            {
                screen=SecondaryFillTimer;
                // process_object.heater1_start();
            }
            else
            {
                screen=ProcessScreen;
            }
        }
        else if(digitalRead(WATER_LEVEL_SENSOR) == HIGH && primary_filling_flag)
        {
            if(millis()-pf_start>=60000)   // If water level is low for more than 1 minutes
            {
                waterlevel_error_flag=1;
                // primary_filling_flag=0;
                // Serial3.println("1");
            }
        }
        else
        {
            waterlevel_error_flag = 0;
            // Serial3.println("0");
        }
    }
    else   // First 10 seconds OFF
    {
        digitalWrite(POWER_DETECTION, HIGH);  // Sensor OFF
    }
}

// void process:: water_level_detection()      // Water level Detection Fuction, It is closed for 6 sec and open for 1 sec
// {

//     uint8_t cycle_position = one_second_counter % 6;
//     if(cycle_position ==0)
//     {
//         digitalWrite(POWER_DETECTION,LOW);
//         delay(500);
//         if(digitalRead(WATER_LEVEL_SENSOR)==HIGH)    //Active low sensor
//         {
//             waterlevel_error_flag=1;
//         }
//         else
//         {
//             waterlevel_error_flag=0;
//         }
//         digitalWrite(POWER_DETECTION,HIGH);

//     }


// }

void process:: secondary_fill()           // This functions runs the Secondary boiler fill logic
{
    if(secondarytimerflag && !error_check_flag && !pauseflag)
    {
        if(one_second_counter>=pre_end_time)
        {
            lcd.clear();
            secondarytimerflag=0;
            one_second_counter=0;
            time_skip=0;
            lcd.setCursor(0,0);
            lcd.print("SECONDARY BOILER");
            lcd.setCursor(0,1);
            lcd.print("FILLING COMPLETED");
            digitalWrite(BUZZER,HIGH);
            delay(1000);
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();
            if(temp_drop_flag)                          //  If  fuction is called between the process because of secondary boiler empty
            {
                process_flag=1;
                temp_drop_flag=0;
                screen=ProcessScreen;
                process_object.Contactor1_start();
                // process_object.heater1_start();
                // if(dduflag)
                // {
                //     buzzerclass_object.heater_start();
                // }
                one_second_counter=resume_value;
                previous_time = one_second_counter;
                lcd.clear();
                
            }
            else                                        // If function is called from start
            {
                screen=ProcessScreen;
                lcd.clear();
                process_flag=1;
                mainscreenflag=0;
                one_second_counter=0;
                pauseflag=0;
                error_check_flag=0;
                flow_error_checkflag=0;
                process_object.Contactor2_start();
                // buzzerclass_object.heater_start();
                // process_object.heater1_stop();
            }
           
        }
    }

}

void process:: process_start()                    // Process timing funcution
{
    if(process_flag && !error_check_flag)
    {
    // end_time=5; //for testing purpose only. Comment this line during actual operation and uncomment the line below.
    if(one_second_counter>= end_time)
    {
        digitalWrite(BUZZER,HIGH);
        buzzerclass_object.Buzzer_beep(1000);
        buzzerclass_object.Buzzer_start();
        one_second_counter=0;
        lcd.clear();
        process_flag=0;
        process_object.heater1_stop();
        // buzzerclass_object.heater_stop();
        process_object.Solenoid1_stop();
        process_object.Solenoid2_stop();

        if(solenoidoverride)
        {
            screen=MainScreen;
            mainscreenflag=1;
        }
        else
        {
            closetap=1;
            error_check_flag=1;
            screen=ErrorScreen;

        }      
    }
    } 


}

void process:: heater1_start()
{
    // digitalWrite(SOLENOID1,HIGH);
    // digitalWrite(CONTACTOR1,HIGH);
    if(!primary_filling_flag  && !error_check_flag)
    {
        digitalWrite(HEATER1,HIGH);
        if(dduflag && process_flag  )
        {
            T_contactor2_start.start();
        }
    }
}
    
void process:: heater1_stop()
{
    // digitalWrite(SOLENOID1,LOW);
    // digitalWrite(CONTACTOR1,LOW);
    digitalWrite(HEATER1,LOW);
    T_contactor1_stop.start();
    
}

void process:: heater2_start()
{
     // solenoid2 on
    // digitalWrite(SOLENOID2,HIGH);
    // contactor2 on
    // digitalWrite(CONTACTOR2,HIGH);
    // heater2 on
    digitalWrite(HEATER2,HIGH);
}

void process:: heater2_stop()
{
    // solenoid2 off
    // digitalWrite(SOLENOID2,LOW);
    // contactor2 off
    // digitalWrite(CONTACTOR2,LOW);
    // heater2 off
    digitalWrite(HEATER2,LOW);
    T_contactor2_stop.start();
}

void process:: process_stop()
{
    digitalWrite(SOLENOID1,LOW);
    digitalWrite(CONTACTOR1,LOW);
    digitalWrite(HEATER1,LOW);
    digitalWrite(SOLENOID2,LOW);
    digitalWrite(CONTACTOR2,LOW);
    digitalWrite(HEATER2,LOW);
}

void process:: Contactor1_start()
{
    digitalWrite(SOLENOID1,HIGH);
    delay(50);
    digitalWrite(CONTACTOR1,HIGH);
    T_heater1_start.start();
}

void process:: Contactor2_start()
{
    digitalWrite(SOLENOID2,HIGH);
    delay(50);
    digitalWrite(CONTACTOR2,HIGH);

    T_heater2_start.start();
}

void process:: Contactor1_stop()
{
    digitalWrite(CONTACTOR1,LOW);
    if(dduflag)
    {
        T_heater2_stop.start();
    }
}

void process:: Contactor2_stop()
{
    digitalWrite(CONTACTOR2,LOW);
}


void process:: Solenoid1_stop()
{
    digitalWrite(SOLENOID1,LOW);
}

void process:: Solenoid2_stop()
{
    digitalWrite(SOLENOID2,LOW);
}

process process_object=process();