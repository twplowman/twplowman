/*!
 * @file Plowman_Temperature.cpp
 *
 * @mainpage Plowman Monitoring
 *
 * @section intro_sec Introduction
 *
 * This is a library for Plowman Temperature monitoring.
 *
 */
//

#include "Adafruit_FONA.h"
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Plowman_Temperature.h>
#include <SD.h>
#define ONE_WIRE_BUS 19 
#define RED 16
#define GREEN 17
#define BLUE 18



    #define DEBUG

    #ifdef DEBUG
        #define DEBUG_PRINTDEC(x) Serial.print (x, DEC)
        #define DEBUG_PRINTHEX(x) Serial.print (x, HEX)
        #define DEBUG_PRINTLN(x)  Serial.println (x)
    #else   
        #define DEBUG_PRINTDEC(x)
        #define DEBUG_PRINTHEX(x) 
        #define DEBUG_PRINTLN(x) 
    #endif

    #ifdef DEBUG_FULLPATH
    #define DEBUG_PRINT(str) \
        Serial.print(millis());     \
        Serial.print(": ");    \
        Serial.print(__PRETTY_FUNCTION__); \
        Serial.print(' ');      \
        Serial.print(__FILE__);     \
        Serial.print(':');      \
        Serial.print(__LINE__);     \
        Serial.print(' ');      \
        Serial.println(str);
    #else
    #define DEBUG_PRINT(str)
    #endif

    //to do - DEBUG SAVE

/**
* @brief Find all devices on the one wire bus.
* @param pin This is the pin to find the bus on.
* @return Void
*/
String Plowman_Temperature::findAllDevices(uint8_t busWire)
{
  String addressStore;
  DEBUG_PRINTLN(__PRETTY_FUNCTION__);
  DEBUG_PRINTLN("Start oneWireSearch");
  bool value = false;
  while (value == false)
  {
    addressStore = findDevices(busWire);
    for (int val = 0; val<100; val++)
    {
      if(addressStore == "0")
      {
        DEBUG_PRINTLN("We found nothing, retry");
        delay(200);
        value = false;
      }  
      else
      {
        DEBUG_PRINTLN("We found something!");
        value = true;
      }
      if (val > 20){return addressStore;}
    }
  }
  DEBUG_PRINTLN("End oneWireSearch");
  return addressStore;
}

/**
* @brief Lower level function used in findAllDevices.
* @return true if any device is found, false if no device or error. 
*/
String Plowman_Temperature::findDevices(uint8_t busWire)
{
  OneWire ow(busWire);

  uint8_t address[8];
  uint8_t count = 0;
  
  if (ow.search(address))
  {
    String addressStore;
    DEBUG_PRINT("\nuint8_t pin ");
    DEBUG_PRINTDEC(ONE_WIRE_BUS);
    DEBUG_PRINT("[][8] = {");
    do {
      count++;
      Serial.print("  {");
      addressStore += " {";
      for (uint8_t i = 0; i < 8; i++)
      {
        Serial.print("0x");
        addressStore += "0x";
        if (address[i] < 0x10) 
        {
          Serial.print("0");
          addressStore += "0"; 
        }
        DEBUG_PRINTHEX(address[i]);
        addressStore += String(address[i],HEX);
        if (i < 7) Serial.print(", ");
        addressStore += "."; 
      }
      addressStore += "}"; 
    } while (ow.search(address));

    DEBUG_PRINTLN("};");
    DEBUG_PRINT("// nr devices found: ");
    DEBUG_PRINTLN(count);
    DEBUG_PRINTLN("ADDRESS STORE: ");
    DEBUG_PRINTLN(addressStore);
    return addressStore;
  }
  else
  {
    return "0";
  }
}


float Plowman_Temperature::tempbyIndex(int index, int busWire)
{

  OneWire oneWire(busWire); 
  DallasTemperature sensors(&oneWire);

    sensors.requestTemperatures();
    float temperature;
    temperature = sensors.getTempCByIndex(index);
    //DEBUG_PRINTLN(temperature);
    return temperature;
}



void Plowman_Temperature::DS18TemperatureExample(uint8_t busWire) 
{
  OneWire oneWire(busWire); 
  DallasTemperature sensors(&oneWire);
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !oneWire.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    oneWire.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  oneWire.reset();
  oneWire.select(addr);
  oneWire.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = oneWire.reset();
  oneWire.select(addr);    
  oneWire.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = oneWire.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
}


