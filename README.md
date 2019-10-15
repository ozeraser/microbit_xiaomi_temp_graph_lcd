# microbit_xiaomi_temp_graph_lcd

![Microbit with lcd showing temp](http://www.l8ter.com/microtemp.jpg)

The micro:bit finds Xiaomi Bluetooth Temperature Sensors, and graphs them to a Waveshare 1.8" LCD https://www.waveshare.com/wiki/1.8inch_LCD_for_micro:bit

![Xiaomi Bluetooth Temperature and Humidity Sensor](http://www.l8ter.com/xiaomitemp.jpg)

LCD uses my https://github.com/ozeraser/WSLCD1in8 fork of the LCD_Driver (files included in this project).


## How to Build
This project uses yotta to build, not pxt.
This project uses the another SoftDevice(S130). That enables BLE Central feature.

Follow these steps to build the project.
** Don't forget copying NRF51822_S130.ld to NRF51822.ld ! **

```bash
# set target to use S130 SoftDevice.
yt target bbc-microbit-classic-gcc-s130

# the linker uses `NRF51822.ld` file, then copy `NRF51822_S130.ld` to `NRF51822.ld`.
cp yotta_targets/bbc-microbit-classic-gcc-s130/ld/NRF51822_S130.ld yotta_targets/bbc-microbit-classic-gcc-s130/ld/NRF51822.ld

# build the project
yt build

# transfer the hex file to micro:bit. (for example, macOS X)
cp build/bbc-microbit-classic-gcc-s130/source/microbit-ble-bridge-combined.hex /Volumes/MICROBIT/microbit-ble-bridge-combined.hex
```