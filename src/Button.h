#ifndef BUTTON_H
#define BUTTON_H




class buttonClass
{
    public:
    // float counter=0.0;
    buttonClass();
    void button_setup();
    void button_ticks();
    void but_check();
    void setPointer(uint8_t col,uint8_t row);
    static void user_settings();
    static void decrement();
    static void increment();
    static void back_screen();
    static void enter_function();
    static void long_press_up();
    static void long_press_down();
    static void back_to_home();
};

extern buttonClass buttonClass_object;

#endif