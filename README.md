# Hardware docs for the DIY LogAir device

- Schematics
- PCB
- BoM
- Arduino code

The hardware as it is streams, once per second, the following data through BLE and to the uSD card:
Latitude, Longitude, Altitude, Speed, Heading, Temperature, Relative Humidity, Atmospheric Pressure, PM1, PM2.5, PM4, PM10

The Arduino-compatible MCU is a Bluepill, a generic implementation of the STM32F103. The chip is flashed with a code compiled with the Arduino IDE and the following libraries: NeoGPS, PMS Library, BME280, SdFat.

The data transmissio nprotocol is extensible to allow implementation of different sensors.

# How To

## Program

### Pre-requisites
- Install Arduino IDE
- Install board definition:
  - File > Preferences > Additional Boards Manager URLs: add the following URL to the list (empty by default):
  `http://dan.drown.org/stm32duino/package_STM32duino_index.json`
- Tools > Board > Boards Manager > Search 'STM32' > Install 'STM32F1xx/GD32F1xx boards'

### Libraries
Install the following libraries from the Library Manager
Tools > Manage Libraries... 

- NeoGPS: Install, and replace the contents of `$HOME/Arduino/libraries/NeoGPS/src/GPSport.h` by:
```
#ifndef GPSport_h
#define GPSport_h

#define gpsPort Serial2
#define GPS_PORT_NAME "Serial2"
#define DEBUG_PORT Serial

#endif
```
- BME280 by Tyler Glenn
- PMS Library by Mariusz Kacki

## Connect, compile and upload
Let us now try and upload a quick code to test if all works...

- Open the Blink example, and add `#define LED_BUILTIN PC13` at the very beginning
- Select the Board: Tools > Board > STM32F1 Boards (Arduino_STM32) > Maple mini
- Select the Port: Tools > Port > ... (depends on your OS)
- Press the Compile and Upload arrow....

If the upload does not go through, this might be because the board cannot auto-reset with your computer.
To upload, you will need to manually reset by pressing the button you can see near where the GPS chip is stuck just before pressing the Compile arrow. It takes a bit of trial and error and some OS's need it while some others don't.

## Upload the LogAir code
Download the code from the repository, and follow the same process as above.

## Connect
UART parameters to connect to the serial monitor are the ones set in the code, but are usually 9600, or 115200 for the latest versions.

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
