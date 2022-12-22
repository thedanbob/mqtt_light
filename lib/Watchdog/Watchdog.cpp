#include "Watchdog.h"
#include "config.h"

Watchdog::Watchdog(Circuit *c) :
  _timer{Ticker{}}
{
  _circuitPtr = c;
}

void Watchdog::start() {
  if (_timer.active()) return;

  _timer.once(WIFI_TIMEOUT*60, [this]() {
    _circuitPtr->processChanges(false); // Save current state before reboot (if configured)
    ESP.restart();
  });
}

void Watchdog::stop() {
  _timer.detach();
}
