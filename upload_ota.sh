#!/bin/bash
# ./upload_ota.sh NAME [IP ADDRESS]

set -e

tools_dir=~/.platformio/packages/framework-arduinoespressif8266/tools
build_dir=.pio/build/$1

if [ ! -f $build_dir/firmware.bin ]; then
  pio run -e $1
fi
gzip -9kf $build_dir/firmware.bin
# python3 $tools_dir/signing.py --mode sign --privatekey ./private.key --bin $build_dir/firmware.bin.gz --out $build_dir/firmware.bin.gz.signed

if [ -n "$2" ]; then
  hostip=$(ifconfig $(route | grep default | sed -E 's/.+[0-9]\s+[0-9]\s+[0-9]\s+(.+)$/\1/') | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*')
  python3 $tools_dir/espota.py -i $2 -I $hostip -f $build_dir/firmware.bin.gz
fi
