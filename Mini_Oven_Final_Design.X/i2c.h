/* 
 * File:   I2C.h
 * Author: maximiliano
 *
 * Created on March 24, 2018, 7:23 PM
 */

#ifndef I2C_H
#define	I2C_H 

void InitI2C_MasterMode(void);  /* Has all of the initialization parameters */
void I2C_Wait(void);            /* Waits for flag to clear */
void I2C_Start(void);           /* Start condition */
void I2C_Stop(void);            /* Stop condition */
void I2C_WriteData(unsigned char);  /* Writes to address byte and or data */
void I2C_WriteControl(unsigned char, bool);    /* Writes to control byte */

void I2C_WriteToSlave(unsigned char slaveAddress, bool writeMode, 
        unsigned char addressByte, unsigned char data); /* Writes to slave */

unsigned char I2C_ReadFromSlave(unsigned char addressByte); /* Reads from slave */
unsigned char I2C_Read(unsigned char ack);  /* Used with read from slave */

#endif	/* I2C_H */

