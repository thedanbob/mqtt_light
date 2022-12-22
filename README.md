# Sonoff/Gosund/Shelly MQTT Light for Home Assistant

This firmware turns a Sonoff Basic, Sonoff 4CH, Gosund SW1, Gosund SW6, or Shelly 1 into an MQTT light in Home Assistant. [platformio.sample.ini](platformio.sample.ini) shows how to configure the project for multiple devices. All options and their defaults can be found in [config.h](include/config.h). No configuration on the Home Assistant side is necessary besides setting up the MQTT integration; all device-specific settings are shared via [MQTT discovery](https://www.home-assistant.io/docs/mqtt/discovery).

Add `upload_protocol = espota` to platformio.ini after flashing the firmware once to enable over-the-air updates. To update OTA a device running the `single_light` environment with an IP address of 192.168.1.3, you would run

```bash
pio run -e single_light -t upload --upload-port 192.168.1.3
```

## Gosund SW6

The Gosund SW6 is a smart 3-way light switch, but it can be used in 2-way mode (same as the SW1). Simply connect the load to Traveler 2 and omit the `THREEWAY` define.
