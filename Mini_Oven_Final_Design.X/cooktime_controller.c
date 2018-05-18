/* 
 * File:   cooktime_controller.c
 * Author: maximiliano
 *
 * Created on April 13, 2018, 12:03 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "cooktime_controller.h"
#include "lcd.h"
#include "i2c.h"

#define _XTAL_FREQ 8000000

static volatile unsigned int cookTimeAdcRead = 0;

/* To Write to RTC */
static volatile uint8_t setCookTimeMin  = 0;

/* To Read Curent min from RTC */
static volatile unsigned char currentMinFromRTC = 0;

void CookTimeDisplay(unsigned int adcRead)
{
    lcd_command(0x80); //cursor home
    char setBakeTimeDisplay[] = "Bake Time";
    lcd_string(setBakeTimeDisplay); 

    lcd_command(0x8A);
    char backMenu[] = "|Back";
    lcd_string(backMenu);
    
    lcd_command(0xCA);
    char nextMenu[] = "|Start";
    lcd_string(nextMenu);
    
    lcd_command(0xC6);  // Set cursor on MIN display
    char minDisplay[] = "MIN";
    lcd_string(minDisplay);

    //Read ADC from main
    cookTimeAdcRead = adcRead/17; //get reading from potentiometer
    
    if(cookTimeAdcRead >= 60)
    {
        cookTimeAdcRead = 59;
    }

    if(cookTimeAdcRead < 10)
    {
        lcd_command(0xC3);  //put cursor on min mark
        lcd_char('0');
        lcd_command(0xC4);  //put cursor on min mark
        char cookTimeContainer[1]; 
        sprintf(cookTimeContainer, "%u", cookTimeAdcRead);
        lcd_string(cookTimeContainer);
    }
    else{
        lcd_command(0xC3);  
        char cookTimeContainer[2]; 
        sprintf(cookTimeContainer, "%u", cookTimeAdcRead);
        lcd_string(cookTimeContainer);
    }
    
}

void WriteTimeToCookToRTC()
{
    bool WRITE_MODE = 0;
    
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x07, 0x10); // Sets Alarm ALM0
    
    //Dummy Values for alarm as not needed
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x0A, 0x00); // seconds to 00
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x0B, setCookTimeMin); // writing cook time for alarm 
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x0C, 0x61); // hours 1 - writing to hours in alarm
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x0D, 0b10010001); // writing Day 1 (mon) and min check and 
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x0E, 0x01); // Writing day 1 of the month
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x0F, 0x01); // Writing month 1 
}

uint8_t IntToBcd_cook_time(uint8_t x)
{
   	return ((x / 10) << 4) | (x % 10);
}

int bcd_to_decimal_cook_time(unsigned char x) {
    return x - 6 * (x >> 4);
} 

void CookTimeSet() //need to read current time
{
    /* Read from current minute to later add how much minutes to cook */ 
    uint8_t currentMin;
    currentMinFromRTC = I2C_ReadFromSlave(0x01);    //address for minutes
    currentMinFromRTC = currentMinFromRTC & 0x7F;     // get 7 bits because last bit is OSC
    currentMin = bcd_to_decimal_cook_time(currentMinFromRTC);      // Converted so that LCD can display 

    /* This is to write to RTC */
    setCookTimeMin  = (uint8_t)cookTimeAdcRead;
    setCookTimeMin  = setCookTimeMin + currentMin;  // This has cooktime + current min
    
    /* Takes care of going over 59 minutes*/
    if(setCookTimeMin > 59)
    {
        setCookTimeMin =  (uint8_t)cookTimeAdcRead - (uint8_t)1;
    }
    
    /* convert to BCD to write to RTC */
    uint8_t convertedCookMinutesToBCD = IntToBcd_cook_time(setCookTimeMin);
    setCookTimeMin  = convertedCookMinutesToBCD;
    
}