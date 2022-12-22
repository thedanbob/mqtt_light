#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Ticker.h>
#include "Circuit.h"

class Watchdog {
  public:
    Watchdog(Circuit *circuit);
    void start();
    void stop();

  private:
    Ticker _timer;
    Circuit *_circuit;
};

#endif
