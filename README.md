# hoermann_door
Control Hörmann doors drives directly via MQTT from Home Assistant

Inspired by: 
- https://blog.bouni.de/posts/2018/hoerrmann-uap1/
- https://github.com/steff393/hgdo
- https://github.com/avshrs/ESP32_Hormann_Supramatic_e3
- https://patents.google.com/patent/WO2005076529A1/de

**USE AT YOUR OWN RISK!**

# Overview

This repo contains the pcb files as well as the software to build an interface device which connects Hörmann door drives like the Supramatic E3 to Home Assistant. The interface device emulates an UAP1 to interact with the door drive. Additionally a BME280 can be connected via I2C to measure temperature, humidity and pressure.

**NOT compatible with drives from the 4th series**

## Alternative 1: prebuild PCB with pic16 MCU

<p align="center">
    <img src="docs/pcb1.png?raw=true">
    <img src="docs/pcb2.png?raw=true">
</p>

## Alternative 2: ESP32 with RS485 Transceiver

<p align="center">
    <img src="docs/PCB.png?raw=false" width="400px">
    <img src="docs/installation.png?raw=false" width="230px">
</p>


## MQTT Discvovery

The door drive is automatically added to Home Assistant via [MQTT Discovery](https://www.home-assistant.io/docs/mqtt/discovery/).

<p align="center">
    <img src="docs/device.png?raw=true">
</p>

## esphome
This repo also contains an esphome implementation that supports both above mentioned variants. For configuration examples see:
- [pic16: preflashed configuration](esphome/preflashed_pic16.yaml)
- [pic16: recommended configuration](esphome/recommended_pic16.yaml)

OR

- [esp and rs485 configuration](esphome/recommended_esp.yaml)


# Folder structure

* `board`: The Eagle schematic and board files
* `build`: already compiled binaries for pic16 board
* `docs`: Documentation
* `esp8266`: Arduino project. Communication to Home Assistant via wifi and mqtt
* `pic16`: MPLabX project. Communication with door drive via Hörmann bus
* `esphome`: implementation of the esphome components

# Thing to do first
## Alternative 1: prebuild PCB with pic16 MCU
1. Get a pcb
    * Optionally, get BME280 sensor pcb
    * Supply pcb with 24V via pins 2 & 3 of `J1`
    * Check if 5V and 3.3V are fine
1. Get tools listed below
1. Rename esp8266/config_template.h to esp8266/config.h and adjust to your environment
1. Add `esp8266` boards to Arduino IDE
    * Open Preferences window
    * Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
    * Open Boards Manager from Tools > Board menu and find `esp8266` platform.
    * Click install button.
    * Select `NodeMCU 1.0 (ESP-12E Module)` as board
1. Add Libraries to Arduino IDE
    * Open Sketch > Include library -> Manage libraries...
    * Install `PubSubClient` (tested with version 2.8.0)
    * Install `PubSubClientTools` (tested with version 0.6.0)
    * Install `Adafruit Unified Sensor` (tested with version 1.1.4)
    * Install `Adafruit BME280 Library` (tested with version 2.1.2)
1. First flashing of `esp8266`
    * Get a cheap USB-UART converter (like the ones with a `CP210x`)
    * Connect the converter to `SV3` (RX, TX and GND)
    * Short `JP1` (Boot mode)
    * Power up board. If board was powered before reset `esp8266` by shorting `JP2`(Reset) for a short time
1. Flashing `pic16`
    * Use any Microchip programmer (PICkit3, ICD3, ...)
    * Open MPLabX and click on File -> Open Project
    * Select `pic16` subfolder (chip icon)
    * Click Clean an Build Main Project
    * Click Make and Program Device Main Project

## Alternative 2: ESP32 with RS485 Transceiver
1. Build a PCB with an RS485 converter and figure out the correct pinout
2. adjust the config in esphome according to config files listed above
3. install initial firmware of esphome from [esphome web](https://web.esphome.io/)
4. Update the device from your own esphome instance over the air

# Used tools
## Alternative 1: prebuild PCB with pic16 MCU

### Schematic and board
* Eagle 7.2.0

### ESP8266
* Arduino IDE 1.8.13
* USB-UART converter

### PIC16
* MPLabX v5.25
* XC8 v2.10
* PICkit3

## Alternative 2: ESP32 with RS485 Transceiver

### Hardware
#### Working Hardware
- esp32 devkit 4
- DC/DC 24V -> 5V Step-Down
- https://www.ti.com/product/SN65HVD72 off a salvaged board

OR
- RS485-UART Coverter HW519
#### possible working hardware
The custom boards for the Series 4 drives should also work. *But the pinout is different!*

