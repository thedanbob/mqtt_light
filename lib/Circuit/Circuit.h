#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <Ticker.h>
#include "config.h"
#include "helper.h"

class Circuit {
  public:
    Circuit();
    void init(bool &updateMode);
    void setChangeCallback(channel_callback_t cb);
    bool get(size_t ch);
    void set(size_t ch, bool state);
    void check();
    void processChanges(bool runCallback = true);

  private:
    static const size_t _buttonPin[CHANNELS];
    static const size_t _relayPin[CHANNELS];
    static const bool _restoreState[CHANNELS];

    bool _currentState[CHANNELS];
    bool _lastState[CHANNELS]; // Last state reported
    unsigned long _btnCount[CHANNELS];
    Ticker _btnTimer[CHANNELS];
    channel_callback_t _changeCallback;
};

#endif
