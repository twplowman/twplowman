#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>

#define ONE_WIRE_BUS 12
#define TEMPERATURE_PRECISION 14

float tempC;
 int n = 1;
int numberOfDevices; 
int i=0;

int change = 0;      int counter = 0;   
int len = 30;
    
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices 
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature. 
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address



void setup(void)
{
  
 sensors.begin();  // Start up the library

  
 numberOfDevices = sensors.getDeviceCount();   // Grab a count of devices on the wire

  
 

}

void printTemperature(DeviceAddress deviceAddress) // function to print the temperature for a device
{
  if ( i > (numberOfDevices - 1)) {i = 0;}

  tempC = sensors.getTempC(deviceAddress);
  Serial.println("Reading: ");

  Serial.println(" ");

}

void loop(void)
{ 
  sensors.begin();  // Start up the library

  
  numberOfDevices = sensors.getDeviceCount();   // Grab a count of devices on the wire
      

     Serial.print("Sensor");
     Serial.println(n);

  numberOfDevices = sensors.getDeviceCount();   // Grab a count of devices on the wire

  
  Serial.println("Scanning Sensors");  // locate devices on the bus


  Serial.println(numberOfDevices, DEC);
  Serial.println(" Sensor"); 
  Serial.println(n);
  delay(1000);
    sensors.requestTemperatures(); // Send the command to get temperatures
  
  for(i=0;i<numberOfDevices; i++) {  // Loop through each device, print out temperature data
     if(sensors.getAddress(tempDeviceAddress, i))
  {
    // Output the device ID
    // It responds almost immediately. Let's print out the data
    printTemperature(tempDeviceAddress); // Use a simple function to print out the data
  } 
  
  }
  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
    {
    String dataString = "";
    dataString += String("Sensor: ");
    dataString += String(n, DEC);
    dataString += String(" addr: ");

    Serial.println(dataString);
    printAddress(tempDeviceAddress);
    delay(2000);
    }
  }


  sensors.requestTemperatures(); // Send the command to get temperatures
  
  for(i=0;i<numberOfDevices; i++) {  // Loop through each device, print out temperature data
     if(sensors.getAddress(tempDeviceAddress, i))
	{
		// Output the device ID
		// It responds almost immediately. Let's print out the data
		printTemperature(tempDeviceAddress); // Use a simple function to print out the data
	} 
	
  }
    delay (100);  


}


// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (  i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    
       String dataString = "";

      dataString += String(deviceAddress[i], HEX);

    
        Serial.println(dataString);
     
       
  
    Serial.print(deviceAddress[i], HEX);
  }
}