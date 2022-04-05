///TPLOWMAN ADAFRUIT SIM808 GPS DATA PROGRAM///
///VPico_1.0.0__________________________030122///
String versionNumber = "pico v1.0.1";
String livestockBoxNo = "PBL004";
unsigned long fanTimeout = 300000; // 5 minute timout for fans
float largePigsTemperatureSetpoint = 30.0; // large pigs fan temperature set point is 30 degrees. (Should be saved in SD)
float smallPigsTemperatureSetpoint = 27.0; // small pigs fan temperature set point is 27 degrees. (Should be saved in SD)
///info_________________________________info///
///__Version Developed whilst at Huby____///
/*
v4.3. || 130921  
- Added delay after sending text: seems to carry on program before text is sent therefore
further commands don't work. fix doesn't work.... 
v4.4. 
GPRS POST Changed
Added return after GPRS 'failed to connect'
Debug Print the status code
changed char * = new char to char[length+1] when sending GPRS message
v4.5.
Added RGB LED Outputs
v.4.6
Added GSM switching for EE and giffgaff
v.4.7. || 241021
Adding Adafruit Screen Module.
v.4.8. || 281021
FYI GSM FAIL TO CONNECT CODES
- 51728. General failure? Just retry - We had GSM and Signal
- 601. GSM not turned on. 
- 55932. General failure? GPRS was only just turned on...
- GSM Status Code print out
v.4.9. || 011121
- Attempt to post to mySQL server
- if code returned is 200, ensure GPRS is 'ON'
v.4.10 
- Changed GPRS Settings and strings for version numbers
- Need a better way to roll out for different modules.
- 51728 if the server is down.
- Removed GPRS Post. Only need to post to SQL now.
V .5.1. || 161121
- Change to v0.5 as we are now rolling out to multiple devices. 
- Changed livestock box number to more readable for website PBL001 etc... 
V .5.2
- Intialise Functions Seperately
- If communications lost with fona, re-initialises
- Added Function to track the status codes - can be expanded on.
V .5.3. || 271121
- Changed a lot of the code including the header and cpp files
- Should now save unsent data to a folder so it can be sent later on
- Still needs testing
- Reset values to false when re-initialising fona (GPS particularly). 
- need to make the leds obsolete. removing from the code. 
V .5.4. || 031221
- Changed pinouts
- Added the stayalive pi.
V .5.5
- Version for PBL 001

V .5.6. || 201221
- Added average temp between all sensors
- Added info on screen for temp sensors
- Added menu functionality to change temperature setpoints and fan timout
- Added fan output and timeout for fans on.
- Header and cpp files changed to allow SD read and write to intial values. 
- Changed GUI 

V 1.0.0 || 030122
- Version for Release. 
- Added if value = 5 roaming - still send values. 
- Added send data about fan status.

V 1.0.1 || 090122
- Fixed bug that resetting server will cause the pi to get stuck in an endless loop trying to turn GPRS back on. 
- Now it always tries to post to the server, even if it thinks GPRS is turned off. If it succeeds to post, then GPRS changes back to true!
- If we don't have GPS, we will also send data to the server (as a test - debug) Should be removerd for release 


To Do:  Reinitialise TFT Screen.
        Change the way data is posted to the server. 
        Increase GUI size
        Show the number of temp sensors working
        If text is sent with certain values, do something to the pi! 

*/
///_________________________________________///
#include <Plowman_Temperature.h>   
Plowman_Temperature plowman;
Plowman_SD plowmanSD;
///________PINOUT___FOR RPI_________________///
// --------808 FONA ------------
  #include "Adafruit_FONA.h"
// Vio              3V
// GND              GND
// Key              GND (always on)
// RX               GPIO 0 (TX)
// TX               GPIO 1 (RX)
// RST              GPIO 10
  #define FONA_RST 10
// -------- SD/SCREEN CARD MODULE -------
  #include <SPI.h>
  #include <SD.h>
  #include <Adafruit_GFX.h>    // Core graphics library
  #include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
// SS (SELECT)      GPIO 8
// MOSI             GPIO 3
// MISO             GPIO 4
// SCK              GPIO 2
// 3v power
// TFT CS           GPIO 5
// TFT RESET        GPIO 6
// TFT DC           GPIO 7
  byte sdSelectPin = 8;
  #define TFT_CS   5
  #define TFT_RST  6 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC   7 
// -------- ONE WIRE TEMPERATURE  -------
// One Wire Pin     GPIO 12 // changed
// -------- RESET BUTTON -------
// RUN PIN          Phyiscial pin 30
//
///________PINOUT__________________________///

//SETUP **FONA MODULE**
HardwareSerial *fonaSerial = &Serial1;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST); // Use this for FONA 800 and 808s
int fonaNoReply = 0;
bool gpsOn = false;
bool gprsOn = false;
bool sslOn = false;
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
int gpsFixint;
String message = "no data yet";
String sqlData = "no SQL data yet";
char smsmessage[150];
char replybuffer[255];
bool enableSSL = false;

