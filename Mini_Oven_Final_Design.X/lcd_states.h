/* 
 * File:   lcd_states.h
 * Author: maximiliano
 *
 * Created on April 4, 2018, 4:22 PM
 */

#ifndef LCD_STATES_H
#define	LCD_STATES_H

static uint8_t LCD_STATES = 0;

enum
{
    TimeSet     = 0,
    TimeDisplay = 1,
    WriteTimeState = 2,
    DisplayTimeState = 3,
    BakeTempSetState = 4,
    BakeTimeDisplay = 5,
    BakeTimeSet = 6,
    InitSpiState = 7,
    BakingStart  = 8,
    CookTimeDone = 9,
    CancelBake = 10,
    
};

#endif	/* LCD_STATES_H */

