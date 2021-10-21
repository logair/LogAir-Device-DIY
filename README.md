# Hardware docs for the DIY LogAir device

- Schematics
- PCB
- BoM
- Arduino code

The hardware as it is streams, once per second, the following data through BLE and to the uSD card:
Latitude, Longitude, Altitude, Speed, Heading, Temperature, Relative Humidity, Atmospheric Pressure, PM1, PM2.5, PM4, PM10

The Arduino-compatible hardware is based on a STM32 MCU. The chip is flashed with a code compiled with the Arduino IDE and the following libraries: NeoGPS, PMS Library, BME280, SdFat.

The data transmissio nprotocol is extensible to allow implementation of different sensors.

# ToDo

## Hardware

- Better power supply, as this one just charges and discharges blindly.
- Integrate the components
- Replace the BLE module with an ESP32 (the STM32 might have to stay, as ESP32 are more limited, e.g., available UARTs)
- Make a similar MVP with SPS30 & SHT3x
- Add a LoRa chip! See Clement's work
- ...

## Firmware

- Implement better data storage and upload 
  - Revamp buffering to limit uSD write operations
  - Revamp data storage on the SD card (textfile + DB?)
  - Implement deferred data upload for when uplink not continuously available
- Implement online GPS config: some changes to the embedded GPS FW are sometimes necessary for best operation
  - Get and store almanach/ephemeris on uSD card! This would really help with no-phone collection
  - Other generic config, constellations, accuracy requirements, etc.
- Implement component recognition: find out which sensors are used from their output stream (starting with PM sensor)
- Add interrupts everywhere?
- ...

## APP

- All 
  - Implement API key for upload/download

- Android version
  - APP should check at startup that GPS is enabled (and available in background if knowable) and prompt if not. 
  - Make it simpler: no settings, sending all packets stored every 10sec, etc
  - Auto-connect: priority to previously connected devices, then to '.*logair.*', etc
  - Map should be loaded at startup, not only when map tab is opened
  - ...

- iOS version
  - Make one :x Nicos is on it!
  - ...

## Backend

  - Implement API key for upload/download
  - ...

# Upcoming

- Arduino library configs
- ToDo list updates
- Assembly instructions (video?)


Licensed under [CERN OHL v.1.2 or later](https://ohwr.org/project/cernohl/wikis/home)
