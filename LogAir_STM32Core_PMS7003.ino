//  License:
//    Copyright (C) 2017-2021, Emmanuel Kellner
//
//    This file is part of LogAir
//
//    LogAir is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Dust is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include <SPI.h>

#include <NMEAGPS.h>
#include <GPSport.h> // choose the GPS module serial port
#include <Streamers.h> // common set of printing and formatting routines for GPS data in csv, printed to debug output device
#include <BME280Spi.h>

#include "PMS.h"

// for convenience, going back to SD.h instead of SdFat
// because we don't want to modify SDConfigFile lib to use SdFat
// Cf. https://forum.arduino.cc/index.php?topic=287298.0
#include <SD.h>
#include <SDConfigFile.h>


#define DEVICE_PIN PA4
#define LED_BUILTIN PC13 // User LED pin definition

#define LED_OFF 1
#define LED_ON 0

#define default_deviceID "LA"
#define default_fwID "LA_0.8A"
#define default_pmID "PMS7003"
#define default_rhID "BME280"
#define default_gpsID "GG-1802"
#define default_rfID "JDY-023"
#define default_bleName "logair"

int battery;
int connected = 0;
int header_step = 10;
char header [128] =""; // header buffer

const uint8_t SDChipSelect = PB0; // SD Chip Select pin



static NMEAGPS  gps; // parse received characters into gps.fix() data structure
static gps_fix  fix; // define set of GPS fix information. Will hold on to various pieces as received from RMC sentence

BME280Spi::Settings settings(
   DEVICE_PIN,
   BME280::OSR_X1, // Temperature
   BME280::OSR_X1, // Humidity
   BME280::OSR_X1, // Pressure
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False
   ); // Default : forced mode, standby time = 1000 ms, Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,

BME280Spi bme(settings);

PMS pms(Serial3);
PMS::DATA data;

int pm1, pm25, pm4, pm10;
int batt = 1 ;

// The filename of the configuration file on the SD card
const char CONFIG_FILE[] = "logair.cfg";

// CONFIG
boolean didReadConfig;
char *deviceID;
char *fwID;
char *pmID;
char *rhID;
char *gpsID;
char *rfID;
char *bleName;

/* READ CONFIG FILE VALUES TO VARIABLES */
boolean readConfiguration() {
  const uint8_t CONFIG_LINE_LENGTH = 127;

  // The open configuration file.
  SDConfigFile cfg;

  // Open the configuration file.
  if (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
    Serial.print("Failed to open configuration file: ");
    Serial.println(CONFIG_FILE);
    return false;
  }

  // Read each setting from the file.
  while (cfg.readNextSetting()) {

    if (cfg.nameIs("deviceID")){ deviceID = cfg.copyValue(); }
    else if (cfg.nameIs("fwID")){ fwID = cfg.copyValue(); }
    else if (cfg.nameIs("pmID")){ pmID = cfg.copyValue(); }
    else if (cfg.nameIs("rhID")){ rhID = cfg.copyValue(); }
    else if (cfg.nameIs("gpsID")){ gpsID = cfg.copyValue(); }
    else if (cfg.nameIs("rfID")){ rfID = cfg.copyValue(); }
    else if (cfg.nameIs("bleName")){ bleName = cfg.copyValue(); }
    else {}
    }

  // clean up
  cfg.end();
  return true;
}


