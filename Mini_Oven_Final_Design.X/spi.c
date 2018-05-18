/* 
 * File:   spi.c
 * Author: maximiliano
 *
 * Created on April 4, 2018, 4:53 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi.h"


void SpiInit(void){
/* Need to configure the following:
 * SSPCON1: Control Register, readable and writable
 * SSPSTAT: Status Register, lower 6 bits are read only, upper two are R/W
 * SSPBUF: Buffer register to which data bytes are written to or read from
 * SSPSR: Used for shifting data
 */
    TRISC = 0x00;
    TRISCbits.TRISC5 = 0; // * Serial Data Out (SDO)
    TRISCbits.TRISC4 = 1; // * Serial Data In (SDI). Automatically controller
    TRISCbits.TRISC3 = 0; // * Serial CLK (SCK)
    TRISAbits.TRISA5 = 1; // Slave 
    
    /*SMP:
     * SPI master mode sampled at end of output line*/    
    SSPSTATbits.SMP = 1;
    
    /* CKE:
     * SPI Select Bit
     * Transmission occurs on transition from active to Idle clock state*/
    SSPSTATbits.CKE = 0;
    
    SSPCON1bits.SSPEN = 1; //master synchronous serial port enabled bit
    
    SSPCON1bits.CKP = 1; // clk polarity set to high . Maybe dont need    
    
    /*SSPM:
     * Setting SSPM to 0010 = FOSC/64
     */    
    SSPCON1bits.SSPM3 = 0; 
    SSPCON1bits.SSPM2 = 0; 
    SSPCON1bits.SSPM1 = 1;
    SSPCON1bits.SSPM0 = 0; 
}

void WriteSpi(uint8_t data){
   
    SSPBUF = data; //get new data
    while(!PIR1bits.SSPIF); 
    PIR1bits.SSPIF = 0; //clear in software
    
}

void WriteToDac(uint16_t data, unsigned int frequency){
    TRISCbits.TRISC2 = 0; // set an output pin for CS
    
    PORTCbits.RC2 = 0;  // Active low to allow writing on DAC chanign for this 
    
    uint16_t command = 0xF000; 
    
    uint16_t output = command |(data<<2);   // Shift 
        
    uint8_t high = (uint8_t)(output>>8);
    
    uint8_t low = output & 0x00ff;
    
    WriteSpi(high);
    WriteSpi(low);
    
    PORTCbits.RC2 = 1; // Stop writing on DAC -- putting it up high 
}
