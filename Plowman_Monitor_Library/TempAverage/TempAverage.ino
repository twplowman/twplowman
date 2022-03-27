

//Gets all temperatures
String SQLGetTemperatures() //TODO
{
  
  // Search for temperature sensors on specified pins
  float t1 = 14.3;
  float t2 = -127;
  float t3 = 18.7;
  float t4 = 15;
  float t5 = -127;
  float t6 = 16.34;
  float t7 = 13.2;
  float t8 = 13.2;

  float tArray[8] = {t1,t2,t3,t4,t5,t6,t7,t8};
  float tAverage = GetAverageTemperature(tArray);

  tftTemperatureAverage(tAverage);

  String sqlTemperatures =  String(t1) + "','" +
                            String(t2) + "','" +
                            String(t3) + "','" +
                            String(t4) + "','" +
                            String(t5) + "','" +
                            String(t6) + "','" +
                            String(t7) + "','" +
                            String(t8) + "'" ;                   
  return sqlTemperatures;
}




float GetAverageTemperature(float *tArray)
{

float tAverage;
int tAverageLength = 0;
int tTotal = 0;

for (int i = 0 ; i <= 8, i++;)
  {

    Serial.println(String(tArray[i]));
    if (tArray[i] != -127) // Check the sensor came back with a reading. If it did, do the below. 
    {

      
      tTotal = tArray[i] + tTotal; // Add the current temp to the total temp to average
      tAverageLength ++; // Count the times we have been in the if loop
      Serial.println(String(tTotal));    
    }

  }

tAverage = tTotal/tAverageLength ;  // Divide total sum of temperatures by the number of readings 

return tAverage;

}

void setup() {
  // put your setup code here, to run once:

SQLGetTemperatures()

}

void loop() {
  // put your main code here, to run repeatedly:

}