//SETUP **SD/SCREEN MODULE**
File myFile;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST); //(larger screen)
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); //(small screen)


//Fan temperature set point button pin numbers. 
#define buttonSelect 11
#define buttonOK 12
#define fanOutput 15
#define livestockSelectButton 13
unsigned long fanStartTime = 0; 
bool fanStatus = false;

//SETUP **TEMP SENSORS**
#define busWire1 16 
#define busWire2 17 
#define busWire3 18
#define busWire4 19
#define busWire5 20
#define busWire6 21
#define busWire7 22
#define busWire8 26 

//SETUP **DEFAULT VALUES**
bool newSession; 
bool newDebugSession;
String uctTimeSD = "00000";
String uctTime = "XX/XX/XXXX,XX:XX:XX"; 
String sqlTime = "YYYY-MM-DD hh:mm:ss";

#define piAlivePin 28 
#define piOnPin 14
bool piAlive = false;

//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//


// serial testing for the fona
void ATCommands()
{
  Serial.print(F("FONA> "));
  while (! Serial.available() ) 
  {
    if (fona.available()) 
    {
      Serial.write(fona.read());
    }
  }
}
  char command = Serial.read();
  //Serial.println(command);


//


/**
 * @brief Initialise Fona 808 Module
 * Set network settings (i.e. for EE)
 * Reset global variables to false
 */
void InitialiseFona()
{
  fonaNoReply = 0;
  gpsOn = false;
  gprsOn = false;
  DebugPrint("Opening communcations with 808");
  fonaSerial->begin(4800);
  fona.begin(*fonaSerial);
  //GIFFGAFF
  //fona.setGPRSNetworkSettings(F("giffgaff.com"), F("giffgaff"), F(""));
  // EE 
  fona.setGPRSNetworkSettings(F("everywhere"), F("eesecure"), F("secure"));
   // Turn on HTTPS redirect
  if (enableSSL)
  {
    fona.setHTTPSRedirect(true);
    DebugPrint("HTTP Redirect = True");
  }
}

/**
 * @brief Initialise Adafruit TFT 2" Screen
 * Run a quick cycle to visually check it works. 
 */
void InitialiseScreen()
{
  tft.init(240, 320); // Init ST7789 320x240
  //tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  tftSD(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);
}

/**
 * @brief Attempt to turn on GPRS from Fona Module
 */
void GPRSsetup()
{ 
  
  DebugPrint("GPRS Setup");
  if (!fona.enableGPRS(true))
  {
    gprsOn = false;
    DebugPrint("Failed to turn on GPRS");
    return;
  }
  gprsOn = true;
  DebugPrint("GPRS Turned on");
  


  delay(100);

}

/**
 * @brief Attempt to turn on GPS from Fona Module
 */
void GPSsetup() //Turn on GPS
{ 
  DebugPrint("GPS Setup");
  fona.enableNetworkTimeSync(true);
  if (!fona.enableGPS(true)) 
  {
    gpsOn = false;
    DebugPrint("Failed to turn on GPS");
    return;
  }
  gpsOn = true;
  DebugPrint("GPS Turned on");
  delay(100);
}

/**
 * @brief Posts latest data to server and checks if there's any data waiting to send
 *
 * @param sqlMessage String: data to send to the server. 
 * @return true: success, false: failure
 */
bool CheckAndPostToServer(String sqlMessage, bool gpsFix)
{
    if (!gprsOn) //Check if GPRS is turned on.
    { 
      DebugPrint("Attempting to turn on GPRS"); 
      GPRSsetup();
    }
    //else // GPRS is probably turned on, so we will attempt to post to the server. 
    //{
      if (gpsFix)
      {
        String HTTPUrl; //Define Url character
        HTTPUrl = ReadHTTPURL();
        char charUrl[150];
        strcpy(charUrl, HTTPUrl.c_str());
        GPRSPostSQL(sqlMessage, charUrl);
        tftHTTPAddress(HTTPUrl);
        
        // Then we check if we have any data to send. 
        if (CheckForDataToSend())
        {
          //ConstructSQLMessage(); // we need to limit this though. otherwise we won't get the correct time between readings. 
        }
        else
        {
          //we have nothing to send! which is good.
        }
        
        return true; // we got to the end of the function.
      }
      else
      {
        String HTTPUrl; //Define Url character
        HTTPUrl = ReadHTTPURL();
        char charUrl[150];
        strcpy(charUrl, HTTPUrl.c_str());
        GPRSPostSQL(sqlMessage, charUrl);
        tftHTTPAddress(HTTPUrl);
        return false; // we didn't bother using the function because no gps signal.
      }
    //}  
}

