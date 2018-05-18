/* 
 * File:   lcd.h
 * Author: maximiliano
 *
 * Created on March 24, 2018, 7:56 PM
 */

#ifndef LCD_H
#define	LCD_H


void lcd_string(char x[]);
void lcd_init(void);
void lcd_command(char);
void lcd_char(char);
void lcd_shiftRight(void);
void lcd_shiftLeft(void);
void lcd_clear(void);

#endif	/* LCD_H */

