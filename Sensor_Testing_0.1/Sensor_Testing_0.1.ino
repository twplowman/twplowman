///TPLOWMAN ADAFRUIT SIM808 GPS DATA PROGRAM///
///VPico_2.0.0__________________________220422///
String versionNumber = "pico v2.0.0";
///info_________________________________info///
///__Version Developed whilst at Huby____///
/*

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

//SETUP **SD/SCREEN MODULE**
File myFile;
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST); //(larger screen)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); //(small screen)

//SETUP **TEMP SENSORS**
#define busWire1 16 
#define busWire2 17 
#define busWire3 18
#define busWire4 19
#define busWire5 20
#define busWire6 21
#define busWire7 22
#define busWire8 26 

bool newSession; 
bool newDebugSession;
String uctTimeSD = "00000";
String uctTime = "XX/XX/XXXX,XX:XX:XX"; 
String sqlTime = "YYYY-MM-DD hh:mm:ss";

/**
 * @brief Initialise Adafruit TFT 2" Screen
 * Run a quick cycle to visually check it works. 
 */
void InitialiseScreen()
{
  //tft.init(240, 320); // Init ST7789 320x240
  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  delay(500);
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(3);
}

void DebugPrint(String data)
{
  if (!newDebugSession)
  {
    if (!plowmanSD.DebugBoolPrint(data,uctTime))
    {
     
      plowmanSD.initialiseSD(sdSelectPin);  
    }
    else
    {

    }
  }
  tftPrint(data);
  Serial.println(data);
}

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

//Set Screen for tft Print function. 
void tftSetScreen()
{
  tft.invertDisplay(true);
  tft.fillRect(0,180,tft.width(),(tft.height()-30),ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(true);
  tft.setTextSize(1);
  tft.setCursor(0, 180);
}

void tftPrint(String data)
{
  tftSetScreen();
  tft.println(data);
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


void setup() 
{

  InitialiseScreen(); 
  delay(500);

  plowmanSD.initialiseSD(sdSelectPin);
  delay(500);

}

void loop()

{
    SQLGetTemperatures(false);
    delay(1000);
}