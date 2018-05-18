/* 
 * File:   main.c
 * Author: maximiliano
 *
 * Created on April 4, 2018, 3:19 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "lcd_states.h"
#include "i2c.h"
#include "lcd.h"
#include "adc.h"
#include "gpio.h"
#include "timer2.h"
#include "timing_controller.h"
#include "bake_temp_set_controller.h"
#include "cooktime_controller.h"
#include "spi.h"

#define _XTAL_FREQ 8000000

#pragma config OSC=INTIO67,WDT=OFF,LVP=OFF,BOREN=OFF //use a different port
#pragma MCLRE=ON

void Baking();
void IfButtonPressedMovedToNextState(uint8_t nextState, uint8_t currentState);
void clearAlarm();
void CheckCookDone();
void GenerateAmplitude(unsigned int);

static volatile unsigned int readADCInput = 0;
static volatile unsigned int readBakeTempADC = 0;
static volatile unsigned int bakeTemp = 0;

/* Variable controls if alarm is set to buzz speaker at 3.5Khz*/
static volatile bool alarmSet;

/* Variable controls SPI sine wave out*/
static volatile bool sineOut = false;

/* Other variables for SPI */
volatile uint8_t iterator = 0;
static uint16_t *ptr;

/* Variable to get different amplitude depending on the temp input*/
static unsigned int amplitudeDivision;


