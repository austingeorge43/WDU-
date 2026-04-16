#include <Arduino.h>
#include "Ext_var.h"

#define PROBE_OK_STABLE_COUNT 10    // Samples required to clear error
#define PROBE_ERROR_TEMP      255.0
#define PT100_PIN 27

// -------------------- RTD Constants --------------------
float R0 = 100.0;     
float R1 = 4700.0;
float R2 = 100.0;
float R3 = 4700.0;

const float Vs = 5.00;
const float Alpha = 0.00385;

// -------------------- Variables --------------------
float Vout = 0, TotalVout = 0, AvgVout = 0;
float Vin = 0, Rt = 0;
float Heater_temp = 0;
float calib_Heater1 = 0;

uint8_t probe1_ok_cnt = 0;
uint8_t Probe1_Err = 0;

int Probe1_actual = 0;   // calibration offset

// ----------------------------------------------------

// Definations
PT100::PT100()
{}

void PT100::PT100_setup()
{
    pinMode(PT100_PIN, INPUT);
}

void PT100::read_temperature()
{
    static uint16_t sample = 0;
    sample++;

    // --------- Sampling ---------
    if (sample < 50)
    {
        float instant_val = analogRead(PT100_PIN);
        Vout = (instant_val * Vs) / 1023.0;
        TotalVout += Vout;
    }


    if (sample >= 50)
    {
        AvgVout = TotalVout / 50.0;
        Vin = AvgVout / 10.0;

        Rt = (R2 * R3 + R3 * (R1 + R2) * Vin / Vs) /
             (R1 - (R1 + R2) * Vin / Vs);

        Heater_temp = ((Rt / R0) - 1) / Alpha;
        calib_Heater1 = Heater_temp + temp_error;
        // Serial3.print("Heater Temp: ");
        // Serial3.println(calib_Heater1);
        
        // PT100_object.PT100_error_check();

        // // =====================================================
        // //               PROBE ERROR LOGIC
        // // =====================================================
        
        // if (Heater_temp > PROBE_ERROR_TEMP)   // Open / Short condition
        // {
        //     Probe1_Err = 1;
        //     probe1_ok_cnt = 0;                // Reset recovery counter
        // }
        // else
        // {
        //     if (Probe1_Err == 1)
        //     {
        //         probe1_ok_cnt++;

        //         if (probe1_ok_cnt >= PROBE_OK_STABLE_COUNT)
        //         {
        //             Probe1_Err = 0;           // Clear error after stable readings
        //             probe1_ok_cnt = 0;
        //         }
        //     }
        // }

        // Reset sampling
        sample = 0;
        TotalVout = 0;
        AvgVout = 0;
    }
}


void PT100::PT100_error_check()
{
    // =====================================================
        //               PROBE ERROR LOGIC
        // =====================================================
        
        if (calib_Heater1 > PROBE_ERROR_TEMP)   // Open / Short condition
        {
            Probe1_Err = 1;
            probe1_ok_cnt = 0;                // Reset recovery counter
        }
        else
        {
            if (Probe1_Err == 1)
            {
                probe1_ok_cnt++;
                // Serial3.print("Probe OK Count: ");
                // Serial3.println(probe1_ok_cnt);

                if (probe1_ok_cnt >= PROBE_OK_STABLE_COUNT)
                {
                    Probe1_Err = 0;           // Clear error after stable readings
                    delay(500);
                    probe1_ok_cnt = 0;
                }
            }
        }

    // This function can be expanded for additional error checks if needed
}

PT100 PT100_object = PT100();