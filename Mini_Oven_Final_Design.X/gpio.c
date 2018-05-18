/* 
 * File:   gpio.c
 * Author: maximiliano
 *
 * Created on April 10, 2018, 9:49 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

/*
 * 
 */
void GPIOInit()
{
    /* Push Button 2: Drives next menu */
    TRISDbits.TRISD2 = 1;
    
    /* Push Button 1: Moves display */
    TRISDbits.TRISD3 = 1;
    
    /* Alarm GPIO for MC Flag*/
    TRISDbits.TRISD0 = 1;
    
    /* Buzzer Speaker GPIO */
    TRISDbits.TRISD1 = 0;
    
    /* LED GPIO To Indicate bake*/
    TRISCbits.TRISC1 = 0;
    LATCbits.LATC1 = 0; // LED indicating baking is on    
    
}

