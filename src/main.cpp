#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "config.h"
#include "Circuit.h"
#include "Button.h"
#include "MqttClient.h"
#include "LED.h"

Circuit circuit;
MQTTClient mqtt;
#ifndef DISABLE_BUTTONS
  Button button(&circuit);
#endif

Ticker stateUpdate;
Ticker sysUpdate;
Ticker blinkTimer;
Ticker watchdog;
bool updateInProgress{false};
char uid[16];

void reboot() {
  circuit.processChanges(false); // Save current state before reboot (if configured)
  ESP.restart();
}

void setup() {
  #if DEVICE == SONOFF
    sprintf(uid, "sonoff_%06X", ESP.getChipId());
  #elif DEVICE == GOSUND
    sprintf(uid, "gosund_%06X", ESP.getChipId());
  #elif DEVICE == SHELLY
    sprintf(uid, "shelly_%06X", ESP.getChipId());
  #endif

  LOG_BEGIN(115200);
  LOG_F("\n\nUnit ID: %s\n", uid);

  initLED();

  // Setup relay(s), restore state
  circuit.begin();
  #ifndef DISABLE_BUTTONS
    // Setup button(s), put device into update mode (no MQTT) if first button is held
    button.begin(updateInProgress);
  #endif

  LOG_F("Connecting to wifi %s: ", WIFI_SSID);
  startBlinking(blinkTimer); // Blink LED until wifi && mqtt connected

  WiFi.mode(WIFI_STA);
  WiFi.hostname(uid);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) LOG(".");

  LOG_LN("done");
  LOG("IP Address is "); LOG_LN(WiFi.localIP());

  // Setup OTA updates
  ArduinoOTA.setHostname(uid);

  ArduinoOTA.onStart([]() {
    LOG_LN("OTA update initiated...");

    mqtt.disconnect();
    updateInProgress = true;
  });

  #ifdef DEBUG
    ArduinoOTA.onEnd([]() {
      LOG_LN("\nOTA update done, restarting...");
    });
  #endif

  #if defined(DEBUG) || !defined(DISABLE_LINK_LED)
    ArduinoOTA.onProgress([](size_t progress, size_t total) {
      LOG_F("Progress: %u%%\r", (progress / (total / 100)));
      setLED((progress / (total / 20)) % 2); // Toggle LED every 5%
    });
  #endif

  ArduinoOTA.onError([](ota_error_t error) {
    #ifdef DEBUG
      LOG_F("OTA error: [%u] ", error);
      switch(error) {
        case OTA_AUTH_ERROR:
          LOG_LN("Auth Failed"); break;
        case OTA_BEGIN_ERROR:
          LOG_LN("Begin Failed"); break;
        case OTA_CONNECT_ERROR:
          LOG_LN("Connect Failed"); break;
        case OTA_RECEIVE_ERROR:
          LOG_LN("Receive Failed"); break;
        case OTA_END_ERROR:
          LOG_LN("End Failed"); break;
      }
    #endif

    ESP.restart();
  });

  ArduinoOTA.begin(false);

  if (!updateInProgress) {
    // Set callback to run when mqtt command received
    mqtt.setCommandCallback([](size_t ch, bool state) {
      circuit.set(ch, state);
    });

    // Set callback to run when circuit state changes
    circuit.setChangeCallback([](size_t ch, bool state) {
      mqtt.sendState(ch, state);
    });

    // Check circuit state every quarter second
    stateUpdate.attach_ms(250, []() {
      circuit.check();
    });

    // Update system info every 10 seconds
    sysUpdate.attach(10, []() {
      mqtt.sendSys();
    });
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (!watchdog.active()) {
      watchdog.once(WIFI_TIMEOUT*60, reboot);
    }
    startBlinking(blinkTimer);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) return;
  }

  watchdog.detach();

  ArduinoOTA.handle();
  if (updateInProgress) return; // Disable mqtt handling during update

  // (Re)connect to mqtt
  if (!mqtt.connected()) {
    startBlinking(blinkTimer);

    if (!mqtt.connect(uid)) {
      delay(100);
      return;
    }

    // Send all info on (re)connection
    mqtt.sendSys();
    for (size_t ch{0}; ch < CHANNELS; ch++) {
      mqtt.sendDiscovery(ch, uid);
      mqtt.sendState(ch, circuit.get(ch));
    }
  }

  stopBlinking(blinkTimer);
  mqtt.loop();
  circuit.processChanges();
}
