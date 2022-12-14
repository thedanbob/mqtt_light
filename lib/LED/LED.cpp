#include "LED.h"
#include "config.h"
#include "helper.h"

#if ENABLE_LED

void initLED() {
  pinMode(LED_PIN, OUTPUT);
  setLED(HIGH);
}

void setLED(int state) {
  invertedWrite(LED_PIN, state);
}

void startBlinking(Ticker &timer) {
  if (timer.active()) return;

  timer.attach_ms(300, []() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  });
}

void stopBlinking(Ticker &timer) {
  timer.detach();
  invertedWrite(LED_PIN, HIGH);
}

#else

void initLED() {}
void setLED(int state) {}
void startBlinking(Ticker &timer) {}
void stopBlinking(Ticker &timer) {}

#endif
