# Hardware docs for LogAir device

- Schematics
- PCB
- BoM
- Arduino code

The hardware as it is streams, once per second, the following data:
Latitude, Longitude, Altitude, Speed, Heading, Temperature, Relative Humidity, Atmospheric Pressure, PM1, PM2.5, PM4, PM10

The Arduino-compatible hardware, based on a STM32 MCU, is flashed with a code compiled with the Arduino IDE and the following libraries: NeoGPS, PMS Library, BME280, SdFat.

The protocol is extensible to allow implementation of different sensors.

Licensed under [CERN OHL v.1.2 or later](https://ohwr.org/project/cernohl/wikis/home)
