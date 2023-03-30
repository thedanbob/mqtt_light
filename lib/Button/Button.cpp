#include "Button.h"
#include "helper.h"

#ifndef DISABLE_BUTTON

const size_t Button::_pin[]{BUTTONS};

Button::Button(Circuit* c) :
  _pressed{SLICE(false, false, false, false)},
  _timer{SLICE(Ticker{}, Ticker{}, Ticker{}, Ticker{})}
{
  _circuitPtr = c;
}

void Button::begin(bool& updateMode) {
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
        if (!_pressed[ch]) {
          _circuitPtr->toggle(ch);
        }
        _pressed[ch] = true;
      } else {
        _pressed[ch] = false;
      }
    });
  }
}

#else

Button::Button(Circuit* c) {}
void Button::begin(bool& updateMode) {}

#endif
