/*
 * @file Plowman_Temperature.h
 */

#include <OneWire.h> 
#include <DallasTemperature.h>

class Plowman_Temperature {

public:
    String findAllDevices(uint8_t busWire);
    float tempbyIndex(int index, int busWire);
    uint8_t getDeviceAddress(int index);
    float tempbyAddress(DeviceAddress deviceAddress);
    void DS18TemperatureExample(uint8_t busWire);

private:
    String findDevices(uint8_t busWire);
};

class Plowman_SD {

public:

void initialiseSD(int sdSelectPin);
void LogInfo(String data, bool save, char file);
bool DebugBoolPrint(String data, String ucTtime);
bool NewFile(String uctTimeSD, String versionNumber, String livestockBoxNo);
bool LogGPSInfo(String data, String uctTimeSD);
bool SaveDataWaitingToSend(String gprsmessagechar, String uctTimeSD);
bool ReadDataWaitingToSend(String uctTimeSD);
String ReadFirstLineUnsentData(String uctTimeSD);
bool DeleteFirstLineUnsentData(String uctTimeSD);
String ReadSetPointFromFile(String fileName);
void WriteSetPointToFile(String fileName, String value);

String versionNumber = "Pico 0.2";
String livestockBoxNo = "Pre - Dev 001";
bool online = false;


};


class Plowman_LED {

public:

void rgbSetup();
void rgbRed();
void rgbGreen();
void rgbBlue();
void rgbOrange();
void rgbCyan();
void rgbOff();


};