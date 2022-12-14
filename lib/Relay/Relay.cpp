#include <EEPROM.h>
#include "Relay.h"

const size_t Relay::_buttonPin[]{BUTTONS};
const size_t Relay::_relayPin[]{RELAYS};
const bool Relay::_restoreState[]{RESTORE_STATES};

Relay::Relay() :
  _lastState{SLICE(false, false, false, false)},
  _btnCount{SLICE(0, 0, 0, 0)},
  _btnTimer{SLICE(Ticker{}, Ticker{}, Ticker{}, Ticker{})}
{}

void Relay::init(bool &updateMode) {
  EEPROM.begin(CHANNELS);

  for (size_t ch{0}; ch < CHANNELS; ch++) {
    // Initialize relays
    pinMode(_relayPin[ch], OUTPUT);
    set(ch, _restoreState[ch] ? EEPROM.read(ch) : LOW, false);
    _lastState[ch] = get(ch);

    // Initialize buttons
    pinMode(_buttonPin[ch], INPUT);

    // Put device in update mode if first button is held while device starts
    if (ch == 0 && !digitalRead(_buttonPin[ch])) {
      updateMode = true;
      return;
    }

    _btnTimer[ch].attach_ms(50, [this, ch]() {
      if (!digitalRead(_buttonPin[ch])) { // LOW = pressed
        _btnCount[ch]++;
      } else {
        if (_btnCount[ch] > 1) {
          set(ch, !get(ch)); // Toggle relay
          _btnCount[ch] = 0;
        }
      }
    });
  }
}

bool Relay::get(size_t ch) {
  return digitalRead(_relayPin[ch]);
}

void Relay::set(size_t ch, bool on, bool write) {
  digitalWrite(_relayPin[ch], on);

  if (_restoreState[ch] && write) {
    EEPROM.write(ch, on);
    EEPROM.commit();
  }
}

bool Relay::hasChanged(size_t ch) {
  bool newState = get(ch);
  if (_lastState[ch] != newState) {
    _lastState[ch] = newState;
    return true;
  }
  return false;
}
