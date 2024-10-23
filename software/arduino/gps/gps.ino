#include <SoftwareSerial.h>
 
SoftwareSerial GPS(4,3); // RX, TX
 
void setup()  
{
  Serial.begin(115200);
 
  while (!Serial) { }
 
  GPS.begin(9600);
}
 
void loop()
{
  if (GPS.available())
  {
    Serial.write(GPS.read());
  }
}