#ifndef __LED_H__
#define __LED_H__

#include <pico/stdlib.h>

bool getOnBoardLedState();

void onboardLED(bool state) ;

void initOnboardLED() ;



#endif