bool GPRSPostSQL(String sqlMessage, char *url)
{
  uint16_t statuscode;
  int16_t length;
  String gprsmessage;
  //url = "http://18.170.89.178:8080/post/sql/v0.5.3/";
  gprsmessage =  String("{\"value\":\"" + sqlMessage + "\"}");
  char gprsmessagechar[gprsmessage.length() + 1];
  strcpy(gprsmessagechar, gprsmessage.c_str());
  DebugPrint(gprsmessagechar);
  DebugPrint(String(fona.GPRSstate()));
  DebugPrint("Sending Data to Server");

  // Check SSL is turned on
  if (enableSSL)
  {
    if (!fona.HTTP_ssl(true)) // Enable SSL 
    {
      DebugPrint("Failed to enable SSL");
      tftSSL(false);
      sslOn = false;
    }
    else
    {
      tftSSL(true);
      sslOn = true;
    }
  }

 


  if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) gprsmessagechar, strlen(gprsmessagechar), &statuscode, (uint16_t *)&length)) {
    DebugPrint("Failed to connect!");
    DebugPrint("Status Code: " + String(statuscode));
    tftGprsStatus(statuscode);
    GPRSStatusCodeAction(statuscode);
    // Save the message to a new file if we failed to send it.
    plowmanSD.SaveDataWaitingToSend(sqlMessage, uctTimeSD);
    return false;
  }
  while (length > 0) 
  {
    while (fona.available()) 
    {
      char c = fona.read();            
      Serial.write(c);
      length--;
      if (! length) break;
    }
  }
  fona.HTTP_POST_end();
  DebugPrint("Data Sent to the server");
  DebugPrint("Status Code: " + String(statuscode));
  GPRSStatusCodeAction(statuscode);
  tftGprsStatus(statuscode);
  return true;
}

void GPRSStatusCodeAction(uint16_t statuscode)
{
  switch (statuscode)
  {
    case 200:
      gprsOn = true;
      break;

    case 601:
      gprsOn = false;
      DebugPrint("Turning off GPRS");
      break;
      
    default:
      gprsOn = true; // Mostly GPRS is on and it may not have had signal to send. 
      break;
  }
}

//Gets GPS Data and any sensor data
//Returns string in correct format. 
//Does not save to SD card. 
String ConstructSQLMessage() //TODO
{
  String sqlTemperature = SQLGetTemperatures(false);
  // get any more data here

  //construct the query. leave some vat for more sensors in the future. 
  // Format is: Box Number, DateTime, Latitude, Longitude, ... 
  // ..      ...T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, S1, S2, S3, S4, S5, S6, S7, S8, ID
  String sqlMessage = sqlData + sqlTemperature;
  return sqlMessage;
}

//Gets all temperatures
String SQLGetTemperatures(bool debugPrint) //TODO
{
  // Search for temperature sensors on specified pins
  float t1 = plowman.tempbyIndex(0,busWire1);
  float t2 = plowman.tempbyIndex(0,busWire2);
  float t3 = plowman.tempbyIndex(0,busWire3);
  float t4 = plowman.tempbyIndex(0,busWire4);
  float t5 = plowman.tempbyIndex(0,busWire5);
  float t6 = plowman.tempbyIndex(0,busWire6);
  float t7 = plowman.tempbyIndex(0,busWire7);
  float t8 = plowman.tempbyIndex(0,busWire8);

  float tArray[8] = {t1,t2,t3,t4,t5,t6,t7,t8}; // Put all in an array to pass into ave temp function. 
  float tAverage = CalculateAverageTemperature(tArray); 

  tftTemperatureAverage(tAverage); //Print to screen
  ShouldWeTurnTheFansOn(tAverage); //Should we turn the fans on??

  // Join together string
  String sqlTemperatures =  String(t1) + "','" +
                            String(t2) + "','" +
                            String(t3) + "','" +
                            String(t4) + "','" +
                            String(t5) + "','" +
                            String(t6) + "','" +
                            String(t7) + "','" +
                            String(t8) + "'" ;

  if (debugPrint) {DebugPrint(sqlTemperatures);}                     
  return sqlTemperatures;
}

/**
 * @brief Takes in an Array of Temperature values and returns average, disregarding -127 or 85 deg.
 *
 * @param tArray float array: Temperature values raw.
 * @return tAverage - Average temperature. 
 */
float CalculateAverageTemperature(float *tArray)
{
  float tAverage;
  int tAverageLength = 0;
  float tTotal = 0;
  for (int i = 0 ; i < 8; i++) // for length of std array =8. needs to be for length of array
    {
      if (tArray[i] != -127 && tArray[i] != 85) // Check the sensor came back with a reading. If it did, do the below. 
      {
        tTotal = tArray[i] + tTotal; // Add the current temp to the total temp to average
        tAverageLength ++; // Count the times we have been in the if loop
      }
    }

  tAverage = tTotal/tAverageLength ;  // Divide total sum of temperatures by the number of readings 
  return tAverage;
}

