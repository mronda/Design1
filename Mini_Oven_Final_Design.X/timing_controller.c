/* 
 * File:   timing_controller.c
 * Author: maximiliano
 *
 * Created on April 12, 2018, 12:35 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "timing_controller.h"
#include "lcd.h"
#include "i2c.h"

#define _XTAL_FREQ 8000000

void WriteTime();
void WriteToRTC();

static volatile unsigned int hrAdcRead = 0;
static volatile unsigned int minAdcRead = 0;

static volatile uint8_t cursorLocation = 0;
static volatile uint8_t hr = 1;
static volatile uint8_t min = 2;

/* To set hour and minutes for RTC */
static volatile uint8_t setMin  = 0;
static volatile uint8_t setHr   = 0;

/* Static variables to read from RTC */
static volatile unsigned char minRead = 0;
static volatile unsigned char hrRead = 0;
static volatile unsigned char secondsRead = 0;

/* Controller to write time */
void TimeSetting(unsigned int readADCInput) 
{
    /* Using these booleans as states */
    lcd_command(0x80); //cursor home
    char setTimeDisplay[] = "Set Time";
    lcd_string(setTimeDisplay); //print time on LCD
    
    lcd_command(0x8B);
    char move[] = "|Move";
    lcd_string(move);
    
    lcd_command(0xCB);
    char next[] = "|Next";
    lcd_string(next);

    lcd_command(0xC2);  /* Move Cursor to add : command */
    lcd_char(':');
    
    lcd_command(0xC0); /* Move cursor to add time */
   
    /* Wait Button to move to next menu */
    /* Move cursor*/
        __delay_ms(30);
        if(PORTDbits.RD3 == 1)
        {
            cursorLocation++;
            if(cursorLocation == 1)
            {
                cursorLocation = hr;
            }            
            if(cursorLocation == 2)
            {
                cursorLocation = min;
            }
            if(cursorLocation > 2)
            {
              cursorLocation = 1;  
            }
        }
         
        if(cursorLocation == hr)
        {
            __delay_ms(30); /// wait a bit to get adc read
            hrAdcRead = readADCInput/85; //get reading from potentiometer
            if(hrAdcRead <= 0)
            {
                hrAdcRead = 1;
            }
            if(hrAdcRead < 10)
            {
                lcd_command(0xC0);  //put cursor on hour mark
                lcd_char('0');
                lcd_command(0xC1);  //put cursor on hour mark
                char hourContainer[1]; 
                sprintf(hourContainer, "%u", hrAdcRead);
                lcd_string(hourContainer);
            }
            else{
                lcd_command(0xC0);  //put cursor on hour mark
                char hourContainer[2]; 
                sprintf(hourContainer, "%u", hrAdcRead);
                lcd_string(hourContainer);
            }
        }
        
        if(cursorLocation == min)
        {
            __delay_ms(30); /// wait a bit to get adc read
            minAdcRead = readADCInput/17; //get reading in min from potentiometer 
           
            if(minAdcRead >= 60)
            {
                minAdcRead = 59;
            }
            
            if(minAdcRead < 10)
            {
                lcd_command(0xC3);  //put cursor on min mark
                lcd_char('0');
                lcd_command(0xC4);  //put cursor on min mark
                char minContainer[1]; 
                sprintf(minContainer, "%u", minAdcRead);
                lcd_string(minContainer);
            }
            else{
                lcd_command(0xC3);  
                char minContainer[2]; 
                sprintf(minContainer, "%u", minAdcRead);
                lcd_string(minContainer);
            }
        }
}


void WriteToRTC()
{
    bool WRITE_MODE = 0;
//    I2C_WriteToSlave(0b11011110, write, 0x00, 0x80); // seconds - writing 0 seconds start bit 1
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x01, setMin); // minutes - writing 0 minutes 
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x02, setHr); // hours - 12format, am, 12hrs
    
    //dummy writes
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x00, 0x80); // seconds - writing 0 seconds start bit 1
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x03, 0x01); // day of week MON = 01
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x04, 0x01); // day of month 01
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x05, 0x03); // month 03
    I2C_WriteToSlave(0b11011110, WRITE_MODE, 0x06, 0x00); // year 00
}

uint8_t IntToBcd(uint8_t x)
{
   	return ((x / 10) << 4) | (x % 10);
}

