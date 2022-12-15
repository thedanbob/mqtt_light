#include <ESP8266WiFi.h>
#include "MqttClient.h"

const String MQTTClient::_names[]{NAMES};
const String MQTTClient::_baseTopic{MQTT_BASE_TOPIC};
const String MQTTClient::_sysTopic{MQTT_BASE_TOPIC MQTT_ATTR_SUF};
const String MQTTClient::_upTopic{MQTT_BASE_TOPIC MQTT_AVTY_SUF};

MQTTClient::MQTTClient() :
  _wifiClient{},
  _pubSubClient{_wifiClient, MQTT_SERVER, MQTT_PORT},
  _cmdCallback{nullptr}
{
  _pubSubClient.set_callback([this](const MQTT::Publish& pub) {
    String cmd{pub.payload_string()};
    if (cmd == MQTT_MSG_RESTART) ESP.restart();
    if (_cmdCallback == nullptr || (cmd != MQTT_MSG_ON && cmd != MQTT_MSG_OFF)) return;

    size_t ch{0};
    #if CHANNELS > 1
      String topic{pub.topic()};

      // Match topic to correct channel
      while(ch < CHANNELS) {
        if (topic == _commandTopic(ch)) break;
        ch++;
      }
      if (ch == CHANNELS) return; // Failed to match topic (should never happen)
    #endif

    _cmdCallback(ch, cmd == MQTT_MSG_ON);
  });
}

bool MQTTClient::connect(String uid) {
  LOG_F("Connecting to MQTT broker %s... ", MQTT_SERVER);
  MQTT::Connect conn{uid};
  conn.set_auth(MQTT_USER, MQTT_PASS)
      .set_will(_upTopic, MQTT_MSG_DOWN, MQTT_QOS, MQTT_RETAIN);

  if (!_pubSubClient.connect(conn)) {
    LOG_LN("FAILED!");
    return false;
  }

  MQTT::Subscribe subs;

  for (size_t ch{0}; ch < CHANNELS; ch++) {
    String topic{_commandTopic(ch)};
    LOG_F("Subscribing to topic %s\n", topic.c_str());
    subs.add_topic(topic, MQTT_QOS);
  }

  _pubSubClient.subscribe(subs);
  _pubSubClient.publish(
    MQTT::Publish{_upTopic, MQTT_MSG_UP}
      .set_retain(MQTT_RETAIN)
      .set_qos(MQTT_QOS)
  );
  return true;
}

void MQTTClient::disconnect() {
  _pubSubClient.publish(
    MQTT::Publish{_upTopic, MQTT_MSG_DOWN}
      .set_retain(MQTT_RETAIN)
      .set_qos(MQTT_QOS)
  );
  _pubSubClient.disconnect();
}

bool MQTTClient::connected() {
  return _pubSubClient.connected();
}

void MQTTClient::loop() {
  _pubSubClient.loop();
}

void MQTTClient::sendState(size_t ch, bool state) {
  String payload{state ? MQTT_MSG_ON : MQTT_MSG_OFF};
  _pubSubClient.publish(
    MQTT::Publish{_stateTopic(ch), payload}
      .set_retain(MQTT_RETAIN)
      .set_qos(MQTT_QOS)
  );
  LOG_F("Channel %d %s\n", ch + 1, payload.c_str());
}

void MQTTClient::sendSys() {
  String payload{"{"
    "\"IP\":\"" + WiFi.localIP().toString() + "\","
    "\"RSSI\":" + String{WiFi.RSSI()} + ","
    "\"FreeMem\":" + String{ESP.getFreeHeap()} +
  "}"};
  _pubSubClient.publish(_sysTopic, payload);
}

void MQTTClient::sendDiscovery(size_t ch, String uid) {
  uid += "_" + String{ch + 1};
  String topic{"homeassistant/light/" + uid + "/config"};
  String payload{"{"
    "\"name\":\"" + _names[ch] + "\","
    "\"uniq_id\":\"" + uid + "\","
    "\"~\":\"" + _baseTopic + "\","
    "\"cmd_t\":\"~" + _commandTopic(ch, false) + "\","
    "\"stat_t\":\"~" + _stateTopic(ch, false) + "\","
    "\"json_attr_t\":\"~" MQTT_ATTR_SUF "\","
    "\"avty_t\":\"~" MQTT_AVTY_SUF "\","
    #ifndef MQTT_MSG_ON_DEFAULT
    "\"pl_on\":\"" MQTT_MSG_ON "\","
    #endif
    #ifndef MQTT_MSG_OFF_DEFAULT
    "\"pl_off\":\"" MQTT_MSG_OFF "\","
    #endif
    #ifndef MQTT_MSG_UP_DEFAULT
    "\"pl_avail\":\"" MQTT_MSG_UP "\","
    #endif
    #ifndef MQTT_MSG_DOWN_DEFAULT
    "\"pl_not_avail\":\"" MQTT_MSG_DOWN "\","
    #endif
    "\"qos\":" STR(MQTT_QOS) ","
    "\"dev\":{"
      "\"name\":\"" DEVICE_NAME "\","
      "\"mf\":\"ITEAD\","
      "\"mdl\":\"" DEVICE_MODEL "\","
      "\"ids\":[\"" + ESP.getChipId() + "\"],"
      "\"cns\":[[\"mac\",\"" + WiFi.macAddress() + "\"]],"
      "\"sw\":\"" VERSION "\""
    "}"
  "}"};

  _pubSubClient.publish(
    MQTT::Publish{topic, payload}.set_retain()
  );
}

void MQTTClient::setCommandCallback(callback_t cb) {
  _cmdCallback = cb;
}

String MQTTClient::_commandTopic(size_t ch, bool includeBase) {
  return String{includeBase ? _baseTopic : ""} + "/" + String{ch + 1} + MQTT_CMD_SUF;
}

String MQTTClient::_stateTopic(size_t ch, bool includeBase) {
  return String{includeBase ? _baseTopic : ""} + "/" + String{ch + 1} + MQTT_STATE_SUF;
}
