#include "Watchdog.h"
#include "config.h"

Watchdog::Watchdog(Circuit *circuit) :
  _timer{Ticker{}}
{
  _circuit = circuit;
}

void Watchdog::start() {
  if (_timer.active()) return;

  _timer.once(WIFI_TIMEOUT*60, [this]() {
    _circuit->processChanges(false); // Save current state before reboot (if configured)
    ESP.restart();
  });
}

void Watchdog::stop() {
  _timer.detach();
}
