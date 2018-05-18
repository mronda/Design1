/* 
 * File:   cooktime_controller.h
 * Author: maximiliano
 *
 * Created on April 13, 2018, 12:03 PM
 */

#ifndef COOKTIME_CONTROLLER_H
#define	COOKTIME_CONTROLLER_H

/* Displays user the amount of time they want to cook for */
void CookTimeDisplay(unsigned int adcRead);     

/* Set the cook time on the RTC  */
void CookTimeSet();  

/* Writing to RTC for alarm */
void WriteTimeToCookToRTC();

#endif	/* COOKTIME_CONTROLLER_H */

