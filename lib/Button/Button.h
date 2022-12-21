#ifndef BUTTON_H
#define BUTTON_H

#include <Ticker.h>
#include "config.h"
#include "helper.h"
#include "Circuit.h"

class Button {
  public:
    Button(Circuit *circuit);
    void begin(bool &updateMode);

  private:
    static const size_t _pin[CHANNELS];

    Circuit *_circuit;
    unsigned long _count[CHANNELS];
    Ticker _timer[CHANNELS];
};

#endif
