#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Ticker.h>
#include "Circuit.h"

class Watchdog {
  public:
    explicit Watchdog(Circuit* c);
    void start();
    void stop();

  private:
    Ticker _timer;
    Circuit* _circuitPtr;
};

#endif
