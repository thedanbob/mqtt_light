#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include "config.h"
#include "helper.h"

class MQTTClient {
  public:
    MQTTClient();
    bool connect(String uid);
    void disconnect();
    bool connected();
    void loop();
    void sendState(size_t ch, bool state);
    void sendSys();
    void sendDiscovery(size_t ch, String uid);
    void setCommandCallback(channel_callback_t cb);

  private:
    static const String _names[];
    static const String _baseTopic;
    static const String _sysTopic;
    static const String _upTopic;

    WiFiClient _wifiClient;
    PubSubClient _pubSubClient;
    channel_callback_t _cmdCallback;

    String _commandTopic(size_t ch, bool includeBase = true);
    String _stateTopic(size_t ch, bool includeBase = true);
};

#endif
