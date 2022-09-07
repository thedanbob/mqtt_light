#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "config.h"
#include "Relay.h"
#include "MqttClient.h"

Relay relay;
MQTTClient mqtt;

Ticker sysUpdate;
bool updateInProgress{false};
char uid[16];

#if ENABLE_LED
  Ticker blinkTimer;

  void startBlinking() {
    if (blinkTimer.active()) return;

    blinkTimer.attach_ms(300, []() {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    });
  }

  void stopBlinking() {
    blinkTimer.detach();
    digitalWrite(LED_PIN, LOW);
  }
#endif

void setup() {
  #if ENABLE_LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LED is inverted, LOW = on
  #endif

  sprintf(uid, "sonoff_%06X", ESP.getChipId());

  LOG_BEGIN(115200);
  LOG_F("\n\nUnit ID: %s\n", uid);

  // Setup buttons & relays, restore state
  // Puts device into update mode (no MQTT) if first button is held
  relay.init(updateInProgress);

  LOG_F("Connecting to wifi %s: ", WIFI_SSID);
  #if ENABLE_LED
    startBlinking(); // Blink LED until wifi && mqtt connected
  #endif
  WiFi.mode(WIFI_STA);
  WiFi.hostname(uid);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    LOG(".");
  }

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
      #if ENABLE_LED
        digitalWrite(LED_PIN, (progress / (total / 20)) % 2); // Toggle LED every 5%
      #endif
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
      relay.set(ch, state);
    });

    // Update system info every 10 seconds
    sysUpdate.attach(10, []() {
      mqtt.sendSys();
    });
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    #if ENABLE_LED
      startBlinking();
    #endif

    if (WiFi.waitForConnectResult() != WL_CONNECTED) return;
  }

  ArduinoOTA.handle();
  if (updateInProgress) return; // Disable mqtt handling during update

  // (Re)connect to mqtt
  if (!mqtt.connected()) {
    if (!mqtt.connect(uid)) {
      #if ENABLE_LED
        startBlinking();
      #endif
      delay(100);
      return;
    }

    #if ENABLE_LED
      stopBlinking();
    #endif

    // Send all info on (re)connection
    mqtt.sendSys();
    for (size_t ch{0}; ch < CHANNELS; ch++) {
      mqtt.sendDiscovery(ch, uid);
      mqtt.sendState(ch, relay.get(ch));
    }
  }

  mqtt.loop();

  for (size_t ch{0}; ch < CHANNELS; ch++) {
    if (relay.hasChanged(ch)) {
      mqtt.sendState(ch, relay.get(ch));
    }
  }
}