int main(void) {
    OSCCONbits.IRCF = 0b111;    /* Running at 8mhz */
    while(!OSCCONbits.IOFS);
    
    TRISDbits.TRISD7 = 0;   // input on or off to allow RTC out
    LATDbits.LATD7 = 1;     // Turn on RTC
    
    //control 2 of mux
    TRISDbits.TRISD6 = 0;   // input on or off to allow spi out
    LATDbits.LATD6 = 0;     // Turn off spi
    
    /*---Initialize all register that I am using---*/
    lcd_init();                 // Initializes LCD
    AdcInit();                  // Initializes ADC
    InitI2C_MasterMode();       // Initialize I2C
    Timer2Init();
    GPIOInit();
    /*------------Initializations------------------*/
    
    ADCON1bits.PCFG = 0b1111;   // turn all IO pins back 
        
    /*------Enable Interrupts needed-----------*/
    INTCONbits.PEIE = 1;  
    INTCONbits.GIE = 1;     //  enable all interrupts
    TMR2IE = 1;             // enable timer2 interrupt
    /*-------Enable Interrupts-----------------*/
    
    LATDbits.LATD0 = 0;     // clear alarm GPIO
    LATDbits.LATD1 = 0;     // clear buzzer GPIO
    clearAlarm();
   
    uint16_t sineLookup[64] = {0x200,0x232,0x263,0x294,0x2c3,0x2f1,0x31c,0x344,
                               0x369,0x38b,0x3a9,0x3c3,0x3d8,0x3e9,0x3f5,0x3fd,
                               0x3ff,0x3fd,0x3f5,0x3e9,0x3d8,0x3c3,0x3a9,0x38b,
                               0x369,0x344,0x31c,0x2f1,0x2c3,0x294,0x263,0x232,
                               0x200,0x1cd,0x19c,0x16b,0x13c,0x10e,0xe3,0xbb,
                               0x96,0x74,0x56,0x3c,0x27,0x16,0xa,0x2,
                               0x0,0x2,0xa,0x16,0x27,0x3c,0x56,0x74,
                               0x96,0xbb,0xe3,0x10e,0x13c,0x16b,0x19c,0x1cd};
    
    ptr = sineLookup;
  
    while(1)
    {
        //checking flag for alarm 
        CheckCookDone();
        
        switch(LCD_STATES)
        {
            case TimeSet:
                TimeSetting(readADCInput);                
                IfButtonPressedMovedToNextState(WriteTimeState, TimeSet);
                break;
                
            case WriteTimeState:
                WriteTime();    
                WriteToRTC();
                lcd_clear();
                LCD_STATES = DisplayTimeState; //write to RTC and move immediately to BakeTime
                break;
                
            case DisplayTimeState:
                ReadRTCAndDisplay();
                
                __delay_ms(25);
                if(PORTDbits.RD2 == 1) //moves states to write
                {
                    lcd_clear();
                    LCD_STATES = BakeTempSetState;
                }
                else{
                    LCD_STATES = DisplayTimeState;
                }
                break;
            case BakeTempSetState:
                /* Returns a temp set by user and will be compared when baking*/
                bakeTemp = BakeTempSet(readBakeTempADC);
                GenerateAmplitude(bakeTemp); // get the heat element 
                NOP();

                __delay_ms(25);
                if(PORTDbits.RD2 == 1) //moves states to write
                {
                    lcd_clear();
                    
                    LCD_STATES = BakeTimeDisplay;
                }
                else if(PORTDbits.RD3 == 1)
                {
                    lcd_clear();
                    LCD_STATES = DisplayTimeState;     
                }
                else{
                    LCD_STATES = BakeTempSetState;
                }
                break;
            case BakeTimeDisplay:
                CookTimeDisplay(readADCInput);
                
                __delay_ms(80);
                if(PORTDbits.RD2 == 1) //moves states to write
                {
                    lcd_clear();
                    LCD_STATES = BakeTimeSet; //Writes all registers and starts SPI
                    NOP();
                }
                else if(PORTDbits.RD3 == 1)
                {
                    lcd_clear();
                    LCD_STATES = BakeTempSetState;
                }
                else{
                    LCD_STATES = BakeTimeDisplay;
                }
                break;
            case BakeTimeSet:
                CookTimeSet();  
                WriteTimeToCookToRTC();     
                LCD_STATES = InitSpiState; // move to output sine wave
                break;
                
            case InitSpiState:
                LATDbits.LATD7 = 0;     // Turn off control for RTC
                LATDbits.LATD6 = 1;     // Turn on control for SPI
                SpiInit();
                PR2 = 10;              // Change Period Register
                sineOut = true;
                
                
                LCD_STATES = BakingStart;
                break;
                
            case BakingStart:
//                ADCON1bits.PCFG = 0b1111;   // turn all IO pins back 

                lcd_command(0x85);
                char baking[] = "Baking...";
                lcd_string(baking);
//                
//                lcd_command(0x89);
//                lcd_char('|');
                
//                lcd_command(0xC0);
//                char bakeTempsetContainer[3];
//                sprintf(bakeTempsetContainer, "%u", bakeTemp);
//                lcd_string(bakeTempsetContainer); 
                
                lcd_command(0xC6);
                char bakeTempContainer[4]; 
                sprintf(bakeTempContainer, "%u", bakeTemp);
                lcd_string(bakeTempContainer);  
                lcd_shiftRight();
                lcd_char('F');
                
//                lcd_shiftRight();
//                lcd_char('F');
                
//                lcd_command(0xC9);
//                char cancel[] = "|Cancel";
//                lcd_string(cancel);  

                
                __delay_ms(30);
                if(PORTDbits.RD2 == 1) //moves states to write
                {
                    lcd_clear();
                    LCD_STATES = CancelBake;
                }
                else{
           
                    LCD_STATES = BakingStart;
                }
                
                //if clear we go back to show time and clear bake time and all
                break;
            case CookTimeDone:
                
                if(PORTDbits.RD2 == 1) //moves states to write
                {
                    PR2 = 255;
                    T2CKPS1 = 1; // PRESCALE 16
                    LATDbits.LATD1 = 0; //turn off speaker
                    alarmSet = false;
                    lcd_clear();
                    LCD_STATES = DisplayTimeState; //cancel alarm and cancel DAC out
                }
                else{
                    LCD_STATES = CookTimeDone;
                }                
                break;
                
            case CancelBake:
                WriteToDac(0x00,0);     //cancel out DAC
                LATCbits.LATC1 = 0;     // Turn off LED indicating Bake
                LATDbits.LATD6 = 0;     // Turn off control for SPI
                LATDbits.LATD7 = 1;     // Turn on control for RTC
                PR2 = 255;  //set timer 2 back to normal
                sineOut = false;
                InitI2C_MasterMode();
                clearAlarm();       //clear internal registers
                LCD_STATES = DisplayTimeState;
                break;
            default:
                break;
        }        
    }

}

