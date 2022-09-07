# Sonoff Light (Home Assistant)

This firmware turns a Sonoff Basic/4CH into an MQTT light in Home Assistant. [platformio.sample.ini](platformio.sample.ini) shows how to configure the project for multiple devices. All options and their defaults can be found in [config.h](include/config.h). No configuration on the Home Assistant side is necessary besides setting up the MQTT integration; all device-specific settings are shared via [MQTT discovery](https://www.home-assistant.io/docs/mqtt/discovery).

The [upload_ota.sh](upload_ota.sh) script allows you to update a device over-the-air after initial flashing. E.g. for a device running the environment `single_light` (configured in platformio.ini) with an IP address of 192.168.1.3:

```bash
./upload_ota.sh single_light 192.168.1.3
```