void Plowman_SD::initialiseSD(int sdSelectPin)
{
  
  DEBUG_PRINTLN(__PRETTY_FUNCTION__);
  int retryAttempts = 0; 
  online = false;
  while (!online)
  {
    DEBUG_PRINT(online);
    if (!SD.begin(sdSelectPin)) {
        DEBUG_PRINTLN("Failed to connect to SD Card, Retrying");
        Serial.println("Failed to connect to SD, Retrying...");
        online = false;
        retryAttempts ++;
        delay(1000);
        if (retryAttempts >= 2) {DEBUG_PRINTLN("Breaking from loop"); break;}
    }
    else
    {
      online = true;  
      DEBUG_PRINTLN("Connected to SD Card");
      return;
    }  
  }
  // We didn't connect
  DEBUG_PRINTLN("Failed after timeout...");
  online = false;

}


bool Plowman_SD::DebugBoolPrint(String data, String uctTime)
{
  File myFile;
  String fileName = "DEBUG.txt";
  if (!SD.exists(fileName)) { //check if file exists
      myFile = SD.open(fileName, FILE_WRITE);
      myFile.close();
      delay(100);
    }
    myFile = SD.open(fileName, FILE_WRITE);
    if (myFile) { //Make sure File has opened
      myFile.print(uctTime);
      myFile.print(" -> ");
      myFile.println(data);
      // close the file:
      myFile.close();
      return true;
    } else { //If file fails to open!
      Serial.println("Failed to write to SD Card");
      return false;
    }
}

bool Plowman_SD::NewFile(String uctTimeSD, String versionNumber = "Pico 0.2", String livestockBoxNo = "Pre-Dev 001")
{
  Plowman_Temperature plowmantemp;
  DebugBoolPrint("Creating new file...", uctTimeSD);
  String fileName = "G" + uctTimeSD + ".csv";
  File myFile;
  String dataln1, dataln2, dataln3;
  Serial.println("************");
  Serial.println(fileName);
  Serial.println("************");
  
  dataln1 = "Plowman Livestock, Temperature Monitoring System, Version No: " + versionNumber + ",Box No: " + livestockBoxNo;
  dataln2 = "Temperature sensor address:," ;//+ String(plowmantemp.findAllDevices());
  dataln3 = "Date, Time, Latitude, Longitude, Temperature";

  if (!SD.exists(fileName)) { //check if file exists
      myFile = SD.open(fileName, FILE_WRITE);
      myFile.close();
      delay(100);
      DebugBoolPrint("File Doesn't Exist, Creating new File", uctTimeSD);
      myFile = SD.open(fileName, FILE_WRITE);
      if (myFile) { //Make sure File has opened
      myFile.println(dataln3);
      // close the file:
      myFile.close();
      return true;
    } else { //If file fails to open!
      // check we have initialised SD CARD
      //We need to go somewhere from here.
      Serial.println("Failed to write to SD Card");
      SD.end();
      return false;
    }
  }
  myFile = SD.open(fileName, FILE_WRITE);
    if (myFile) { //Make sure File has opened
      myFile.println(dataln1);
      myFile.println(dataln2);
      myFile.println(dataln3);
      // close the file:
      myFile.close();
      return true;
    } else { //If file fails to open!
      // check we have initialised SD CARD
      //We need to go somewhere from here.
      Serial.println("Failed to write to SD Card");
      SD.end();
      return false;
    }
    Serial.println(dataln1);
    Serial.println(dataln2);
    Serial.println(dataln3);
}

bool Plowman_SD::LogGPSInfo(String data, String uctTimeSD)
{
  DebugBoolPrint("Logging Info...",uctTimeSD);
  String fileName;
  File myFile;
  fileName = "G" + uctTimeSD + ".csv";
  if (!SD.exists(fileName)) 
  { //check if file exists
    myFile = SD.open(fileName, FILE_WRITE);
    myFile.close();
    delay(100);
    DebugBoolPrint("GXXXXX.csv File Doesn't Exist, Creating new File", uctTimeSD);
  }
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) 
  { //Make sure File has opened
    
    myFile.println(data);
    // close the file:
    myFile.close();
  } 
  else 
  { //If file fails to open!
    // check we have initialised SD CARD
    //We need to go somewhere from here.
    Serial.println("GXXXXX.csv Failed to write to SD Card");
    SD.end();
    return false;
  }
  Serial.println(data);
  return true;
}

