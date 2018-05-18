/* 
 * File:   time_alarm_controller.c
 * Author: maximiliano
 *
 * Created on April 10, 2018, 9:54 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "time_alarm_controller.h"


/* TimeSetting: Function sets up starting time on the LCD
 Takes in an ADC value to get the time and prints in on the LCd*/
void TimeSetting() 
{

    bool write = 0;
    lcd_command(0x80); //cursor home
    lcd_string("Time");
    
    while()
    lcd_command(0x80); //cursor home
    
    
    I2C_WriteToSlave(0b11011110, write, 0x00, 0x80); // seconds - writing 0 seconds start bit 1
    I2C_WriteToSlave(0b11011110, write, 0x01, 0x00); // minutes - writing 0 minutes 
    I2C_WriteToSlave(0b11011110, write, 0x02, 0x52); // hours - 12format, am, 12hrs
    I2C_WriteToSlave(0b11011110, write, 0x03, 0x01); // day of week MON = 01
    I2C_WriteToSlave(0b11011110, write, 0x04, 0x01); // day of month 01
    I2C_WriteToSlave(0b11011110, write, 0x05, 0x03); // month 03
    I2C_WriteToSlave(0b11011110, write, 0x06, 0x00); // year 00
    
    LCD_STATES = BakeTimeSet;    
    
}