void ShouldWeTurnTheFansOn(float tAverage)
{
  unsigned long fanTimeOn = millis() - fanStartTime;
  String fanTimeOnSeconds = String(fanTimeOn/1000);
  DebugPrint("Time Fans are on:" + String(fanTimeOn));
  float fanTemperatureSetPoint = GetTemperatureSetPoint();
  if (fanTimeOn > fanTimeout) //if the fans have been on for more than 5mins e.g. enter this statement 
  {
    if (tAverage > fanTemperatureSetPoint) //if the average temperature is greater than the set point, turn the fans on.
    {
      SwitchFans(true);
      fanStartTime = millis(); //we have just turned the fans on. Reset the timer to 0
      DebugPrint("Turning on fans T = " + String(tAverage));
    }
    else //if the average temperature is less than the set point, turn the fans off.
    {
      fanStartTime = millis(); //we have just turned the fans off. Reset the timer to 0
      SwitchFans(false);
      DebugPrint("Turning off fans T = " + String(tAverage));
    }
  }
}

//Switches between temperature set point based upon pigs type selected. 
float GetTemperatureSetPoint()
{
  float fanTemperatureSetPoint;
  if (digitalRead(livestockSelectButton) == 1)
  {
    fanTemperatureSetPoint = largePigsTemperatureSetpoint;
  }
  else
  {
    fanTemperatureSetPoint = smallPigsTemperatureSetpoint;
  }
  return fanTemperatureSetPoint;
}

String GetTemperatureSetPointType()
{
  String fanTemperatureSetPointType;
  if (digitalRead(livestockSelectButton) == 1)
  {
    fanTemperatureSetPointType = "L";
  }
  else
  {
    fanTemperatureSetPointType = "S";
  }
  return fanTemperatureSetPointType;
}

void SwitchFans(bool truefalse)
{
  if (truefalse)
  {
    arduino::digitalWrite(fanOutput,HIGH);
    fanStatus = true;
    tftFansOnOff();
  }
  else
  {
    arduino::digitalWrite(fanOutput, LOW);
    fanStatus = false;
    tftFansOnOff();
  }
}

//Function to send older data. 
bool SQLPostOldData()
{

}

//Function to check if we need to send data.
//loops through csv file (of the correct day) and returns indexes of lines without a 200. works for the current line too. 
bool CheckForDataToSend()
{
  //plowmanSD.ReadDataWaitingToSend(uctTimeSD);
}

bool DeleteSentData()
{

}

//Requests GPS Data from FONA Module
bool getGPS() 
{
  DebugPrint("Getting GPS...");
  if (!gpsOn) 
  { //Check if GPS is actually on. We should never get here
    GPSsetup();
    DebugPrint("GPS Wasn't on!");
    return false;
  }

  // ****** GET GPS DATA FROM FONA MODULE ****** //
  char gpsBuffer[120];   //create buffer to store GPS raw data
  fona.getGPS(32, gpsBuffer, 120); //request GPS data
  DebugPrint(gpsBuffer);
  
  // ****** PROCESS DATA INTO PARTS ****** //
  //Skip GPS run status
  char *tok = strtok(gpsBuffer, ",");   
  if (!tok) { return false; }
  
  //Get fix status
  char *gpsFix = strtok(NULL,",");      
  if (!gpsFix) {  return false; }
  gpsFixint = atoi(gpsFix);   //Convert to integer
  
  //Get date
  char *uctime = strtok(NULL,",");      
  if (!uctime) { return false; }
  String uctTimeYear = String(uctime[0]) + String(uctime[1]) + String(uctime[2]) + String(uctime[3]);
  String uctTimeMonth = String(uctime[4]) + String(uctime[5]);
  String uctTimeDay = String(uctime[6]) + String(uctime[7]);
  String uctTimeTime = String(uctime[8]) + String(uctime[9]) + ":" + String(uctime[10]) + String(uctime[11]) + ":" + String(uctime[12]) + String(uctime[13]);
  uctTime = uctTimeDay + "/" + uctTimeMonth + '/' + uctTimeYear + "," + uctTimeTime; // Process date into readable format [D/M/Y HR:MIN:SEC]
  uctTimeSD = String(uctime[6]) + String(uctime[7]) + String(uctime[4]) + String(uctime[5]) + String(uctime[2]) + String(uctime[3]);
  sqlTime = uctTimeYear + "-" + uctTimeMonth + "-" + uctTimeDay + " " + uctTimeTime; // Process date into sql format [YYYY-MM-DD hh:mm:ss]

  //Get the latitude
  char *latp = strtok(NULL, ","); 
  if (!latp)
    { return false; }

  //Get the longitude
  char *longp = strtok(NULL, ",");
  if (!longp)
    { return false; }
  float lat = atof(latp);   //convert to float
  float lon = atof(longp);  //convert to float
  
  //TODO, get speed.

  // ****** FINISHED PROCESSING DATA ****** //

  if (gpsFixint == 0) 
  { 
    //If we had no fix, exit the function and try again later
    DebugPrint("No Fix, Retrying");
    tftGPS(false);
    return false;
  }
  tftGPS(true); // Else we have a fix, set GPS icon to green

  //Store data as different types depending on text message or SD write. 
  //TODO add switch case to POST to website.
  //TODO add switch case to Save to PICO hard drive 
  //Create Text Message
  String smsMessageStr =  String(uctTime + ","  + "http://www.google.com/maps/place/" +  String(lat,5) + "," + String(lon,5));
  strcpy(smsmessage, smsMessageStr.c_str());
  //Create SQL message
  sqlData = String( "'" + livestockBoxNo + "','" + sqlTime + "','" +  String(lat,5) + "','" + String(lon,5) + "','" );
  return true; //Function was successful   
}

