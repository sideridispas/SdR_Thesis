//i2c Master(UNO)
#include <Wire.h>

void setup()
{
  Wire.begin();
  Serial.begin(9600);
}

void loop()
{
  int first_time = HIGH;
  delay(200);  
  Wire.requestFrom(5,32);
  
  while(Wire.available())
  {
    char c = Wire.read();
    if(first_time){
      if(c == 'N'){
        //Serial.println("Not ready");
        return;
      }else{
        first_time = LOW;
      }
    }
    Serial.print(c);    
  }
  Serial.println();
}
