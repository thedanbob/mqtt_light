#include "Circuit.h"
#include "helper.h"
#include <EEPROM.h>

const size_t Circuit::_buttonPin[]{BUTTONS};
const size_t Circuit::_relayPin[]{RELAYS};
const bool Circuit::_restoreState[]{RESTORE_STATES};

Circuit::Circuit() :
  _lastState{SLICE(false, false, false, false)},
  _btnCount{SLICE(0, 0, 0, 0)},
  _btnTimer{SLICE(Ticker{}, Ticker{}, Ticker{}, Ticker{})}
{}

void Circuit::init(bool &updateMode) {
  #ifdef GOSUND
    pinMode(STATE, INPUT);
    pinMode(BTN_LED, OUTPUT);
  #endif

  EEPROM.begin(CHANNELS);

  for (size_t ch{0}; ch < CHANNELS; ch++) {
    // Initialize button
    pinMode(_buttonPin[ch], INPUT);

    // Put device in update mode if first button is held while device starts
    if (ch == 0 && invertedRead(_buttonPin[ch])) {
      updateMode = true;
      return;
    }

    // Initialize relay
    pinMode(_relayPin[ch], OUTPUT);
    set(ch, _restoreState[ch] ? EEPROM.read(ch) : LOW);
    _lastState[ch] = get(ch);

    _btnTimer[ch].attach_ms(50, [this, ch]() {
      #ifdef GOSUND
        // Keep LED in sync with circuit state
        invertedWrite(BTN_LED, get(ch));
      #endif

      // Toggle relay after momentary button press
      if (invertedRead(_buttonPin[ch])) {
        _btnCount[ch]++;
      } else {
        if (_btnCount[ch] > 1) {
          toggleOutput(_relayPin[ch]);
          _btnCount[ch] = 0;
        }
      }
    });
  }
}

bool Circuit::get(size_t ch) {
  #ifndef GOSUND // 2-way switch
    return digitalRead(_relayPin[ch]);
  #elif defined(REVERSE_STATE)
    return invertedRead(STATE);
  #else
    return digitalRead(STATE);
  #endif
}

void Circuit::set(size_t ch, bool state) {
  // Less efficient than digitalWrite but works for both 2- and 3-way switches
  if (get(ch) != state) {
    toggleOutput(_relayPin[ch]);
  }
}

bool Circuit::hasChanged(size_t ch) {
  bool newState{get(ch)};
  if (_lastState[ch] == newState) return false;

  _lastState[ch] = newState;
  if (_restoreState[ch]) {
    EEPROM.write(ch, newState);
    EEPROM.commit();
  }

  return true;
}
