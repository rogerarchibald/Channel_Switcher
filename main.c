/*
 
This is the PIR-equipped Power LED driver board for channel-lights and anything else that needs some power LED's.  There are two main modes in which this can work: Constant-on or Controlled Mode, explained below.
 
    Constant-on: When you power it up, the light ramps up.  It's just always on.  Initially wasn't planning to have this option but I added this functionality for a friend who wanted to run this off a light switch (run a wall-wart off the light switch).  In this mode the LED drive is on until power is removed or the board is changed to Controlled Mode
 
    Controlled Mode: The lights are controlled via the PIR sensor and the on-board button.  This was the intent when I first built the boards.
 
 When power is first applied to the board, if the lights come up automatically then the board is in Constant-on mode.  If they do not come up automatically then the board is in Controlled Mode.  At any time, to switch between modes just press and hold the button for 4 seconds, and the board will toggle between the two modes.  When it toggles, the current state is saved in on-board EEPROM and remembered for next power-up.
 
 
 
 When in Controlled Mode there are several different states, the flow of which will be as follows:
 
 From power-on, the LED is off.  To turn it on, a quick press of the button will make LED ramp up and a 5-minute timer will be started.  Every time that the ADC detects motion on the PIR, that 5-minute timer is reset.
 
 When the 5-minute timer times out, the LED will ramp off.  After it ramps off, anytime the ADC detects new PIR motion it will ramp back up and the cycle will repeat.
 
 In Controlled mode, any time that the LED is on when the button is pressed, the LED will immediately turn off. When off due to a button press, the only way to get them back on is to press the button (i.e. the PIR won't re-activate the light if it was shut off rather than being allowed to ramp down)
 
 If the light is off, a long press (1 second) will cause the light to come on and stay on.  It will stay on until another press is detected and then it will go back to a 'waiting for button-press' state.
 
 
 So, we will have the following states:
 
 Off, need button press to turn on
 Off, will turn on if PIR motion is detected
 On, will turn off after 5 minutes of not seeing motion
 On, will maintain on until a button press is detected
 Ramping from On to Off
 Ramping from Off to On
 
 
 The states, and transitions between states will be controlled by the PIR sensor, the button and time.
 
 PIR Sensor will be an ADC value on PB4/ADC2
 The Button input will be on PB3, the pin has an external pullup
 The PWM output for the LED is on PB1/OC0B and has an external pulldown.
 0603 LED is on PB2 and will turn on when motion is detected.
 
 Will use Timer0 for the PWM and Timer1 for the mS timer
 
Fuse settings are as follows to set the clock and enable brown-out detector so we don't get squirrely reading EEPROM on startup
 L: 0xE2
 H: 0xDD
 E: 0xFF
 
 
*/

#include "channelswitch_lib.h"
#include "Timer.h"
#include "ADC.h"
#include <util/delay.h>
#include <avr/eeprom.h>


u8 state = waiting;  //this is the state of the state machine.
u8 PIR_enable = 0;  //this will determine if the PIR alone is enough to start things (when 1)or if I need a button press(when 0).
u8 CNTRL_Mode = 0;  //when CNTRL_Mode == 0, the button/PIR sensor/switch-case below determine light function.  When it == 1, the light is on when power is applied

int main(void) {
    u8 tempADC; //will just read the ADC once per cycle and keep the value here
    PORTB = 0;  //make sure we're low before hitting the DDR to try to avoid a flick on the output
    DDRB = 0x07;    //PB4 and PB3 are inputs, everything else output
    _delay_ms(250); //wait a tick to make sure that VCC is up and stable before reading EEPROM (BOD is also enabled but why not spend a .25 second to be sure?
    initTimer1();   //timer1 will be my (5)mS timer
    initTimer0();   //Timer0 is the PWM for the LED
    initADC();
    sei();  //enable Global Interrupts
    CNTRL_Mode = read_EEPROM(); //get value from EEPROM
    if(CNTRL_Mode){
        setConstOn();   //if the CNTRL_Mode isn't 0, set this thing up for autopilot.
    }
 
	while (1) {
        
  
        tempADC = checkADC();   //will return a 0 if the baseline is within the envelope, return 1 if it's outside.
        
        if(tempADC){
            set_led} else {clr_led} //just for a visual of what's happening with the PIR
      
        
        
        switch (state) {
            
                
        case waiting:
                if((checkquickpress()) || (PIR_enable && tempADC)){
                    set_PWMVal(max_PWM);
                    resetTimer();
                    PIR_enable = 1;
                     state = running;
                }
                break;
                
                
        case running:
            
                if(tempADC){
                  resetTimer();
                }
                if(check4press()){
                    shut_r_down();
                }
            
            break;
            
                
        case no_timer:
                
            if(check4press()){
                shut_r_down();
                }
                break;
                
                
        case conston:
                //not much to do here, it just sits.  
                
                break;
                
        }   //end of switch/case
        
    
    

    }   //end of while1
	return 0; // never reached

} //end of main



//hard turn-off and PIR disable due to a button push or a time-out
void shut_r_down(void){
    set_PWMVal(0);
    OCR0B = 0;
    state = waiting;
    PIR_enable = 0;
}


//toggle between working via the 3 states (waiting, running, no_timer) or working so that when it's powered up, the light is on.
void changeControlMode(){
    if (CNTRL_Mode == 0){
        CNTRL_Mode =1 ;  //toggle the mode
        setConstOn();
 
    }else{
        CNTRL_Mode = 0;
        shut_r_down();
     
    }
    write_EEPROM(); //write CNTRL_Mode back to EEPROM so it will be remembered at startup.
}

//function which will prime the light to turn on and set the mode to ignore short/medium button presses.  Will be called at the start of Main if we detect that the CNTRL_Mode bit was set from EEPROM, or else will be called as the light is switched from CNTRL_Mode 0 to CNTRL_Mode 1.
void setConstOn(void){
    OCR0B = 0;  //kill the LED before ramping it up, just to make the change obvious
    set_PWMVal(max_PWM);
    state = conston; //make the switch/case stop looking at buttons and lights and whatnot
}




//following two functions are to read/write the EEPROM to

//will read the value stored in EEPROM space 35
u8 read_EEPROM(void){
    u8 temp;
    temp = eeprom_read_byte((u8*)35);
    return (temp);
}

//toggle LSB of button_less and write to EEPROM
void write_EEPROM(void){
    eeprom_write_byte((u8*)35, CNTRL_Mode);
  }


