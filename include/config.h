#ifndef SONOFF_CONFIG_H
#define SONOFF_CONFIG_H

// Device types
#define SONOFF 0
#define GOSUND 1

/*
  ====================================================================================================
                        Define constants below or, preferably, in platformio.ini
  ====================================================================================================
*/

//#define WIFI_SSID "wifi_ssid"           // Your WiFi ssid
//#define WIFI_PASS "wifi_password"       // Your WiFi password
//#define WIFI_TIMEOUT 10                 // Minutes to wait for a wifi connection before rebooting

//#define MQTT_SERVER "192.168.0.1"       // Your mqtt server ip address
//#define MQTT_PORT 1883                  // Your mqtt port
//#define MQTT_BASE_TOPIC "sonoff/basic"  // Base mqtt topic
//#define MQTT_USER "sonoff"              // mqtt username
//#define MQTT_PASS "password"            // mqtt password
//#define MQTT_RETAIN true                // Retain state/availability messages (recommended)
//#define MQTT_QOS 0                      // QOS level for mqtt messages (0 or 1)

//#define DEVICE SONOFF                   // SONOFF or GOSUND

// Friendly name for device discovery
//#define NAME "MQTT Light"

// Multi channel (Sonoff 4CH only)
//#define NAME_1 "Sonoff light 1"
//#define NAME_2 "Sonoff light 2"
//#define NAME_3 "Sonoff light 3"
//#define NAME_4 "Sonoff light 4"

// Restore relay states after power loss. If set to false, relay will be off when power is restored.
//#define RESTORE_STATE true

// Multi channel
//#define RESTORE_STATE_1 true
//#define RESTORE_STATE_2 true
//#define RESTORE_STATE_3 true
//#define RESTORE_STATE_4 true

//#define DISABLE_LINK_LED    // Enable the status LED
//#define THREEWAY            // Put Gosund SW6 in 3-way mode (2-way by default)
//#define REVERSE_STATE       // Reverse state pin (LOW = on) (Gosund SW6 only)


// Uncomment below to enable debug reporting
//#define DEBUG

/*
  ====================================================================================================
                                         END USER CONFIGURATION
  ====================================================================================================
*/

#include <Arduino.h>

// Defaults
#ifndef WIFI_SSID
  #define WIFI_SSID "wifi_ssid"
#endif
#ifndef WIFI_PASS
  #define WIFI_PASS "wifi_password"
#endif
#ifndef WIFI_TIMEOUT
  #define WIFI_TIMEOUT 10
#endif
#ifndef MQTT_SERVER
  #define MQTT_SERVER "192.168.0.1"
#endif
#ifndef MQTT_PORT
  #define MQTT_PORT 1883
#endif
#ifndef MQTT_BASE_TOPIC
  #define MQTT_BASE_TOPIC "sonoff/basic"
#endif
#ifndef MQTT_USER
  #define MQTT_USER "sonoff"
#endif
#ifndef MQTT_PASS
  #define MQTT_PASS "password"
#endif
#ifndef MQTT_RETAIN
  #define MQTT_RETAIN true
#endif
#ifndef MQTT_QOS
  #define MQTT_QOS 0
#endif

#ifndef DEVICE
  #define DEVICE SONOFF
#endif

#ifndef CHANNELS
  #if DEVICE == SONOFF
    #if defined(NAME_4)
      #define CHANNELS 4
    #elif defined(NAME_3)
      #define CHANNELS 3
    #elif defined(NAME_2)
      #define CHANNELS 2
    #else
      #define CHANNELS 1
    #endif
  #else
    #define CHANNELS 1
  #endif
#endif
#if CHANNELS > 4
  #define CHANNELS 4
#endif

#ifdef RESTORE_STATE
  #define RESTORE_STATE_1 RESTORE_STATE
#endif
#ifndef RESTORE_STATE_1
  #define RESTORE_STATE_1 true
#endif
#ifndef RESTORE_STATE_2
  #define RESTORE_STATE_2 true
