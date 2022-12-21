#include "Button.h"

const size_t Button::_pin[]{BUTTONS};

Button::Button(Circuit *circuit) :
  _count{SLICE(0, 0, 0, 0)},
  _timer{SLICE(Ticker{}, Ticker{}, Ticker{}, Ticker{})}
{
  _circuit = circuit;
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
        invertedWrite(BTN_LED, _circuit->get(ch));
      #endif

      // Toggle relay after momentary button press
      if (invertedRead(_pin[ch])) {
        _count[ch]++;
      } else {
        if (_count[ch] > 1) {
          _circuit->toggle(ch);
          _count[ch] = 0;
        }
      }
    });
  }
}
