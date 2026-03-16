#ifndef BUZZER_H
#define BUZZER_H

class buzzerclass
{
    public:
    buzzerclass();
    void buzzer_setup();
    void buzzer_on();
    void buzzer_off();
    void buzzer_beep();
    void buzzer_update();
    void Buzzer_start();
    void heater_start();
    void heater_stop();
    void Buzzer_beep(uint32_t interval1);
};

extern buzzerclass buzzerclass_object;

#endif