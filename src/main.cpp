#include <Arduino.h>
// #include "Lcd_Display.h"
#include "Ext_Var.h"

#define SOLENOID1 13   // Solenoid valve 1 pin(Water inlet and primary comdenser)
#define SOLENOID2 12   // Solenoid valve 2 pin(Secondary condenser)

uint16_t time_counter=0;       // Counter for process timer
int one_second_counter=0;      // Process elapsed time (seconds)

uint16_t time_counter2=0;      // Additional counter used for errors
int one_second_counter2=0;     // Additional one second counter

bool pauseflag=0;              // Pauses process timer

# define PERIOD_EXAMPLE_VALUE (0x00C2)   // Timer period value


/*================ System Initialization ================*/
void setup() {

  lcd_object.lcd_setup(); 

  // eeprom_object.eeprom_defaultvalue();
  // eeprom_object.eeprom_datawrite();


  Serial3.begin(9600); 
  // Serial3.println("BOOT");
              // Initialize LCD
                // Start serial communication
  eeprom_object.eeprom_update_sensor();// Update EEPROM with Sensors default values
  
  buttonClass_object.button_setup();  // Initialize button module
  buzzerclass_object.buzzer_setup();  // Initialize buzzer module
  process_object.process_setup();     // Initialize process control
  PT100_object.PT100_setup();         // Initialize PT100 sensor
  // digitalWrite(SOLENOID1,HIGH);

  // eeprom_object.eeprom_datawrite(); // Write default EEPROM data
  // delay(300);

  eeprom_object.eeprom_dataread();    // Read saved EEPROM settings

  cli();                              // Disable interrupts during timer setup

  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;                           // Enable timer overflow interrupt
  TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;                   // Set timer in normal mode
  TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);                      // Disable event counting
  TCA0.SINGLE.PER = PERIOD_EXAMPLE_VALUE;                            // Load timer period value
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc | TCA_SINGLE_ENABLE_bm; // Set clock prescaler and enable timer

  sei();                              // Enable global interrupts
}


/*================ Timer Overflow Interrupt ================*/
ISR(TCA0_OVF_vect)
{
  time_counter2++;                    // Increment Additional timer

  if (time_counter2 >= 80)            // Check if ~1 second elapsed
  {
    one_second_counter2++;            // Increment Additional timer seconds
    time_counter2=0;                  // Reset additional counter
  }

  if((process_flag || secondarytimerflag) && !pauseflag) // Run timer when process active
  {
    time_counter++;                   // Increment process timer

    if (time_counter >= 80)           // Check if ~1 second elapsed
    {
      one_second_counter++;           // Increment process seconds
      // Serial3.println(one_second_counter); // Debug print
      time_counter=0;                 // Reset process counter
    }
  }

  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; // Clear interrupt flag
}


/*================ Main Execution Loop ================*/
void loop() {

  lcd_object.lcd_display();           // Update LCD screen
  buttonClass_object.button_ticks();  // Process button events
  buzzerclass_object.buzzer_update(); // Update buzzer state,Secondary heater start and stop ticker
  // buzzerclass_object.Buzzer_update();
  lcd_object.lcd_blink_update();      // Update LCD blinking during Error page
  process_object.ticker_update();     // Update process-related timers
  PT100_object.read_temperature();    // Read temperature from sensor
  // Serial3.println("1");

}