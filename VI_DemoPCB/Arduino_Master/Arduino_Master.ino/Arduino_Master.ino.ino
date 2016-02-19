//i2c Master(UNO)
#include <SD.h>
#include <Wire.h>

#define DATA_INT 3 //interrupt pin for slave data
#define SD_CS 10 //SD Card module Chip select
volatile int data_ready = HIGH; //interrupt flag for data ready from slave

String p1, p2,p3,p4,p5,p6;


void setup()
{
  Wire.begin();
  Serial.begin(9600);

  //set up interrupt for data ready waiting from slave
  attachInterrupt(digitalPinToInterrupt(DATA_INT), Data_Interrupt, FALLING);
  
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CS)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  Serial.print("System loading...");
  //throw first 5 datalines in order to get rid of corrupted data
  for(int i=0;i<5;i++){
    waitforDATA();
    noInterrupts();
    data_ready = HIGH;
    interrupts();
    String temp = getPacket(1);
    temp = getPacket(2);
    temp = getPacket(3);
    temp = getPacket(4);
    temp = getPacket(5);
    temp = getPacket(6);
  }
  Serial.println("System Ready");

  //Writing the column titles
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  dataFile.print("Date,Time,I,V1,V2,P");
  for (int i=1;i<21;i++){
        dataFile.print(",temp");
        dataFile.print(i);
  }
  dataFile.println();
  dataFile.close();
  
  //delay(100);
}

void loop()
{
  waitforDATA();
  noInterrupts();
  data_ready = HIGH;
  interrupts();

  unsigned long StartTime = millis();  //Get starting time

  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    p1 = getPacket(1).substring(0,getPacket(1).indexOf('e'));
    dataFile.print(p1);
    dataFile.print(',');
    p2 = getPacket(2).substring(0,getPacket(2).indexOf('e'));
    dataFile.print(p2);
    dataFile.print(',');
    p3 = getPacket(3).substring(0,getPacket(3).indexOf('e'));
    dataFile.print(p3);
    dataFile.print(',');
    p4 = getPacket(4).substring(0,getPacket(4).indexOf('e'));
    dataFile.print(p4);
    dataFile.print(',');
    p5 = getPacket(5).substring(0,getPacket(5).indexOf('e'));
    dataFile.print(p5);
    dataFile.print(',');
    p6 = getPacket(6).substring(0,getPacket(6).indexOf('e'));
    dataFile.println(p6);
    dataFile.close();
    
    // print to the serial port too:
    Serial.println(p1);
    Serial.println(p2);
    Serial.println(p3);
    Serial.println(p4);
    Serial.println(p5);
    Serial.println(p6);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file!");
  }
  
  //Get end time
  unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;
  Serial.print(ElapsedTime);
  Serial.println(" ms"); 
  
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
  if (n == 1){
    Wire.requestFrom(5,32);
  }else if (n == 2){
    Wire.requestFrom(5,32);
  }else{
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
  Serial.println(receivedValue);
  return 1;
}

void Data_Interrupt(){
  data_ready = LOW;  
}

void waitforDATA() {
  while (data_ready) continue;
}

String getPacket(int n){
  // SEND COMMAND FOR n PACKET
  Wire.beginTransmission(5);
  Wire.write(n); // n packet request
  delay(20);
  Wire.endTransmission();
  
  // GET RESPONSE FOR n PACKET
  String receivedValue = "";  
  int first_time = HIGH;
  if (n == 1){
    Wire.requestFrom(5,32);
  }else if (n == 2){
    Wire.requestFrom(5,32);
  }else{
    Wire.requestFrom(5,29);
  }
    
  while(Wire.available()){
    char c = Wire.read();
    if(first_time){
      if(c == 'N'){
        //Serial.println("Not ready");
        return "Not ready";
      }else{
        first_time = LOW;
      }
    }
    receivedValue =  receivedValue + c;
  }
  
  return(receivedValue);
}
