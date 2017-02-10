//
/*
 Will put function prototypes here for main.c as well as defines for u8 etc...
 
*/
//

#ifndef channelswitch_lib_h
#define channelswitch_lib_h

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>


#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

//LED-tastic macros
#define tog_led {PORTB ^= 0x01;}
#define clr_led {PORTB &= ~(0x01);}
#define set_led {PORTB |= 0x01;}

//Define maximum value for PWM
#define max_PWM 255

enum stateMachineModes{
    waiting,
    running,
    no_timer,
    conston
};  //waiting to enable the light, running on a timer, or constantly on.


//function prototype for main:
void shut_r_down(void);
void changeControlMode(void);
void setConstOn(void);

//functions to read/write EEPROM
u8 read_EEPROM(void);
void write_EEPROM(void);

#endif /* channelswitch_lib_h */
