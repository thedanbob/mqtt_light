#include "Circuit.h"
#include <EEPROM.h>

const size_t Circuit::_relayPin[]{RELAYS};
const bool Circuit::_restoreState[]{RESTORE_STATES};

Circuit::Circuit() :
  _currentState{SLICE(false, false, false, false)},
  _lastState{SLICE(false, false, false, false)}
{}

void Circuit::begin() {
  #ifdef STATE
    pinMode(STATE, INPUT);
  #endif

  EEPROM.begin(CHANNELS);

  for (size_t ch{0}; ch < CHANNELS; ch++) {
    pinMode(_relayPin[ch], OUTPUT);
    set(ch, _restoreState[ch] ? EEPROM.read(ch) : LOW);
    _lastState[ch] = get(ch);
  }
}

void Circuit::setChangeCallback(state_callback_t cb) {
  _changeCallback = cb;
}

bool Circuit::get(size_t ch) {
  #ifndef THREEWAY
    return digitalRead(_relayPin[ch]);
  #elif defined(REVERSE_STATE)
    return invertedRead(STATE);
  #else
    return digitalRead(STATE);
  #endif
}

void Circuit::set(size_t ch, bool state) {
  #ifndef THREEWAY
    digitalWrite(_relayPin[ch], state);
  #else
    if (get(ch) != state) toggle(ch);
  #endif
}

void Circuit::toggle(size_t ch) {
  toggleOutput(_relayPin[ch]);
}

void Circuit::check() {
  for (size_t ch{0}; ch < CHANNELS; ch++) {
    _currentState[ch] = get(ch);
  }
}

void Circuit::processChanges(bool runCallback) {
  for (size_t ch{0}; ch < CHANNELS; ch++) {
    if (_lastState[ch] == _currentState[ch]) continue;

    _lastState[ch] = _currentState[ch];
    if (_restoreState[ch]) {
      EEPROM.write(ch, _currentState[ch]);
      EEPROM.commit();
    }

    if (runCallback && _changeCallback != nullptr) {
      _changeCallback(ch, _currentState[ch]);
    }
  }
}
