//
/*
 Will read the Analog to Digital Converter on PB4/ADC 2 to know the state of the PIR output.  VCC is the reference
*/
 //

#include "ADC.h"

#define tolerance 150   //this is how far above or below my center point of 512 (1024/2) my ADC can be before I'll say something's going on here.
#define upperThresh (512 + tolerance)
#define lowerThresh (512 - tolerance)


//initializing the ADC is simply setting mux channel and enabling the ADC
void initADC(void){
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);   //turn on ADC and give it a /64 prescaler which will give me a 125K ADC clock.
    ADMUX = (1 << MUX1);    //select ADC2.
}




//right now just going to return a flag as opposed to the ADC value.  If I'm beyond the upper or lower thresholds then retun a 1, otherwise returna zero
u8 checkADC (void){

    u16 adcgrab;
    ADCSRA |= 0x10;  //clear the flag before starting conversion
    ADCSRA |= (1 << ADSC);  //start conversion
    while (!(ADCSRA & 0x10)){} //wait for conversion to finish...it looks like it's ~109uS to convert and I'm not in a hurry, so why not hang out

    
    adcgrab = ADC;
    if((adcgrab > upperThresh)|| (adcgrab < lowerThresh)){
        return 1;
    } else {
       return 0;
}
}   //end of check ADC function
