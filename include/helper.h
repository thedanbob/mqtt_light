#ifndef HELPER_H
#define HELPER_H

#include <Arduino.h>
#include <functional>

typedef std::function<void (size_t ch, bool state)> state_callback_t;

inline int invertedRead(uint8_t pin) { return !digitalRead(pin); }
inline void invertedWrite(uint8_t pin, uint8_t val) { digitalWrite(pin, !val); }
inline void toggleOutput(uint8_t pin) { digitalWrite(pin, !digitalRead(pin)); }

#endif
