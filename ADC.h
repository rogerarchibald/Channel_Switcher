//
//  ADC.h
//  channel_Switcher
//
//  Created by Roger on 6/28/16.
//  Copyright © 2016 Roger. All rights reserved.
//

#ifndef ADC_h
#define ADC_h

#include <stdio.h>
#include "channelswitch_lib.h"


//function prototypes
void initADC(void);
u8 checkADC (void); //check, will return a flag depending on the value

#endif /* ADC_h */
