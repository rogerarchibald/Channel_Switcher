//
//  Timer.h
//  channel_Switcher
//
//  Created by Roger on 6/27/16.
//  Copyright © 2016 Roger. All rights reserved.
//

#ifndef Timer_h
#define Timer_h

#include "channelswitch_lib.h"


//function prototypes:
void initTimer1 (void);
void initTimer0 (void); //2Khz for PWM


u8 button_read(void);   //read the button
u8 check4press(void);   //see if the button has been pressed
u8 checkquickpress(void);   //see if the button has been pressed and released quickly
void set_PWMVal(u8 targetval);  //tell the mS ISR where I want OCR0B to end up
void resetTimer(void);      //reset the 5-minute timeout for the PIR


#endif /* Timer_h */
