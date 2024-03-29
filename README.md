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

## Assemble
<details>
  <summary>Click to expand for instructions!</summary>
  
  ### Material

- LogAir PCB
- Bluepill + connectors
- BME280 + connector
- uSD module
- BLE module
- PMS7003 + connector
- DC step-up
- Charger module
- GPS module

- Soldering iron + tin
- double-sided tape + jelly tape if available
- cutting pliers
- flat pliers
  
  ### Instructions

0. A very first step should be to try and upload code to the Bluepill. This is before the rest as it happened that the chips could not be programmed, and realizing that after all that work is not ideal. Upload a bootloader, a blink code, and there you go.

1. Solder the Bluepill connectors. Use the holes in the PCB as a guide for alignment. The G (GND, ground) pins are harder to solder, because of the copper mass involved.
<div align="center"><img src=/img/1_PCB_and_Bluepill.jpg width=400px</img></div>

2. Solder the assembled Bluepill to the PCB. Pay attention to the orientation.
<div align=center><img src=/img/2_PCB_back.jpg width=400px</img></div>

3. Prepare the BME280 (solder the connectors), the BLE module (straighten the pins with a flat plier), and the uSD module
<div align=center><img src=/img/3_Prepare_BLE_BME280_SD.jpg width=400px</img></div>

4. Place the BLE, BME, and uSD modules. Follow the picture, the BME280 (violet module) should be as far as possible from the PCB), with its pins barely sticking through on the other side.
<div align=center><img src=/img/4.1_Placement_BLE_BME_SD.jpg width=400px</img></div>
<div align=center><img src=/img/4.2_Placement_BLE_BME_SD.jpg width=400px</img></div>

5. Prepare the PCB for the power modules: put a thin layer of tin on the (blue) charger module pads, and a solder blob on one of the (red) step-up pads
<div align=center><img src=/img/5.1_Power_Preparing.jpg width=400px</img></div>

6. Solder the charger module by putting it in place, and adding solder on the pads from the top. I count about 5 seconds of applying heat for the tin applied on step 5 to melt. Then solder the step-up module using the first pad to hold it in place. When you are done, check the conductivity between the pins of the two modules, and that between the pads of the charging module. The latter because the method used to solder can produce 'tin overflows'. 
<div align=center><img src=/img/5.2_Power_Place.jpg width=400px</img></div>
<div align=center><img src=/img/5.3_Power_Solder.jpg width=400px</img></div>

Then place and solder the switch. Here, try to apply a minimal amount of tin. As the other side of the pins will go against the PMS7003 which has a conductive casing, the pins should be cut or filed down, then taped over (two layers of normal, clear tape are enough)
<div align=center><img src=/img/6.2_Switch_Solder.jpg width=400px</img></div>
<div align=center><img src=/img/6.3_Switch_cut.jpg width=400px</img></div>

7. Bend lightly the SWD pins of the Bluepill to give room for soldering the GPS wires
<div align=center><img src=/img/6.1_Switch_Place.jpg width=400px</img></div>

8. Solder the GPS wires. The order is the same as their order on the GPS-side plug, you can use the PCB markings (5V is red) and pictures to help you. The wires should not go over each other at any point for the order to be preserved.
<div align=center><img src=/img/7.1_GPS_Prepare.jpg width=400px</img></div>
<div align=center><img src=/img/7.2_GPS_Solder.jpg width=400px</img></div>

9. Solder the connector for the PMS7003. As the width of the Bluepill vary a bit, it is safer to solder it lightly 'bent' away from the bluepill. To do that, solder the pins that are on the far side from the Bluepill first, then plug the PMS7003 in to see how thing fit. Then remove and solder the rest. 5-10 degrees should do.
<div align=center><img src=/img/8_PMS7003_pin_solder.jpg width=400px</img></div>