/* WORK FUNCTION */ // ALL TIME CONSUMING TASKS GO THERE
static void doSomeWork()
{
  char buffer [128] =""; // buffer
  char bufferSD [256] = ""; // buffer for SD writing

  sprintf(bufferSD, deviceID);

  if (fix.valid.location){
    uint32_t timestamp = (NeoGPS::clock_t) fix.dateTime;
    timestamp += 946684800; // convert from Y2K to UNIX Epoch

    sprintf(bufferSD, "%s,%" PRIu32 "", bufferSD, timestamp); // timestamp in seconds from EPOCC Y2K

    sprintf(buffer, "[%f,%f,%d,%.1f,%.1f",
        fix.latitude(), fix.longitude(), (int) round(fix.altitude()),
        fix.speed_kph(), fix.heading() );

    sprintf(bufferSD, "%s,%f,%f,%d,%.1f,%.1f", bufferSD,
        fix.latitude(), fix.longitude(), (int) round(fix.altitude()),
        fix.speed_kph(), fix.heading() );

  } else {
    sprintf(buffer, "[,,,,");
    sprintf(bufferSD, "%s,,,,", bufferSD);
  }

///////////////////////////////////////////////////////// BME280
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  bme.read(pres, temp, hum, tempUnit, presUnit);

///////////////////////////////////////////////////////// PMS7003
  if (pms.read(data)){
    // Check
    pm10 = data.PM_AE_UG_10_0 ;
    pm25 = data.PM_AE_UG_2_5 ;
  }

  // Had to go from %.1f to %d.%02d and int(x),int(x*100)%100 to go around a float issue with config of the new IDE
  // quite sure there is a better way, but not for now.
  // sprintf(buffer, "%s,%.1f,%.1f,%.1f,,%d,,%d$", buffer, temp, hum, pres, pm25, pm10);
  // sprintf(bufferSD, "%s,%.1f,%.1f,%.1f,,%d,,%d$", bufferSD, temp, hum, pres, pm25, pm10);

  sprintf(buffer, "%s,%d.%02d,%d.%02d,%d.%02d,,%d,,%d$", buffer, int(temp), int(temp*100)%100, int(hum), int(hum*100)%100, int(pres), int(pres*100)%100, pm25, pm10);
  
  sprintf(bufferSD, "%s,%d.%02d,%d.%02d,%d.%02d,,%d,,%d$", bufferSD, int(temp), int(temp*100)%100, int(hum), int(hum*100)%100, int(pres), int(pres*100)%100, pm25, pm10);

  // Turn LED ON as we will open the SD file and write. DO NOT POWER OFF DEVICE BEFORE LED_OFF
  digitalWrite(LED_BUILTIN,LED_ON);

  File logfile = SD.open("dust.txt", O_RDWR | O_APPEND);  // open logfile
  if (!logfile){                                          // if file doesn't exist
    logfile = SD.open("dust.txt", O_RDWR | O_CREAT);

    if(!logfile){                                         // Failed logfile opening/creation.
      if(Serial) Serial.println( F("Failed to open dust.txt") ); // Send message. ADD LED BLINKING
    }
      logfile.println("Header:DeviceId,FW_Version,pmSensorID,relhSensorID,gpsID,rfID");
      logfile.println("Body:Latitude,Longitude,Temperature[C],Relative_Humidity[%RH],Pressure[Pa],PM1,PM2.5,PM4,PM10");
  }

  logfile.println(bufferSD);
  logfile.close();

  // Turn LED OFF, can power off if needed.
  digitalWrite(LED_BUILTIN,LED_OFF);

  // Write to UARTs for USB and BLE
  Serial.println(buffer);
  Serial1.println(buffer);
} // doSomeWork


//---------- SETUP ------------//
void setup()
{

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PA8, INPUT);
  
  // SPI and SD first to read config
  SPI.begin();
  SD.begin(SDChipSelect);

  // Configuration
  didReadConfig = false;

  // giving default value to variables
  deviceID = default_deviceID;
  fwID = default_fwID;
  pmID = default_pmID;
  rhID = default_rhID;
  gpsID = default_gpsID;
  rfID = default_rfID;
  bleName = default_bleName;

  // Read our configuration from the SD card file.
  didReadConfig = readConfiguration();

  delay(1000);

  // USB Serial
  Serial.begin( 115200 ); 
  
  // TODO: Config BLE (BAUD from 9600 to 115200 if needed) 
  Serial1.begin( 115200 ); // Bluetooth
  Serial1.write("AT+NAME");
  Serial1.write(bleName);
  Serial1.println("");

  // TODO: Implement GPS config from file on SD. 
  delay(2000);
  gpsPort.begin( 115200 ); // Serial2


  // PMS7003
  Serial3.begin( 9600 ); // SDS018

  bme.begin();

  // HEADER: deviceID, fwVersion, pmID, rhID, gpsID, rfID (radiofreq, BLE,LoRa,WiFi)
  sprintf(header, "{%s,%s,%s,%s,%s,%s$", deviceID, fwID, pmID, rfID, gpsID, rfID );

  Serial.flush();
  Serial1.flush();
}


void loop()
{
  while (gps.available( gpsPort )) {
    fix = gps.read();
    doSomeWork();
    header_step ++;
  }

  if (pms.read(data)){
    pm10 = data.PM_AE_UG_10_0 ;
    pm25 = data.PM_AE_UG_2_5 ;
  }

  if (header_step > 10){
    header_step = 0;

    Serial1.println(header);
    Serial.println(header);

    if (digitalRead(PA8)){
      Serial.println("BLE Connected");
    } else {
      Serial.println("BLE Not Connected");
    }
  }
}

