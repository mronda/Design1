/*
 * File:   lcd.c
 * Author: maximiliano
 *
 * Created on March 24, 2018, 7:56 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"

#define _XTAL_FREQ 8000000

void lcd_string(char x[])
{
	int i = 0;
	while(x[i] != 0)
	{
		lcd_char(x[i]);
		i++;
	}
	return;
}

void lcd_init(void){
    lcd_command(0x33);
    lcd_command(0x32);
    lcd_command(0x2C);
    lcd_command(0x0C);
    lcd_command(0x01);    //clear scren
}

void lcd_command(char x){

     char temp;

     TRISAbits.TRISA0 = 0; //d4
     TRISAbits.TRISA1 = 0; //d5
     TRISAbits.TRISA2 = 0; //d6
     TRISAbits.TRISA3 = 0; //d7
     TRISAbits.TRISA4 = 0; //rs
     TRISAbits.TRISA7 = 0; //e

     temp = x;

     PORTA = 0;
     __delay_ms(5);
     x = x >>4;
     x = x & 0xF;
     x = x | 0x80;
     PORTA = x;
    __delay_ms(5);
     x = x & 0xF;
     PORTA = x;
     __delay_ms(5);
     PORTA = 0;
     __delay_ms(5);

     x = temp;
     x = x & 0xF;
     x = x | 0x80;
     PORTA = x;
     __delay_ms(5);
     x = x & 0xF;
     PORTA = x;
     __delay_ms(5);
}

void lcd_shiftRight()
{
        lcd_command(0x14); //shifts right once
}

void lcd_clear()
{
    lcd_command(0x01);
}

void lcd_shiftLeft()
{
       lcd_command(0x10); // shift left once
}

void lcd_char(char x){
     char temp;
     TRISAbits.TRISA0 = 0;
     TRISAbits.TRISA1 = 0;
     TRISAbits.TRISA2 = 0;
     TRISAbits.TRISA3 = 0;
     TRISAbits.TRISA4 = 0;
     TRISAbits.TRISA7 = 0;
     temp = x;
     PORTA = 0x10;
      __delay_ms(1);
     x = x >>4;
     x = x & 0xF;
     x = x | 0x90;
     PORTA = x;
      __delay_ms(1);
     x = x & 0x1F;
     PORTA = x;
      __delay_ms(1);
     PORTA = 0x10;
      __delay_ms(1);
     x = temp;
     x = x & 0xF;
     x = x | 0x90;
     PORTA = x;
      __delay_ms(1);
     x = x & 0x1F;
     PORTA = x;
      __delay_ms(1);
}

