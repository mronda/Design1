/* 
 * File:   I2C.c
 * Author: maximiliano
 *
 * Created on March 24, 2018, 7:31 PM
 */

#include <xc.h>
#include <p18f4620.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "i2c.h"

/* Variables to control writing */
//static unsigned char WRITE_MODE = 0x00;
//static unsigned char READ_MODE = 0x01;
//static unsigned char RTC_ADDRESS = 0b1101111;  // RTC address given

static bool WRITE_MODE = 0;
static bool READ_MODE = 1;
static unsigned char RTC_ADDRESS = 0b11011110;  // RTC address given

void InitI2C_MasterMode()
{ 
    SSPCON1 = 0x28;     /*Enable SDA and SCK and also Master mode, clock 
                        * (FOSC/4x(SSPADD+1)) */
    
    SSPCON2 = 0x00;
    
    SSPADD = 0x09;      /* we we set this? now is 100khz */
    
    SSPSTAT = 0x80;     /* Slew rate disabled */
                                
    TRISCbits.TRISC3 = 1;   /* SCK need to be input for slave mode  */
    TRISCbits.TRISC4 = 1;   /* SDA need to be input for slave mode  */
    
//    SSPIE=1;		/* Enable SSP Interrupts*/
    SSPIF=0;
}

void I2C_Start()
{
    I2C_Wait();
    SSPCON2bits.SEN = 1;
}

void I2C_Stop()
{
    I2C_Wait();
    PEN = 1;
}

void I2C_Wait()
{
    while((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));    /* Wait for transfer to 
                                                     * finish */
}

/* Function writes to address byte and also writes to data of RTC*/
void I2C_WriteData(unsigned char data)
{
    I2C_Wait();
    SSPBUF = data;
}

/* Function writes only to the address of the slave */
void I2C_WriteControl(unsigned char slaveAddress, bool R_W_mode)
{
//    unsigned char tempSlaveAddress;  // Slave adddress
    
    if(R_W_mode) //if true meaning read 
    {
        slaveAddress = slaveAddress | (1 << 0);   //sets pm flag
    }
    else{
        slaveAddress = slaveAddress & ~(1 << 0);  //clear first bit to write
        
    }
   
    I2C_Wait();
    SSPBUF = slaveAddress;
}

unsigned char I2C_Read(unsigned char ack)
{
    unsigned char readData;
    
    I2C_Wait();
    RCEN = 1;
       
    I2C_Wait();

    NOP();
    
    readData = SSPBUF;
    I2C_Wait();
    if(ack) ACKDT = 0;
    else    ACKDT = 1;
    ACKEN = 1;
    return readData;
}

unsigned char I2C_ReadFromSlave(unsigned char addressByte)
{
    unsigned char dataFromAddress;
    I2C_Start();
    I2C_WriteControl(0b11011110, WRITE_MODE); // Write to slave 
    I2C_WriteData(addressByte);   // write to address where data is
    
    I2C_Start();

    I2C_WriteControl(0b11011110, READ_MODE); //slave address is first hardcoded

    dataFromAddress = I2C_Read(0);
    
    PEN = 1;    //stop condition, no need to call the stop function 
    
    return dataFromAddress;   
}

void I2C_WriteToSlave(unsigned char slaveAddress, unsigned char R_W_mode, 
        unsigned char addressByte, unsigned char data)
{
    I2C_Start();
    I2C_WriteControl(slaveAddress, R_W_mode); //write is 0 ()
    I2C_WriteData(addressByte);
    I2C_WriteData(data);
    I2C_Stop(); 
}
