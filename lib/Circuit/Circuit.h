#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <Ticker.h>
#include "config.h"
#include "helper.h"

class Circuit {
  public:
    Circuit();
    void begin();
    void setChangeCallback(state_callback_t cb);
    bool get(size_t ch);
    void set(size_t ch, bool state);
    void toggle(size_t ch);
    void check();
    void processChanges(bool runCallback = true);

  private:
    static const size_t _relayPin[CHANNELS];
    static const bool _restoreState[CHANNELS];

    bool _currentState[CHANNELS];
    bool _lastState[CHANNELS]; // Last state reported
    state_callback_t _changeCallback;
};

#endif
