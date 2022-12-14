#ifndef RELAY_H
#define RELAY_H

#include <Ticker.h>
#include "config.h"

class Relay {
  public:
    Relay();
    void init(bool &updateMode);
    bool get(size_t ch);
    void set(size_t ch, bool state);
    bool hasChanged(size_t ch);

  private:
    static const size_t _buttonPin[CHANNELS];
    static const size_t _relayPin[CHANNELS];
    static const bool _restoreState[CHANNELS];

    bool _lastState[CHANNELS]; // Last state reported
    unsigned long _btnCount[CHANNELS];
    Ticker _btnTimer[CHANNELS];
};

#endif
