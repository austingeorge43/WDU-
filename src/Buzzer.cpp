#include <Arduino.h>
#include "Ext_var.h"
// #include "Ticker.h"

#define BUZZER 18
void Buzzer_on_off();
void heater2_start();
void heater_stop();
Ticker buzzer(Buzzer_on_off,1000,0,MILLIS);
Ticker heater(heater2_start,5000,0,MILLIS);
Ticker heaterstop(heater_stop,5000,0,MILLIS);

// Definations

buzzerclass::buzzerclass()
{}

void buzzerclass:: buzzer_setup()
{
    pinMode(BUZZER,OUTPUT);
}

void buzzerclass:: buzzer_update()
{
    buzzer.update();
    heater.update();
    heaterstop.update();
}

void buzzerclass ::Buzzer_start()
{
   buzzer.start();
}

void buzzerclass ::heater_start()
{
   heater.start();
}

void buzzerclass :: heater_stop()
{
    Serial3.println("Heater Stop");
    heaterstop.start();
}

void buzzerclass ::Buzzer_beep(uint32_t interval1)
{
    buzzer.interval(interval1);
    // buzzer.start();
}

void buzzerclass:: buzzer_on()
{
    digitalWrite(BUZZER,HIGH);

}

void buzzerclass:: buzzer_off()
{
    digitalWrite(BUZZER,LOW);

}

void buzzerclass:: buzzer_beep()
{
    buzzerclass_object.buzzer_on();
    delay(1000);
    buzzerclass_object.buzzer_off();
}

void Buzzer_on_off()
{
    digitalWrite(BUZZER,LOW);
    buzzer.stop(); 
}

void heater2_start()
{
    process_object.heater2_start();
    heater.stop();
}

void heater_stop()
{
    process_object.heater2_stop();
    heaterstop.stop();
}

buzzerclass buzzerclass_object = buzzerclass();