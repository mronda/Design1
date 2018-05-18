/*
 * File:   timer2.c
 * Author: maximiliano
 *
 * Created on March 26, 2018, 12:29 PM
 */


#include <xc.h>
#include <p18f4620.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer2.h"

void Timer2Init(){
    PR2 = 255;      //getting 100ms timer
    T2OUTPS3 = 1;
    T2OUTPS2 = 1;
    T2OUTPS1 = 1;
    T2OUTPS0 = 1;
    T2CKPS1 = 1; // PRESCALE 16
    T2CONbits.TMR2ON = 1;
}
