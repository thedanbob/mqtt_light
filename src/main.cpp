#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "config.h"
#include "Circuit.h"
#include "Button.h"
#include "MqttClient.h"
#include "LED.h"

LED linkLED;
Circuit circuit;
MQTTClient mqtt;
Button button(&circuit);

Ticker stateUpdate;
Ticker sysUpdate;
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

  linkLED.begin();
  circuit.begin();
  button.begin(updateInProgress);

  LOG_F("Connecting to wifi %s: ", WIFI_SSID);
  linkLED.blink();

  WiFi.mode(WIFI_STA);
  WiFi.hostname(uid);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) LOG(".");

  LOG_LN("done");
  LOG("IP Address is "); LOG_LN(WiFi.localIP());

  ArduinoOTA.setHostname(uid);

  ArduinoOTA.onStart([]() {
    LOG_LN("OTA update initiated...");
    mqtt.disconnect();
    updateInProgress = true;
  });

  ArduinoOTA.onEnd([]() {
    LOG_LN("\nOTA update done, restarting...");
  });

  ArduinoOTA.onProgress([](size_t progress, size_t total) {
    LOG_F("Progress: %u%%\r", (progress / (total / 100)));
    linkLED.set((progress / (total / 20)) % 2); // Toggle LED every 5%
  });

  ArduinoOTA.onError([](ota_error_t error) {
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

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (!watchdog.active()) {
      watchdog.once(WIFI_TIMEOUT*60, reboot);
    }
    linkLED.blink();
    if (WiFi.waitForConnectResult() != WL_CONNECTED) return;
  }

  watchdog.detach();

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