void CheckCookDone()
{
    if(PORTDbits.RD0 == 1)
    {
         WriteToDac(0x00,0);     //cancel out DAC
        LATDbits.LATD6 = 0;     // Turn off control for SPI
        LATDbits.LATD7 = 1;     // Turn on control for RTC
        sineOut = false;
        InitI2C_MasterMode();
        
        clearAlarm();
        
        lcd_clear();        
        lcd_command(0x80);
        char bakeDone[] = "Bake Done!";
        lcd_string(bakeDone);
       
        lcd_command(0xCB);
        char clear[] = "Clear";
        lcd_string(clear);
        
        LCD_STATES = CookTimeDone;  /* Move to Cook Time Done and Ring */
        
        alarmSet = true;            /* Goes to Timer 2 and toggles alarm */
        PR2 = 1;                    /* Changing period for better sound */
    }
}

void clearAlarm()
{
    I2C_WriteToSlave(0b11011110, 0, 0x07, 0x00); // set aalarm ALM0
    __delay_ms(100);
    I2C_WriteToSlave(0b11011110, 0, 0x0D, 0x00); // set aalarm ALM0
      
}

void IfButtonPressedMovedToNextState(uint8_t nextState, uint8_t currentState)
{
    __delay_ms(80);
    if(PORTDbits.RD2 == 1) //moves states to write
    {
        LCD_STATES = nextState;
    }
    else{
        LCD_STATES = currentState;
    }
}

void GenerateAmplitude(unsigned int readAmplitude)
{
        if(readAmplitude >= 100 && readAmplitude < 150)
        {
            amplitudeDivision = 5;
        }
        else if(readAmplitude >= 150 && readAmplitude < 200)
        {
            amplitudeDivision = 4;
        }
        else if(readAmplitude >= 200 && readAmplitude < 250)
        {
            amplitudeDivision = 3;
        }
        else if(readAmplitude >= 250 && readAmplitude < 300)
        {
            amplitudeDivision = 2;
        }
        else if(readAmplitude >= 300)
        {
            amplitudeDivision = 1;
        }
        return;
}


void Baking() // SPI out here
{
//    char baking[] = "Baking";
//    unsigned int bakeTemp = readADCInput/3;
//    lcd_string(baking);
//    char adcContainer[2]; 
//    sprintf(adcContainer, "%u", bakeTemp);
//    lcd_command(0xC0); //print bake temp
//    lcd_string(adcContainer); 
}

void interrupt ISR(void)
{
    if(TMR2IF)
    {     
        if(sineOut)
        {
                   
            LATCbits.LATC1 = 1; // LED indicating baking is on
       
            if(iterator >= 64)
            {
                iterator = 0; 
            }

            WriteToDac(ptr[iterator]/amplitudeDivision, 0);
//            WriteToDac(1023, 0);
            iterator++; 
        }
        
        if(!sineOut)
        {
            /*Scan for ADC input on AN12*/
            ADCON1bits.PCFG = 0b0000;       // In analog input mode
            readADCInput = AdcRead(0x0C);   // Reading from CH 12
            readBakeTempADC = AdcRead(0x0A);   // Reading from CH 10
            ADCON1bits.PCFG = 0b1111;   // turn all IO pins back 
  
            if(alarmSet)
            {
                LATDbits.LATD1 ^= 1;
            }
        }
  
        TMR2IF = 0; //turn off interrupt  
        

    }
}


