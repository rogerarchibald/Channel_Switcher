//
/*
 Timer file here to run the mS timer which will keep track of how long this whole thing has been on in addition to the PWM of the LED drive.  
 
 LED will PWM at ~250hz (Timer0) and my mS timer will make an interrupt every 5mS (Timer1).  This will give plenty of resolution for debouncing but will keep the accumulation numbers in a reasonable realm.  5 minutes /5mS = 60,000...Goin 16-bit here.
 */







//

#include "Timer.h"

static u16 onTimeLeft;  //when the lights are on, this is the 5-minute timer that will be reset by the PIR or will time-out and trigger the lights shutting down
static u8 PWMTarget = 0;
static u8 freshPress = 0;   //flag to see if the button has been pressed
static u8 quickPress = 0;   //flag to check if the button has been pressed and released quickly
static u8 longPressArm = 1; //arm this to check if the button has been held down for a long time as will be done to switch control methods.
extern u8 state; //this is the state machine controller from main.c


#define PWM_Interval 2  //How many cycles of the ISR in between step changes in the PWM rate when ramping
#define debounce_time 10   //how many consistently hi button cycles of my mS timer I need before I'll re-arm a falling edge.  10 x 5 = 50mS, seems good


//initializing Timer1 for my mS timer...Want a 5mS rollover.  8Mhz clock over a 512 prescaler = 15.625K clock into Timer, i.e. 64uS period.  I want 5mS so I'll have a CTC value of 78.  64uS * 78 = 4.992mS, close enough for me

void initTimer1 (void){
    OCR1A = 78;    //this will allow me to generate a compare/match ISR
    OCR1C = 78;    //this is my CTC value
    TCCR1 = (1 << CTC1) | (1 << CS13) | (1 << CS11);  //Enable CTC and select clock source as a /512 prescaler
    TIFR |= (1 << OCF1A); //clear interrupt flag
    TIMSK |= (1 << OCIE1A); //enable interrupt on Timer1 Output Compare RegisterA match
}




void initTimer0 (void){
    //Set Timer0 up for a Phase/Frequency corrected PWM with ~250Hz frequency on OC0B
    OCR0B = 0;
    TCCR0A = (1 << COM0B1) | (1 << WGM00);  //set up PWM output and set mode to phase/frequency corrected PWM
    TCCR0B = (1 << CS01) | (1 << CS00);   //Set clock source as /64 prescaler from main clock
}




//this is my 5mS rollover ISR
ISR(TIMER1_COMPA_vect){
    static u8 PWM_interrupts = 0; //when ramping, this is how many 5mS rollovers I will go through before changing OCR0B
    
    static u8 press_arm = 0;    //part of my debouncer, will only arm the detection of a falling edge after it's been consistently hi for X cycles
    static u8 last_state = 1;   //what state was the button in last time
    static u16 cycles_in_state = 0;  //how many ISR's have I been through without changing status
    
    volatile u8 current_state = button_read();
    if(press_arm && !current_state){
        freshPress = 1;
        press_arm = 0;
    }
    if(current_state == last_state) {
        cycles_in_state ++;
    }else{cycles_in_state = 0;}
    
    if((cycles_in_state > debounce_time) && current_state){
        press_arm = 1;  //if the button's been hi for greater than the debounce time, re-arm a falling edge
        longPressArm = 1;   //longPresArm has a separate reset since 'press_arm' will be cleared when the button is pressed, but long press_arm is only concerned with teh button staying low for 4 seconds.
    }
    
    last_state = current_state; //get ready for next loop
    
    PWM_interrupts ++;
    
    if ((PWM_interrupts >= PWM_Interval) && (OCR0B != PWMTarget)){
        if(OCR0B < PWMTarget){
            OCR0B ++;
        }else {(OCR0B --);}
        
        PWM_interrupts = 0; //reset this
    }
    

    
    if(freshPress){
        if(current_state){  //if freshpress is hi but the button has been released
            quickPress = 1;
            freshPress = 0;
        } else if (cycles_in_state > 200){  //if the button's been low for >1second
            freshPress = 0;
            if(state != conston){
            state = no_timer;
            PWMTarget = max_PWM;
            OCR0B = max_PWM;
            }   //if a >1-second press has happened and we're not in conston mode, go to timerless mode
        
        }
    
    }//end of if freshPress
    
    if(state == running){
        onTimeLeft --;
        if (onTimeLeft == 0){
            set_PWMVal(0);
            state = waiting;
        }
        
    }   //end of if we're running
    
    
    //if the button has been low for > 4 seconds
    if((!current_state)&& (cycles_in_state > 800) && longPressArm){
        longPressArm = 0;   //will be re-armed at debounce check
        quickPress = 0; //clear this in case it was set while the light was on
        changeControlMode();
       
    }   //end of what to do if 'longPressArm' is hi and the button's been low for >4 seconds.  changeControlMode will toggle between the button/PIR control method and just keeping this on all the time.
    
    

}   //end of 5mS rollover ISR.








u8 checkquickpress(void){
    if(!quickPress){
        return 0;}else{
            quickPress = 0;
            return 1;
        }
        }


u8 check4press(void) {
    if(freshPress){
        freshPress = 0;
        return 1;
    }else {return 0;}
    
}


u8 button_read(void){
    if(PINB & 0x08){
        return 1;
    }else {return 0;}
}

void set_PWMVal(u8 targetval){
    PWMTarget = targetval;
}


void resetTimer(void){
    onTimeLeft = 60000; //60k/5mS = 5 minutes
}




