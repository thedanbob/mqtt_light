#ifndef LED_H
#define LED_H

#include <Ticker.h>

void initLED();
void setLED(int state);
void startBlinking(Ticker &timer);
void stopBlinking(Ticker &timer);

#endif
