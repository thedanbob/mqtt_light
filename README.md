# MQTT Light for Home Assistant

This firmware turns a **Sonoff Basic/4CH**, **Gosund SW1/SW5/SW6**, or **Shelly 1** into an [MQTT light](https://www.home-assistant.io/integrations/light.mqtt) in Home Assistant. [platformio.sample.ini](platformio.sample.ini) shows how to configure the project for multiple devices. All options and their defaults can be found in [config.h](include/config.h). No configuration on the Home Assistant side is necessary besides setting up the MQTT integration; all device-specific settings are shared via [MQTT discovery](https://www.home-assistant.io/docs/mqtt/discovery).

Add `upload_protocol = espota` to platformio.ini after flashing the firmware once to enable over-the-air updates. To update OTA a device running the `single_light` environment with an IP address of 192.168.1.3, you would run

```bash
pio run -e single_light -t upload --upload-port 192.168.1.3
```

You can also add `upload_port = 192.168.1.3` to platformio.ini instead of using the --upload-port flag.

## Gosund SW6

The Gosund SW6 is a smart 3-way light switch, but it can be used in 2-way mode (same as the SW1/SW5). Simply connect the load to Traveler 2 and omit the `THREEWAY` define.

## Remote reboot

You can reboot the devices via MQTT by sending a message (default: `restart`) manually to the device's command topic (default: `<base topic>/cmd`). Home Assistant provides a way to publish arbitrary messages in Settings > Devices & Services > (MQTT Integration) Configure.

## Other firmware

[MQTT Garage Door](https://github.com/thedanbob/mqtt_garage_door) \
[MQTT Power Cycler](https://github.com/thedanbob/mqtt_power_cycle)
