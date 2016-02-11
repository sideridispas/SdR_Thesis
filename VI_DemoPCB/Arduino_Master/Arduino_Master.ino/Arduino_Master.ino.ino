//i2c Master(UNO)
#include <Wire.h>

void setup()
{
  Wire.begin();
  Serial.begin(9600);
}

void loop()
{
  Serial.println("--------------------------");
  while(!printPacket(3)){}
  delay(20);
  while(!printPacket(1)){}
  delay(20);
  while(!printPacket(2)){}

  
  delay(100);
}

int printPacket(int n){
  // SEND COMMAND FOR n PACKET
  Wire.beginTransmission(5);
  Wire.write(n); // n packet request
  delay(20);
  Wire.endTransmission();

  
  // GET RESPONSE FOR n PACKET
  String receivedValue = "";  
  int first_time = HIGH;
  if(n == 1){
    Wire.requestFrom(5,30);
  }else if(n == 2){
    Wire.requestFrom(5,29);
  }else if(n == 3){
    Wire.requestFrom(5,29);
  }
    
  while(Wire.available()){
    char c = Wire.read();
    if(first_time){
      if(c == 'N'){
        //Serial.println("Not ready");
        return 0;
      }else{
        first_time = LOW;
      }
    }
    receivedValue =  receivedValue + c;
  }
  
  Serial.print(n);
  Serial.println(" packet received:");
  Serial.println(receivedValue);
  return 1;
}