#endif
#ifndef RESTORE_STATE_3
  #define RESTORE_STATE_3 true
#endif
#ifndef RESTORE_STATE_4
  #define RESTORE_STATE_4 true
#endif

#ifdef NAME
  #define NAME_1 NAME
#endif
#ifndef NAME_1
  #define NAME_1 ""
#endif
#ifndef NAME_2
  #define NAME_2 ""
#endif
#ifndef NAME_3
  #define NAME_3 ""
#endif
#ifndef NAME_4
  #define NAME_4 ""
#endif

// Debug helpers
#ifdef DEBUG
  #define LOG_BEGIN(x) Serial.begin(x)
  #define LOG(x) Serial.print(x)
  #define LOG_LN(x) Serial.println(x)
  #define LOG_F(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define LOG_BEGIN(x)
  #define LOG(x)
  #define LOG_LN(x)
  #define LOG_F(x, ...)
#endif

// String helpers (must be defined twice for reasons)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define CONCAT_HELPER(a, b) a##b
#define CONCAT(a, b) CONCAT_HELPER(a, b)

// Array initialization helpers
#define SLICE1(a, b, c, d) a
#define SLICE2(a, b, c, d) a, b
#define SLICE3(a, b, c, d) a, b, c
#define SLICE4(a, b, c, d) a, b, c, d
#define SLICE CONCAT(SLICE, CHANNELS)

// Other constants
#if DEVICE == SONOFF
  #define LINK_LED 13
  #define BUTTONS SLICE(0, 9, 10, 14)
  #define RELAYS SLICE(12, 5, 4, 15)
#elif DEVICE == GOSUND
  #ifndef DEVICE_MODEL
    #ifndef THREEWAY
      #define DEVICE_MODEL "Gosund SW1"
    #else
      #define DEVICE_MODEL "Gosund SW6"
    #endif
  #endif
  #define CHANNELS 1
  #define BTN_LED 2 // Circuit status LED
  #define LINK_LED 16 // Wifi & MQTT status LED
  #define BUTTONS 0
  #define RELAYS 14
  #define STATE 4 // 3-way switch, separate pin to sense circuit status
#endif

#define RESTORE_STATES SLICE(RESTORE_STATE_1, RESTORE_STATE_2, RESTORE_STATE_3, RESTORE_STATE_4)
#define NAMES SLICE(NAME_1, NAME_2, NAME_3, NAME_4)

#ifndef DEVICE_MODEL
  #if DEVICE == SONOFF
    #if CHANNELS == 1
      #define DEVICE_MODEL "Sonoff Basic"
    #else
      #define DEVICE_MODEL "Sonoff 4CH"
    #endif
  #elif DEVICE == GOSUND
    #ifndef THREEWAY
      #define DEVICE_MODEL "Gosund SW1"
    #else
      #define DEVICE_MODEL "Gosund SW6"
    #endif
  #endif
#endif

#ifndef DEVICE_NAME
  #define DEVICE_NAME DEVICE_MODEL
#endif

#define MQTT_CMD_SUF "/cmd"
#define MQTT_STATE_SUF "/stat"
#define MQTT_ATTR_SUF "/sys"
#define MQTT_AVTY_SUF "/up"

// Home Assistant defaults
#ifndef MQTT_MSG_ON
  #define MQTT_MSG_ON "ON"
  #define MQTT_MSG_ON_DEFAULT
#endif
#ifndef MQTT_MSG_OFF
  #define MQTT_MSG_OFF "OFF"
  #define MQTT_MSG_OFF_DEFAULT
#endif
#ifndef MQTT_MSG_UP
  #define MQTT_MSG_UP "online"
  #define MQTT_MSG_UP_DEFAULT
#endif
#ifndef MQTT_MSG_DOWN
  #define MQTT_MSG_DOWN "offline"
  #define MQTT_MSG_DOWN_DEFAULT
#endif
#ifndef MQTT_MSG_RESTART
  #define MQTT_MSG_RESTART "restart"
#endif

#ifndef VERSION
  #define VERSION "20220412000000"
#endif

#endif
