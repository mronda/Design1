/* 
 * File:   bake_temp_set_controller.c
 * Author: maximiliano
 *
 * Created on April 12, 2018, 2:23 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "bake_temp_set_controller.h"
#include "lcd.h"

#define _XTAL_FREQ 8000000

static volatile unsigned int tempAdcRead = 0;
static volatile unsigned int getModOfTemp = 0;

/* Bake Temp increments by 5 starting at 100F - 400F*/
unsigned int BakeTempSet(unsigned int adcRead)
{
    //150 adc/4 and top temp is 355
    
    lcd_command(0x80); //cursor home
 
    char setBakeTempDisplay[] = "Bake Temp";
    lcd_string(setBakeTempDisplay); //print time on LCD
    
    lcd_command(0x8B);
    char deliminator[] = "|Back";
    lcd_string(deliminator);
    
    lcd_command(0xCB);
    char nextMenu[] = "|Next";
    lcd_string(nextMenu);
    
    lcd_command(0xC8);
    lcd_char('F');
    
    lcd_command(0xC4); /* Move cursor to add temo */
    __delay_ms(30);
    tempAdcRead = adcRead/4; //get reading from potentiometer
    tempAdcRead += 100;
    getModOfTemp = tempAdcRead % 5; //0 if 100
    tempAdcRead = tempAdcRead - getModOfTemp;
//    NOP();
   
    char bakeTempContainer[3]; 
    sprintf(bakeTempContainer, "%u", tempAdcRead);
    lcd_string(bakeTempContainer);  
    
    return tempAdcRead;
}
