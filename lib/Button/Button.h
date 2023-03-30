#ifndef BUTTON_H
#define BUTTON_H

#include <Ticker.h>
#include "config.h"
#include "Circuit.h"

class Button {
  public:
    explicit Button(Circuit* c);
    void begin(bool& updateMode);

  private:
    static const size_t _pin[CHANNELS];

    Circuit* _circuitPtr;
    boolean _pressed[CHANNELS];
    Ticker _timer[CHANNELS];
};

#endif
