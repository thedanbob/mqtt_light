#include "LED.h"
#include "config.h"
#include "helper.h"

#ifndef DISABLE_LINK_LED

void initLED() {
  pinMode(LINK_LED, OUTPUT);
  setLED(HIGH);
}

void setLED(int state) {
  invertedWrite(LINK_LED, state);
}

void startBlinking(Ticker &timer) {
  if (timer.active()) return;

  timer.attach_ms(300, []() {
    digitalWrite(LINK_LED, !digitalRead(LINK_LED));
  });
}

void stopBlinking(Ticker &timer) {
  if (!timer.active()) return;

  timer.detach();
  invertedWrite(LINK_LED, HIGH);
}

#else

void initLED() {}
void setLED(int state) {}
void startBlinking(Ticker &timer) {}
void stopBlinking(Ticker &timer) {}

#endif
