/* 
 * File:   adc.c
 * Author: maximiliano
 *
 * Created on March 26, 2018, 12:23 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "adc.h"

void AdcInit(void)
{
    ADCON2 = 0b10100101;
}

unsigned int AdcRead(char channel)
{
    ADCON0bits.CHS = channel;

    ADCON0bits.ADON = 1;    // Bit on

    ADCON0bits.GODONE = 1; // Start Converting

    while(ADCON0bits.GODONE){} // loop until false and stop converting

    ADCON0bits.ADON = 0;

    return ADRES;
}

