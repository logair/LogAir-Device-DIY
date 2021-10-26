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
//    LogAir is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <SPI.h>
#include <BME280Spi.h>

#include <SD.h>
#include <SDConfigFile.h>

#include <NMEAGPS.h>
#include <GPSport.h> // choose the GPS module serial port
#include <Streamers.h> // common set of printing and formatting routines for GPS data in csv, printed to debug output device

#include "PMS.h"

// Bluepill to PC SERIAL BAUDRATE
#define USB_SERIAL 115200

// SPI device pin definition
#define DEVICE_PIN PA4

// LED pin and status definition
#define LED_BUILTIN PC13 // User LED pin definition
#define LED_OFF 1 
#define LED_ON 0

// Default config values, if logair.cfg not present on SD card
#define default_deviceID "LA-CF"
#define default_fwID "CC_0.7_proto"
#define default_pmID "PMS7003"
#define default_rhID "GY-BME280"
#define default_gpsID "ATGM336H"
#define default_rfID "JDY_023_gen"
#define default_bleName "logair"

// GPS config 
const char baud115200 [] PROGMEM = "PUBX,41,1,3,3,115200,0";

/*
HardwareSerial SerialX( RX, TX)
*/
//HardwareSerial Serial1(PB7,PB6);
HardwareSerial Serial2(PA3,PA2);
HardwareSerial Serial3(PB11,PB10);

int battery;


int connected = 0;
int header_step = 10;

const uint8_t SDChipSelect = PB0; // SD Chip Select pin



// HEADER
char header [128] =""; // header buffer
  
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

// was const char*
void sendATCommand(char * command){

  Serial1.println(command);
  //wait some time
  delay(100);
  
  char reply[100];
  int i = 0;
  while (Serial1.available()) {
    reply[i] = Serial1.read();
    i += 1;
  }
  //end the string
  reply[i] = '\0';
  Serial.print(reply);
  Serial.println("Reply end");
}

// WORK FUNCTION, ALL TIME CONSUMiNG TASKS GO THERE
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

    int pres_i = round(pres);
    int temp_i=round(temp);
    int hum_i=round(hum);

///////////////////////////////////////////////////////// PMS7003
  if (pms.read(data)){
    // Check
    pm10 = data.PM_AE_UG_10_0 ;
    pm25 = data.PM_AE_UG_2_5 ;
  }

//  float pm25_corr, pm10_corr;
//  pm25_corr = pm25 / ( 1.0 + 0.48756*pow( ( hum / 100.0 ), 8.60068 ) );
//  pm10_corr = pm10 / ( 1.0 + 0.81559*pow( ( hum / 100.0 ), 5.83411 ) );


  sprintf(buffer, "%s,%d.%02d,%d.%02d,%d.%02d,,%d,,%d$", buffer, int(temp), int(temp*100)%100, int(hum), int(hum*100)%100, int(pres), int(pres*100)%100, pm25, pm10);
  sprintf(bufferSD, "%s,%d.%02d,%d.%02d,%d.%02d,,%d,,%d$", bufferSD, int(temp), int(temp*100)%100, int(hum), int(hum*100)%100, int(pres), int(pres*100)%100, pm25, pm10);

//  sprintf(buffer, "%s,%.1f,%.1f,%.1f,,%d,,%d$", buffer, temp, hum, pres, pm25, pm10);
//sprintf(bufferSD, "%s,%.1f,%.1f,%.1f,,%d,,%d$", bufferSD, temp, hum, pres, pm25, pm10);

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


  digitalWrite(LED_BUILTIN,LED_OFF);
  
  Serial.println(buffer);
  Serial1.println(buffer);
} // doSomeWork


//---------- SETUP ------------//
void setup()
{

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PA8, INPUT);
  Serial.begin( USB_SERIAL ); // USB Serial

  SPI.begin();
  bme.begin();
  
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
  
  // Bluetooth config
  Serial1.begin(9600); // Bluetooth

  sendATCommand("AT+BAUD0");
  sendATCommand("AT+BAUD0");
  sendATCommand("AT+RST");
  sendATCommand("AT+RST");

  Serial1.flush();
  delay(100);
  Serial1.end();
  Serial1.begin(115200);

  char bleNameString[sizeof("AT+NAME") + sizeof(bleName)]; 
  sprintf(bleNameString, "AT+NAME%s", bleName);

// The JDY-023 sometimes ignores the first AT+NAME commands given to it. We make sure it will get them again.
  for(int i=0; i<3; i++){
    Serial1.println(bleNameString);
    delay(100);
  }


  // CONFIG GPS TO 115200 bauds
  gpsPort.begin(9600); // Serial2
  gps.send_P( &gpsPort, (const __FlashStringHelper *) baud115200 );
  gpsPort.flush();
  delay(100);
  gpsPort.end();
  gpsPort.begin( 115200 ); 

  // Begin PMS7003
  Serial3.begin( 9600 ); // PMS7003

  // HEADER: deviceID, fwVersion, pmID, rhID, gpsID, rfID (radiofreq, BLE,LoRa,WiFi)
  sprintf(header, "{%s,%s,%s,%s,%s,%s$", deviceID, fwID, pmID, rfID, gpsID, rfID );

  Serial.println("end setup");
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
    // Check
    pm10 = data.PM_AE_UG_10_0 ;
    pm25 = data.PM_AE_UG_2_5 ;
  }

  if (header_step > 10){
    header_step = 0;

    Serial1.println(header);
    Serial.println(header);

    if (digitalRead(PA8)){
      Serial.println("Connected");
    } else {
      Serial.println("Not Connected");
    }
  }
}