void DebugPrint(String data)
{
  if (!newDebugSession)
  {
    if (!plowmanSD.DebugBoolPrint(data,uctTime))
    {
      tftSD(false);
      plowmanSD.initialiseSD(sdSelectPin);  
    }
    else
    {
      tftSD(true);
    }
  }
  tftPrint(data);
  Serial.println(data);
}

uint8_t CheckNetwork() 
{
  DebugPrint("Checking for network...");
  uint8_t n = fona.getNetworkStatus();
  switch (n) 
  {
    case 0 : 
    {
      DebugPrint("not registered..."); //Not Registered
      tftNetwork(0);
      break;
    }
    case 1 : 
    {
      DebugPrint("Registered - Home"); //Registered (Home)
      tftNetwork(1);
      break;
    }
    case 2 : 
    {
      DebugPrint("Not Registered - Searching"); //Not Registered (Searching)
      tftNetwork(2);
      break;
    }
    case 3 : 
    {
      DebugPrint("Denied"); //Denied
      tftNetwork(3);
      break;
    }
    case 4 : 
    {
      DebugPrint("Unknown");
      tftNetwork(4);
      break; //Unknown
    }
    case 5 : 
    {
      DebugPrint("Registered - Roaming");
      tftNetwork(5);
      break; //Registered (Roaming)
    }
  }
  return n;
}

void ReadBattery()
{
  // read the battery voltage and percentage
  DebugPrint("Reading Battery...");
  uint16_t vbat;
  if (! fona.getBattPercent(&vbat)) 
  {
    DebugPrint("Failed to read battery");
    fonaNoReply++;
    return;
  } 
  else 
  {
    DebugPrint(String(vbat));
    fonaNoReply = 0;
  }

  if (vbat < 40) 
  {
    DebugPrint("Battery less than 40%");
    tftBattery(true, String(vbat)+"%");
  }
  else 
  {
    tftBattery(false, String(vbat)+"%");
  }
}

void ReadSetpoints()
{
  String fanTimeoutString;
  String largePigsTempString;
  String smallPigsTempString;
  char *stopStr;

  largePigsTempString = plowmanSD.ReadSetPointFromFile("LPigs.txt");
  smallPigsTempString = plowmanSD.ReadSetPointFromFile("SPigs.txt");
  fanTimeoutString = plowmanSD.ReadSetPointFromFile("FTimeout.txt");

  char largePigsTempChar[largePigsTempString.length() + 1];
  strcpy(largePigsTempChar, largePigsTempString.c_str());
  largePigsTemperatureSetpoint = atof(largePigsTempChar);

  char smallPigsTempChar[smallPigsTempString.length() + 1];
  strcpy(smallPigsTempChar, smallPigsTempString.c_str());
  smallPigsTemperatureSetpoint = atof(smallPigsTempChar);

  char fanTimeoutChar[fanTimeoutString.length() + 1];
  strcpy(fanTimeoutChar,fanTimeoutString.c_str());
  fanTimeout = strtoul(fanTimeoutChar,&stopStr,10);
}

String ReadHTTPURL()
{
  String HTTPUrl;
  HTTPUrl =  plowmanSD.ReadSetPointFromFile("HTTP.txt");
  return HTTPUrl;
}

void WriteSetpoints(int setPointType, String value)
{
  String fileName;
  switch (setPointType)
  {
  case 1:
    fileName = "LPigs.txt";
    break;
  
  case 2:
    fileName = "SPigs.txt";
    break;

  case 3:
    fileName = "FTimeout.txt";
    break;
  default:
    Serial.println("error on writing setpoints");
    return;
  }
  plowmanSD.WriteSetPointToFile(fileName,value);
}

