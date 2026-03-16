#ifndef LCD_Display_H
#define LCD_Display_H

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>



class lcdclass
{
    public:

    lcdclass();
    void lcd_setup();
    void lcd_blink_update();
    void lcd_display();


};

 extern LiquidCrystal_I2C lcd;        //Object for Predefined class Library
 extern lcdclass lcd_object;          //Object for user defined class


#endif