10. For a smaller device, I cut most of the pins apparent on the back of the PCB to the top of the tin cone. Apply insulation on the pins, and this is important. I use a sticky, translucent, think tape I found on aliexpress. Any other ~1-2mm insulating material will easily do.
<div align=center><img src=/img/9_Pin_Cut_and_Stickies.jpg width=400px</img></div>

11. Solder the battery wires to the charger module pads
<div align=center><img src=/img/10_Battery.jpg width=400px</img></div>

12. Before finalizing the assembly, write down the PMS7003 serial number somewhere, to track device quality :)
<div align=center><img src=/img/11_Sensor_info.jpg width=400px</img></div>

13. And it is done :)
<div align=center><img src=/img/12_Final.jpg width=400px</img></div>

  </details>
  
## Program

<details>
  <summary>Click to expand for programming instructions!</summary>
  
### Pre-requisites
- A Bluepill with functional bootload (e.g. generic_boot20_pc13.bin from https://github.com/rogerclarkmelbourne/STM32duino-bootloader
- Install Arduino IDE
- Install board definition:File > Preferences > Additional Boards Manager URLs: add the following URL to the list (empty by default):
  `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`
- Tools > Board > Boards Manager > Search 'STM32' > Install 'STM32 MCU based boards'
    
<details>
<summary>For older instructions using unofficial stm32duino core</summary>
  
- Install board definition:File > Preferences > Additional Boards Manager URLs: add the following URL to the list (empty by default):
`http://dan.drown.org/stm32duino/package_STM32duino_index.json`
- Tools > Board > Boards Manager > Search 'STM32' > Install 'STM32F1xx/GD32F1xx boards'
- Once done, close the Board Manager and Select the Board: Tools > Board > STM32F1 Boards (Arduino_STM32) > Maple mini
  
NO NEED TO FOLLOW THE CONFIGURE SECTION BELOW.
    
</details>
  
### Configure
  - Select the proper board:
  `Tools > Boards > STM32 Boards (selected from submenu) > Generic STM32F1 series`
  - Enable configurations we need:
    - `Tools > U(S)ART Support > Enabled (Generic 'Serial')`
    - `Tools > USB Support > CDC (Generic Serial supersedes U(S)ART)`
    - `Tools > Upload Method > Maple DFU Bootloader 2.0`

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
- Select the Port: Tools > Port > ... (depends on your OS)
- Press the Compile and Upload arrow....

If the upload does not go through, this might be because the board cannot auto-reset with your computer.
To upload, you will need to manually reset by pressing the button you can see near where the GPS chip is stuck just before pressing the Compile arrow. It takes a bit of trial and error and some OS's need it while some others don't.

## Upload the LogAir code
Download the code from the repository, and follow the same process as above.

## Connect
UART parameters to connect to the serial monitor are the ones set in the code. The default is 112500, but 9600 was previously used.

## Troubleshoot

### Board appears in dmesg, but not in lsusb 
You might need to create a udev rules file
`/etc/udev/rules.d/45-bluepill.rules`
```
ATTRS{idVendor}=="1eaf", ATTRS{idProduct}=="0004", MODE="0666" SYMLINK+="maple", ENV{ID_MM_DEVICE_IGNORE}="1"
ATTRS{idVendor}=="1eaf", ATTRS{idProduct}=="0003", MODE="0666" SYMLINK+="maple", ENV{ID_MM_DEVICE_IGNORE}="1"
```
  </details>
  
## Use


<details>
  <summary>Click to expand for usage instructions!</summary>
  
### Turning ON / OFF
The switch onthe back of the device allows to turn the device on and off.

The red or green blinking light shows when the device is writing on the SD card. When the LED is ON, the device should not be turned OFF as it might corrupt the card.

### Blinking LEDs

The modules we use include LEDs of varying colors. Here are some useful informations you can get from them:

- Bluetooth LED (green or red), left side of the device: This LED is blinking if no phone is connected to the device, and stays on when a connection is active.
- GPS LED (blue and red):
  - The red LED shows the module is powered, and should always be on when the device runs. 
  - The blue LED is blinking when the module receives signals strong enough to get a 'fix', or know its position. This is important if you rely on the SD card only to record the measurements, i.e. if you're not using a smartphone to send your data to the server.
- Device LEDs:
  - Left LED: red or green, signals the controller receives power. Should always be ON when the device is turned on.
  - Right LED: red or green, currently parametered to signal the device is writing to the SD card and should not be powered off. You can turn the device OFF when the LED is OFF.

### With the LogAir APP
Follow the instructions on our video tutorial.

### Stand-alone
The LogAir device records its measurements on the micro-SD card, which can be extracted and imported into a spreadsheet or GIS program.

The limitation in this mode is that the device can only rely on its own GPS module for time and position. This means that, before a valid GPS signal is acquired, the measurements on the SD card will not list any time or position. 

The signal depends on your location, the time of the day, if there are obstacles between the device and the sky, etc. Indoor measurements are often difficult, but windowsills sometimes give good results.

We are trying to find a way to give the current time to the device by using a program on the computer, or preparing it on the SD card, but there is no easy solution there... See ToDos, input welcome :)

