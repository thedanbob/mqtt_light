#include "Button.h"

#ifndef DISABLE_BUTTON

const size_t Button::_pin[]{BUTTONS};

Button::Button(Circuit *c) :
  _count{SLICE(0, 0, 0, 0)},
  _timer{SLICE(Ticker{}, Ticker{}, Ticker{}, Ticker{})}
{
  _circuitPtr = c;
}

void Button::begin(bool &updateMode) {
  #ifdef BTN_LED
    pinMode(BTN_LED, OUTPUT);
  #endif

  for (size_t ch{0}; ch < CHANNELS; ch++) {
    pinMode(_pin[ch], INPUT);

    // Put device in update mode if first button is held while device starts
    if (ch == 0 && invertedRead(_pin[ch])) {
      updateMode = true;
      return;
    }

    _timer[ch].attach_ms(50, [this, ch]() {
      #ifdef BTN_LED
        // Keep LED in sync with circuit state
        invertedWrite(BTN_LED, _circuitPtr->get(ch));
      #endif

      // Toggle relay after momentary button press
      if (invertedRead(_pin[ch])) {
        _count[ch]++;
      } else {
        if (_count[ch] > 1) {
          _circuitPtr->toggle(ch);
          _count[ch] = 0;
        }
      }
    });
  }
}

#else

Button::Button(Circuit *c) {}
void Button::begin(bool &updateMode) {}

#endif