void WriteTime()
{
    setMin  = (uint8_t)minAdcRead;
    setHr   = (uint8_t)hrAdcRead;
    
    uint8_t convertedMinutesToBCD       = IntToBcd(setMin);
    uint8_t convertedHoursToBCD         = IntToBcd(setHr);   
    
    convertedHoursToBCD = convertedHoursToBCD | (1 << 5);   //sets pm flag
    convertedHoursToBCD = convertedHoursToBCD | (1 << 6);   //sets to 12 hour format
    
    /* Set the appropiate hour and minutes on the BCD scale that RTC understands */
    setMin  = convertedMinutesToBCD;
    setHr   = convertedHoursToBCD;
}


/*
 *
 *---------This part of the code handles the Reading of the RTC --------------*/
int bcd_to_decimal(unsigned char x) {
    return x - 6 * (x >> 4);
} 

void ReadAndDisplayMinFromRTC()
{
    uint8_t min;

    minRead = I2C_ReadFromSlave(0x01);  //address for minutes
    NOP();
    minRead = minRead & 0x7F;           // get 7 bits because last bit is OSC
    min = bcd_to_decimal(minRead);      // Converted so that LCD can display 
    NOP();
    
    if(min < 10)
    {
        lcd_command(0xC3);  //put cursor on min mark
        lcd_char('0');
        lcd_command(0xC4);  //put cursor on min mark
        char minContainerDisplay[1]; 
        sprintf(minContainerDisplay, "%u", min);
        lcd_string(minContainerDisplay);
    }
    else{
        lcd_command(0xC3);  
        char minContainerDisplay[2]; 
        sprintf(minContainerDisplay, "%u", min);
        lcd_string(minContainerDisplay);
    }
}

void ReadAndDisplayHrFromRTC()
{
    uint8_t hours;
    
    hrRead = I2C_ReadFromSlave(0x02); //adress for hours
    NOP();
    hrRead = hrRead & 0x1F; // get 7 bits becase last bit is OSC
    hours = bcd_to_decimal(hrRead);  
    
    if(hours < 10)
    {
        lcd_command(0xC0);  //put cursor on hour mark
        lcd_char('0');
        lcd_command(0xC1);  //put cursor on hour mark
        char hourContainerDisplay[1]; 
        sprintf(hourContainerDisplay, "%u", hours);
        lcd_string(hourContainerDisplay);
    }
    else{
        lcd_command(0xC0);  //put cursor on hour mark
        char hourContainerDisplay[2]; 
        sprintf(hourContainerDisplay, "%u", hours);
        lcd_string(hourContainerDisplay);
    }
}

ReadAndDisplaySecFromRTC()
{
    uint8_t seconds;
    
    secondsRead = I2C_ReadFromSlave(0x00); //address for seconds
    NOP();
    secondsRead = secondsRead & 0x7F; // get 7 bits because last bit is OSC
    seconds = bcd_to_decimal(secondsRead);
    
   
    if(seconds < 10)
    {
        lcd_command(0xC6);  //put cursor on second mark
        lcd_char('0');
        lcd_command(0xC7);  //put cursor on second mark
        char secondContainerDisplay[1]; 
        sprintf(secondContainerDisplay, "%u", seconds);
        lcd_string(secondContainerDisplay);
    }
    else{
        lcd_command(0xC6);  
        char secondContainerDisplay[2]; 
        sprintf(secondContainerDisplay, "%u", seconds);
        lcd_string(secondContainerDisplay);
    }
}

void ReadRTCAndDisplay()
{
    lcd_command(0x80);
    char time[] = "Time";
    lcd_string(time);
    
    lcd_command(0x8B);
    char deliminator[] = "|";
    lcd_string(deliminator);
    
    lcd_command(0xCB);
    char bakeT[] = "|Bake";
    lcd_string(bakeT);
    
    /* Read and Display Hours */
    ReadAndDisplayHrFromRTC();
    
    /* Add : for reading easiness */
    lcd_command(0xC2);
    lcd_char(':');
    
    /* Read And Display Minutes */
    ReadAndDisplayMinFromRTC();
   
    /* Add : for reading easiness */
    lcd_command(0xC5);
    lcd_char(':');
    
    /* Read And Display Seconds */
    ReadAndDisplaySecFromRTC();
 
}