</details>

# ToDo

Here are several items that are planned, currently developed, or on hold pending contributions. If you are interested in any of them be it for giving a helping hand or advices, please get in touch :)
  
## Hardware

These items focus on the device conception, including electronics, casing, production, etc.

<details>
  <summary>Click to expand for Hardware development tasks!</summary>
  
- Disconnect PC14 and PC15 for RTC reliability
- Better power supply, as this one just charges and discharges blindly.
- Integrate the components on a printed circuit board
- Replace the BLE module with an ESP32 (the STM32 might have to stay, as ESP32 are more limited, e.g., available UARTs)
- Make a similar MVP with SPS30 & SHT3x
- Add a LoRa chip! See Clement's work
- ...

</details>

## Firmware

Firmware development focuses on the program that makes the device run. The brain within the machine :)

<details>
  <summary>Click to expand for Firmware development tasks!</summary>

- Provide a time base when no GPS signal is present, and no phone is connected
  - Implement RTC (there is an on-board crystal on the Bluepill).
  - Implement time input from serial (BLE and/or PC)
- Implement better data storage and upload 
  - Log data with human-readable time as well as UNIX timestamp
  - Revamp buffering to limit uSD write operations
  - Revamp data storage on the SD card (textfile + DB?)
  - Implement deferred data upload for when uplink not continuously available
- Implement online GPS config: some changes to the embedded GPS FW are sometimes necessary for best operation
  - Get and store almanach/ephemeris on uSD card! This would really help with no-phone collection
  - Other generic config, constellations, accuracy requirements, etc.
- Implement component recognition: find out which sensors are used from their output stream (starting with PM sensor)
- Add interrupts everywhere?
- ...

</details>

## APP

Here, developmetn of our application that helps collect and send data from the device to the servers, and get it back in forms of maps, messages, etc. Our UI sucks? Contribute a better one ;)

<details>
  <summary>Click to expand for APP development tasks!</summary>
  
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

</details>

## Backend

All the work that helps gather the data, safekeep it, develop ways to use it, share it, etc. DBs, services, APIs and the like. Put your hoodie on and hack away!

<details>
  <summary>Click to expand for Backend development tasks!</summary>
  
  - Implement API key for upload/download
  - ...

</details>


## Data

All the work that develop ways to analyse, understand and use the data. Algorithms, GIS, dashboards etc.

 <details>
  <summary>Click to expand for Data visualisation and Data science tasks!</summary>
  
  - Make a dashboard! 
  - Build a QGIS server that works fast enough
  - Find missing data points in the contributions, and signal them for collection
  - Find periods in the signal, and exploit them
  - Build a better pipeline for extraction and analysis

</details>

# Upcoming

- Arduino library configs
- ToDo list updates
- Assembly instructions (video?)


Licensed under [CERN OHL v.1.2 or later](https://ohwr.org/project/cernohl/wikis/home)
