/* 
 * File:   spi.h
 * Author: maximiliano
 *
 * Created on April 4, 2018, 4:52 PM
 */

#ifndef SPI_H
#define	SPI_H

void SpiInit(void);
void WriteSpi(uint8_t data);
void WriteToDac(uint16_t data, unsigned int frequency);

#endif	/* SPI_H */

