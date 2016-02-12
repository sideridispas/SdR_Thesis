//i2c Master(UNO)
#include <Wire.h>

#define DATA_INT 3 //interrupt pin for slave data

volatile int data_ready = HIGH; //interrupt flag for data ready from slave
void setup()
{
  Wire.begin();
  Serial.begin(9600);

  //set up interrupt for data ready waiting from slave
  attachInterrupt(digitalPinToInterrupt(DATA_INT), Data_Interrupt, FALLING);

  delay(1000);
  waitforDATA(); //initial delay for stabilizing the communication
  noInterrupts();
  data_ready = HIGH;
  interrupts();
  delay(1000);
}

void loop()
{
  waitforDATA();
  noInterrupts();
  data_ready = HIGH;
  interrupts();
  
  while(!printPacket(1)){}
  //delay(10);
  while(!printPacket(3)){}
  //delay(10);
  while(!printPacket(2)){}
  Serial.println();
  
  delay(500);
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
    Wire.requestFrom(5,30);
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
  
  //Serial.print(n);
  //Serial.println(" packet received:");
  Serial.print(receivedValue);
  return 1;
}

void Data_Interrupt(){
  data_ready = LOW;  
}

void waitforDATA() {
  while (data_ready) continue;
}