//DONT FORGET THAT THE LENGTH OF THE FILENAME IS LIMITED
bool Plowman_SD::SaveDataWaitingToSend(String gprsmessage, String uctTimeSD)
{
  DebugBoolPrint("Saving Unsent data",uctTimeSD);
  String fileName;
  File myFile;
  fileName = "Unsent.txt";
  if (!SD.exists(fileName)) 
  { //check if file exists
    myFile = SD.open(fileName, FILE_WRITE);
    myFile.close();
    delay(100);
    //DebugBoolPrint("DataWaitingToSend File Doesn't Exist, Creating new File", uctTimeSD);
    Serial.println("Unsent File Doesn't Exist, Creating new File");  
  }
  myFile = SD.open(fileName, FILE_WRITE); 
  if (myFile) 
  { //Make sure File has opened
    Serial.println("Writing GPRS Message");
    myFile.println(gprsmessage);
    // close the file:
    myFile.close(); 
  } 
  else 
  {  
    //If file fails to open!
    // check we have initialised SD CARD
    //We need to go somewhere from here.
    Serial.println("Unsent Failed to write to SD");
    SD.end();
    return false;
  }
  Serial.println(gprsmessage);
  return true;
}

bool Plowman_SD::ReadDataWaitingToSend(String uctTimeSD) 
{
  String fileName;
  String buffer;
  File myFile;
  uint8_t count = 0;
  fileName = "Unsent.txt";

  if (!SD.exists(fileName)) 
  {
    DebugBoolPrint("Attempting to read 'waiting to send' - File Doesn't Exist", uctTimeSD);
    return false;
  }

  myFile = SD.open(fileName,FILE_READ);

  while (myFile.available())
  {
    buffer = myFile.readStringUntil('\n'); //read the line. 
    Serial.println(buffer);
    count ++;
  }
  Serial.println("No of lines = " + String(count));
  myFile.close();
  return true;
}


String Plowman_SD::ReadFirstLineUnsentData(String uctTimeSD)
{
  String fileName;
  String buffer;
  File myFile;
  fileName = "Unsent.txt";
  if (!SD.exists(fileName))
  {
    DebugBoolPrint("Attempting to read 'waiting to send' - File Doesn't Exist", uctTimeSD);
    return "";
  }
  myFile = SD.open(fileName,FILE_READ);
  while (myFile.available())
  {
    buffer = myFile.readStringUntil('\n'); //read the first line. 
    return buffer;
  }
}

bool Plowman_SD::DeleteFirstLineUnsentData(String uctTimeSD) //TODO
{
  File tempFile;
  File newFile;
  uint8_t lineNumber;
  String buffer;
  SdVolume SDvolume;

  lineNumber = 1;

  if(!SD.exists("Unsent.txt"))
  {
    DebugBoolPrint("Unsent.txt doesn't exist",uctTimeSD);
    return false;
  }

  tempFile = SD.open("Unsent.txt", FILE_READ);
  newFile = SD.open("newfile.txt",FILE_WRITE);

  if (newFile && tempFile)
  {
    
    while (tempFile.available())
    {
      
      buffer = tempFile.readStringUntil('\n'); // line by line 
      if (!lineNumber == 1)
      {
        newFile.println(buffer); // if line number is 1 we are on the first line and we don't want to copy
      }
      lineNumber ++ ;
    }
  }
  tempFile.close();
  newFile.close();

  SD.remove("Unsent.txt");
}


String Plowman_SD::ReadSetPointFromFile(String fileName)
{

  String buffer;
  File myFile;

  if (!SD.exists(fileName)) 
  {
    return "File Doesn't Exist";
  }

  myFile = SD.open(fileName,FILE_READ);

  if (myFile)
  {
    buffer = myFile.readStringUntil('\n'); //read the line. 
    Serial.println(buffer);
  }
  myFile.close();
  return buffer;

}

void Plowman_SD::WriteSetPointToFile(String fileName, String value)
{

  File myFile;
  if (SD.exists(fileName))
  {
    SD.remove(fileName); // Remove current file
  }
  myFile = SD.open(fileName,FILE_WRITE);

  if (myFile)
  {
    myFile.println(value); 
  }

  myFile.close();

}














void Plowman_LED::rgbRed()
{
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, HIGH);
}

void Plowman_LED::rgbGreen()
{
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, HIGH);
}

void Plowman_LED::rgbBlue()
{
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, LOW);
}

void Plowman_LED::rgbOrange()
{
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, HIGH);
}

void Plowman_LED::rgbCyan()
{
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, LOW);
}

void Plowman_LED::rgbOff()
{
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, HIGH);
}

void Plowman_LED::rgbSetup()
{
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
}
