#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "config.h"
#include "Circuit.h"
#include "MqttClient.h"
#include "LED.h"

Circuit circuit;
MQTTClient mqtt;

Ticker sysUpdate;
Ticker blinkTimer;
Ticker watchdog;
bool updateInProgress{false};
char uid[16];

void reboot() {
  for (size_t ch{0}; ch < CHANNELS; ch++) {
    circuit.hasChanged(ch); // Save current state before reboot (if configured)
  }
  ESP.restart();
}

void setup() {
  #ifdef GOSUND
    sprintf(uid, "gosund_%06X", ESP.getChipId());
  #else
    sprintf(uid, "sonoff_%06X", ESP.getChipId());
  #endif

  LOG_BEGIN(115200);
  LOG_F("\n\nUnit ID: %s\n", uid);

  initLED();

  // Setup buttons & relays, restore state
  // Puts device into update mode (no MQTT) if first button is held
  circuit.init(updateInProgress);

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

  #if defined(DEBUG) || ENABLE_LED
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

    stopBlinking(blinkTimer);

    // Send all info on (re)connection
    mqtt.sendSys();
    for (size_t ch{0}; ch < CHANNELS; ch++) {
      mqtt.sendDiscovery(ch, uid);
      mqtt.sendState(ch, circuit.get(ch));
    }
  }

  mqtt.loop();

  for (size_t ch{0}; ch < CHANNELS; ch++) {
    if (circuit.hasChanged(ch)) {
      mqtt.sendState(ch, circuit.get(ch));
    }
  }
}