//we have come here to set the temperature values and fan timeouts. 
void SetpointSetup()
{
  //redraw the screen 
  const unsigned long setpointTimeout = 60000; // one minute timout.
  int position = 1; 
  unsigned long startTime = millis(); // get start of function time.
  unsigned long currentTime;  // initialise current time
  tft.fillScreen(ST77XX_BLACK);
  tftUpdateSetpointCursor(position,false);   // set highlight on position 1
  delay(1000);

  while (currentTime < setpointTimeout) //Loop an listen to button presses. Add a timeout?
  {
    if (digitalRead(buttonOK) == 0)
    {
      // check for exiting the function.
      if (position == 7)
      {
        tftCancelSetpointEditor("Settings Discarded");
        ReadSetpoints();
        tftUpdateTemperature();
        tftFansOnOff();
        return; // exit without saving
      } 
      if (position == 8 )
      {
        WriteSetpoints(1,String(largePigsTemperatureSetpoint));
        WriteSetpoints(2,String(smallPigsTemperatureSetpoint));
        WriteSetpoints(3,String(fanTimeout));
        delay(500);
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(0,60);
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_WHITE);
        tft.println("Settings Saved");
        tft.fillRect(50,72,100,3,ST77XX_GREEN);
        delay(2000);
        tft.fillScreen(ST77XX_BLACK);
        tftUpdateTemperature();
        tftFansOnOff();
        return;
      }
      else
      {
        tftUpdateSetpointCursor(position,true);
        delay(500);
      }
      startTime = millis(); // reset timout as we pressed something
    }
    // go to the next position
    if (digitalRead(buttonSelect) == 0)
    {
      if (position > 7)
      { 
        position = 1;
      }
      else
      {
        position ++;
      }
      tftUpdateSetpointCursor(position,false);
      delay(500);
      startTime = millis(); // reset timout as we pressed something
    }
    currentTime = millis() - startTime;
  }
  tftCancelSetpointEditor("Editor Timeout");
  ReadSetpoints();
  tftUpdateTemperature();
  tftFansOnOff();
}


void tomDelay()
{
 
  for (int i = 0; i < 15; i++) 
  {
    // delay between readings
    if (digitalRead(buttonSelect)==0 && digitalRead(buttonOK)==0)
    {
      SetpointSetup(); // we run the set point setup and try to edit things. 
    }
    delay(100); 
  }
}


//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//






/*
 * TFT SCREEN 
 */

// Redraw screen to Setpoint Editor.
void tftSetScreenSetpointEditor()
{
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.println("  Setpoint Editor");
  tft.fillRect(0,18,140,2,ST77XX_WHITE);
  tft.setCursor(10,25);
  tft.println("Large Pigs:");
  tft.setCursor(10,35);
  tft.println("  [-] " + String(largePigsTemperatureSetpoint) + " [+] ");
  tft.setCursor(10,55);
  tft.println("Small Pigs:");
  tft.setCursor(10,65);
  tft.println("  [-] " + String(smallPigsTemperatureSetpoint) + " [+] ");
  tft.setCursor(10,85);
  tft.println("Fan Timeout:");
  String fanTimeoutString = String(fanTimeout/60000); // convert to minutes
  tft.setCursor(10,95);
  tft.println("  [-] " + String(fanTimeoutString) + "mins [+] ");
  tft.fillRect(0,110,140,2,ST77XX_WHITE);
  tft.setCursor(10,115);
  tft.println("[Cancel]   [Submit]");
}

// Redraw screen to show cursor position in setpoint editor
// If @param updateValue flag is True, update the parameter under the cursor.
void tftUpdateSetpointCursor(int position, bool updateValue)
{
  int cursorX; 
  int cursorY;
  tft.fillScreen(ST77XX_BLACK); //Clear Screen
  switch (position)
  {
    case 1: // large pigs negative

      cursorX = 20;
      cursorY = 35;
      if (updateValue) { largePigsTemperatureSetpoint = largePigsTemperatureSetpoint - 0.5;}
      break;
    case 2: // large pigs positive   
      cursorX = 80;
      cursorY = 35;
      if (updateValue) { largePigsTemperatureSetpoint = largePigsTemperatureSetpoint + 0.5;}
      break;
    case 3: // small pigs negative
      cursorX = 20;
      cursorY = 65;
      if (updateValue) { smallPigsTemperatureSetpoint = smallPigsTemperatureSetpoint - 0.5;}
      break;
    case 4: // small pigs positive
      cursorX = 80;
      cursorY = 65;
      if (updateValue) { smallPigsTemperatureSetpoint = smallPigsTemperatureSetpoint + 0.5;}
      break;
    case 5: // fan negative
      cursorX = 20;
      cursorY = 95;
      if (updateValue) { fanTimeout = fanTimeout - 60000;}
      break;
    case 6: // fan positive
      cursorX = 80;
      cursorY = 95;
      if (updateValue) { fanTimeout = fanTimeout + 60000;}
      break;
    case 7: // cancel
    cursorX = 25;
    cursorY = 115;
    break;
    case 8: // submit
      cursorX = 90;
      cursorY = 115;
      break;
    default:
      break;
  }
  tft.fillRect(cursorX,cursorY+8,22,2,ST77XX_GREEN); //Draw Cursor
  tftSetScreenSetpointEditor(); //Use function to redraw screen
}

//Exit Screen for Setpoint Editor
void tftCancelSetpointEditor(String message)
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0,60);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(message);
  tft.fillRect(50,72,100,3,ST77XX_RED);
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);
}

