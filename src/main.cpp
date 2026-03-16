#include <Arduino.h>
// #include "Lcd_Display.h"
#include "Ext_Var.h"

#define SOLENOID1 13
#define SOLENOID2 12

uint16_t time_counter=0;
int one_second_counter=0;

uint16_t time_counter2=0;
int one_second_counter2=0;
bool pauseflag=0;
# define PERIOD_EXAMPLE_VALUE (0x00C2)

void setup() {

  Serial3.begin(9600);
  lcd_object.lcd_setup();
  buttonClass_object.button_setup();
  buzzerclass_object.buzzer_setup();
  process_object.process_setup();
  PT100_object.PT100_setup();
  // digitalWrite(SOLENOID1,HIGH);

  // eeprom_object.eeprom_datawrite();
  // delay(300);
  eeprom_object.eeprom_dataread();

  cli();
  /* enable overflow interrupt */
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;

  /* set Normal mode */
  TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;

  /* disable event counting */
  TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);

  /* set the period */
  TCA0.SINGLE.PER = PERIOD_EXAMPLE_VALUE;

  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc | TCA_SINGLE_ENABLE_bm;/* set clock source (sys_clk/256) */
  /* start timer */
  sei();

  // put your setup code here, to run once:
  
}

ISR(TCA0_OVF_vect)
{
  time_counter2++;
  if (time_counter2 >= 80) // 60 / 0.0124 ≈ 4839
    {
      one_second_counter2++;
      time_counter2=0;
    }

  if((process_flag || secondarytimerflag) && !pauseflag)
  {
    time_counter++;
    if (time_counter >= 80) // 60 / 0.0124 ≈ 4839
    {
      one_second_counter++;
      Serial3.println(one_second_counter);
      time_counter=0;
    }
  }

    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

void loop() {
  lcd_object.lcd_display();
  buttonClass_object.button_ticks();
  buzzerclass_object.buzzer_update();
  // buzzerclass_object.Buzzer_update();                                 
  lcd_object.lcd_blink_update();
  PT100_object.read_temperature();
  // Serial3.println("1");
  // put your main code here, to run repeatedly:
}
