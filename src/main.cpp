#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "config.h"
#include "MqttClient.h"
#include "LED.h"
#include "Circuit.h"
#include "Button.h"
#include "Watchdog.h"

MQTTClient mqtt;
LED linkLED;
Circuit circuit;
Button button(&circuit);
Watchdog watchdog(&circuit);

Ticker stateUpdate;
Ticker sysUpdate;
bool updateInProgress{false};
char uid[sizeof(UID_PREFIX) + 9]; // 32-bit Chip ID is up to 8 hex characters so "_%06X" -> 9

void setup() {
  sprintf(uid, UID_PREFIX "_%06X", ESP.getChipId());

  LOG_BEGIN(115200);
  LOG_F("\n\nUnit ID: %s\n", uid);

  linkLED.begin();
  circuit.begin();
  button.begin(updateInProgress);

  LOG_F("Connecting to wifi %s: ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(uid);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  linkLED.blink();
  watchdog.start();
  while (WiFi.waitForConnectResult(1000) != WL_CONNECTED) LOG(".");

  watchdog.stop();
  LOG("done\nIP Address is "); LOG_LN(WiFi.localIP());

  ArduinoOTA.setHostname(uid);

  ArduinoOTA.onStart([]() {
    LOG_LN("OTA update initiated...");
    mqtt.disconnect();
    updateInProgress = true;
  });

  ArduinoOTA.onEnd([]() {
    LOG_LN("\nOTA update done, restarting...");
  });

  ArduinoOTA.onProgress([](auto progress, auto total) {
    LOG_F("Progress: %u%%\r", (progress / (total / 100)));
    linkLED.set((progress / (total / 20)) % 2); // Toggle LED every 5%
  });

  ArduinoOTA.onError([](auto error) {
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

    ESP.restart();
  });

  ArduinoOTA.begin(false);
  if (updateInProgress) return;

  // Set callback to run when mqtt command received
  mqtt.setCommandCallback([](auto ch, auto state) {
    circuit.set(ch, state);
  });

  // Set callback to run when circuit state changes
  circuit.setChangeCallback([](auto ch, auto state) {
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

void loop() {
  if (!WiFi.isConnected()) {
    linkLED.blink();
    watchdog.start();

    if (WiFi.waitForConnectResult() != WL_CONNECTED) return;
  }

  watchdog.stop();

  ArduinoOTA.handle();
  if (updateInProgress) return; // Disable mqtt handling during update

  // (Re)connect to mqtt
  if (!mqtt.connected()) {
    linkLED.blink();

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

  linkLED.reset();
  mqtt.loop();
  circuit.processChanges();
}