//Set Screen for tft Print function. 
void tftSetScreen()
{
  tft.invertDisplay(true);
  tft.fillRect(0,110,tft.width(),(tft.height()-100),ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(true);
  tft.setTextSize(1);
  tft.setCursor(0, 110);
}

void tftPrint(String data)
{
  tftSetScreen();
  tft.println(data);
}


void tftGPS(bool onoff)
{
  int radius = 5;
  int xpos = tft.width()-radius-1;
  int ypos = 7;
  String text = "GPS";
  int textSize = 1;
    if (onoff)
  {
    tft.fillCircle(xpos,ypos,radius, ST77XX_GREEN);
  }
  else
  {
    tft.fillCircle(xpos,ypos,radius, ST77XX_RED);
  }  
}

void tftSSL(bool onoff)
{
  int radius = 5;
  int xpos = tft.width() - radius - 1 ;
  int ypos = 21;
  String text = "SSL";
  int textSize = 1;
    if (onoff)
  {
    tft.fillCircle(xpos,ypos,radius, ST77XX_GREEN);
  }
  else
  {
    tft.fillCircle(xpos,ypos,radius, ST77XX_RED);
  }  
}

void tftNetwork(int value)
{
  int xpos = 1;
  int ypos = 2;
  int xlength = 8;
  int ylength = 10;
  String text = "Network";
  uint16_t colour = ST77XX_RED;
  int radius = 5;
  switch (value)
  {
    case 0:
    {
    text = "No Signal";
    colour = ST77XX_RED;
    break;
    }
    case 1:
    {
    text = "Registered";
    colour = ST77XX_GREEN;
    break;
    }
    case 2:
    {
    text = "Searching";
    colour = ST77XX_ORANGE;
    break;
    }
    case 3:
    {
    text = "Denied";
    colour = ST77XX_RED;
    break;
    }
    case 4:
    {
    text = "Unknown";
    colour = ST77XX_RED;
    break;
    }
    case 5:
    {
    text = "Roaming";
    colour = ST77XX_GREEN;
    break;
    }
  }
  int textSize = 1;
  uint16_t xposText = xpos+xlength+5;
  uint16_t yposText = ypos+(ylength/2)-(7*textSize)/2;
  tft.fillRect(xposText,yposText-5,tft.width()-tft.width()/2,(7*textSize)+10,ST77XX_BLACK);
  tft.setCursor(xposText,yposText);
  tft.setTextSize(textSize);
  tft.println(text);
  tft.fillCircle((xpos+(xlength/2)),(ypos+(ylength/2)),radius, colour);
}

void tftSD(bool onoff)
{
  int xpos = 0;
  int ypos = 105;
  tft.fillRect(xpos,ypos,140,2,ST77XX_BLACK);
  if (onoff)
  {
    tft.fillRect(xpos,ypos,140,1,ST77XX_GREEN);
  }
  else
  {
    tft.fillRect(xpos,ypos,140,2,ST77XX_RED);
  }
}

void tftBattery(bool onoff, String batteryPercentage)
{
  int textSize = 1;
  int xpos = tft.width()-(7*batteryPercentage.length()*textSize)-15;
  int ypos = 4;
  int xlength = 7*batteryPercentage.length()*textSize+15;
  int radius = 5;
  
  tft.setCursor(xpos,ypos);
  tft.setTextSize(textSize);
  if (onoff)
  {
    tft.fillRoundRect(xpos-20,ypos-5,xlength,(7*textSize)+10,radius,ST77XX_YELLOW);
    tft.setTextColor(ST77XX_BLACK);
    tft.println(batteryPercentage);
  }
  else
  {
    tft.fillRoundRect(xpos-20,ypos-5,xlength,(7*textSize)+10,radius,ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.println(batteryPercentage);
  } 
tft.setTextColor(ST77XX_WHITE);
}

void tftGprsStatus(uint16_t gprsStatus)
{
  int xpos = 5;
  int ypos = 45;
  int textSize = 1;
  String text = "GPRS status : " + String(gprsStatus);
  int ylength = textSize*7;
  tft.setCursor(xpos,ypos);
  tft.fillRect(0,ypos,tft.width(),ylength,ST77XX_BLACK);
  tft.setTextSize(textSize);
  tft.setTextColor(ST77XX_WHITE);
  if (gprsStatus == 200)
  {
    tft.fillRect(0,ypos,tft.width(),ylength,ST77XX_GREEN);
  }
  tft.println(text);
}

void tftTemperatureAverage(float tAverage)
{
  int xpos = 5;
  int ypos = 55;
  int textSize = 1;
  String text = "Temperature : " + String(tAverage);
  int ylength = textSize*7;

  tft.setCursor(xpos,ypos);
  tft.fillRect(xpos,ypos,tft.width(),ylength,ST77XX_BLACK);
  tft.setTextSize(textSize);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(text);
}

void tftUpdateTemperature()
{
  float fanTemperatureSetPoint = GetTemperatureSetPoint();
  String fanTemperatureSetPointType = GetTemperatureSetPointType();
  int xpos = 5;
  int ypos = 65;
  int textSize = 1;
  String text = fanTemperatureSetPointType + " - Setpoint: " + String(fanTemperatureSetPoint);
  int ylength = textSize * 7 ;

  tft.setCursor(xpos,ypos);
  tft.fillRect(xpos,ypos,tft.width(),ylength,ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(text);
}

void tftUpdateIndividualTemperatures(int temperatureSensor)
{

  int xpos = 0;


}

void tftFansOnOff()
{
  String text;
  int xpos = 5;
  int ypos = 75;
  int textSize = 1;
  if (fanStatus)
  {
    text = "Fan Status  : ON";
  }
  else
  {
    text = "Fan Status  : OFF";
  }
  int ylength = textSize * 7 ;

  tft.setCursor(xpos,ypos);
  tft.fillRect(xpos,ypos,tft.width(),ylength,ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(text);
}


void tftHTTPAddress(String httpUrl)
{
  String text;
  int xpos = 5;
  int ypos = 85;
  int textSize = 1;
  text = "URL: " + httpUrl;
  int ylength = textSize * 7 ;

  tft.setCursor(xpos,ypos);
  tft.fillRect(xpos,ypos,tft.width(),ylength,ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(text);
}





/*
* TFT SCREEN 
*/




//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//
//_______________________________________________________________________________________________________________________________________________________________//





void setup() 
{

  arduino::pinMode(buttonSelect,INPUT_PULLUP);
  arduino::pinMode(buttonOK,INPUT_PULLUP);
  arduino::pinMode(livestockSelectButton,INPUT_PULLUP);
  arduino::pinMode(fanOutput,OUTPUT);
  arduino::pinMode(piAlivePin,OUTPUT);
  arduino::pinMode(piOnPin,OUTPUT);
  InitialiseScreen(); 
  newSession = true;
  //Serial.begin(115200);
  delay(500);

  plowmanSD.initialiseSD(sdSelectPin);
  delay(500);
  DebugPrint("SD Initialised");


// Check the saved values from the SD card. 
// Large Pigs Setpoint
// Small Pigs Setpoint
// Fan Timeout

  ReadSetpoints();
  tftUpdateTemperature();
  SwitchFans(false);


  InitialiseFona();

  arduino::digitalWrite(piOnPin,HIGH); 

}





void loop() 
{
  bool gpsFix;
  String sqlMessage;
  if (gpsOn) 
  { // Perform these things if the GPS is on.
    // REQUEST GPS DATA //
    gpsFix = getGPS();
    if (!gpsFix)
    {  // if false, we failed to get GPS data.
      DebugPrint("MAIN - Not logging GPS");
      sqlData = "'PBLTEST','2021-09-17 01:23:45','54.08750','-1.14486',' "; // "livestock box no, YYYY-MM-DD hh:mm:ss, lat, lon," 
      //  "'" + livestockBoxNo + "','" + sqlTime + "','" +  String(lat,5) + "','" + String(lon,5) + "','" 
      sqlMessage = ConstructSQLMessage();
    } 
    else 
    { 
      // We got the correct data back, log to the sd card.
      DebugPrint("Getting Temperature...");
      sqlMessage = ConstructSQLMessage();
      if(!plowmanSD.LogGPSInfo(sqlMessage,uctTimeSD)) // We write to the SD card with the SQL Message
      { 
        // We failed to write to the SD card. let's try and re-initialise it. 
        plowmanSD.initialiseSD(sdSelectPin);
      }
    }
  }
  else 
  {
    GPSsetup(); // Setup the GPS
    // We probable want to log sensor data even when the GPS is off, otherwise we will lose data. 
  }
  // Now Check network
  uint8_t n = CheckNetwork();
  if (n == 1 ||  n == 5)  // If we have a signal, we will send data to the server
  {
    CheckAndPostToServer(sqlMessage,gpsFix);
  }
  else
  {
    // Save the message to a new file if we failed to send it.
    // We should have some GPS data
    if (gpsFix)
    {
      plowmanSD.SaveDataWaitingToSend(sqlMessage, uctTimeSD);
      //plowmanSD.ReadDataWaitingToSend(uctTimeSD);
      //fona.sendSMS("07845300660", "Checking SIM");
    }
  }
  //   What else do we need to do every 10 seconds?
  ReadBattery();
  if (fonaNoReply > 10) 
  {
    // If we disconnected from the fona for some reason or the battery was very low, try to reinitialise.
    InitialiseFona();
  } 
  
  DebugPrint("Looping...");  
  tomDelay();
  tftUpdateTemperature();
  DebugPrint("Finish Looping...");
  if (piAlive) //Send a signal to the other pi to make sure we are alive
  { 
    arduino::digitalWrite(piAlivePin,HIGH); 
    piAlive = false; 
  }
  else 
  { 
    arduino::digitalWrite(piAlivePin,LOW); 
    piAlive = true; 
  }
}
