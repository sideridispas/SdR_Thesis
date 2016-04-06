//i2c Master(UNO)
#include <SD.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#define DATA_INT 3 //interrupt pin for slave data
#define SD_CS 10 //SD Card module Chip select
volatile int data_ready = HIGH; //interrupt flag for data ready from slave
SoftwareSerial xbee(5, 6); // RX, TX
String p1,p2,p3,p4,p5,p6;


void setup()
{
  Wire.begin();
  Serial.begin(9600);
  xbee.begin(115200);
  
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
    //Serial.println("==Before wait");
    waitforDATA();
    noInterrupts();
    data_ready = HIGH;
    interrupts();
    //Serial.println("==After wait");
    String temp = getPacket(1);
    temp = getPacket(2);
    temp = getPacket(3);
    temp = getPacket(4);
    temp = getPacket(5);
    temp = getPacket(6);
  }
  Serial.println("System Ready");

  //Writing the column titles
  File dataFile = SD.open("alone4.csv", FILE_WRITE);
  dataFile.print("Date,Time,I,V1,V2,P");
  for (int i=1;i<21;i++){
        dataFile.print(",temp");
        dataFile.print(i);
  }
  dataFile.println();
  dataFile.close();
  
  delay(100);
}

void loop()
{
  waitforDATA();
  noInterrupts();
  data_ready = HIGH;
  interrupts();

  unsigned long StartTime = millis();  //Get starting time

  File dataFile = SD.open("alone4.csv", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    p1 = getPacket(1);
    p2 = getPacket(2);
    p3 = getPacket(3);
    p4 = getPacket(4);
    p5 = getPacket(5);
    p6 = getPacket(6);

    //Get end time
  unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;
  Serial.print(ElapsedTime);
  Serial.println(" ms");

    p1 = p1.substring(0,p1.indexOf('e'));
    p2 = p2.substring(0,p2.indexOf('e'));
    p3 = p3.substring(0,p3.indexOf('e'));
    p4 = p4.substring(0,p4.indexOf('e'));
    p5 = p5.substring(0,p5.indexOf('e'));
    p6 = p6.substring(0,p6.indexOf('e'));
    
    dataFile.print(p1);
    dataFile.print(',');
    dataFile.print(p2);
    dataFile.print(',');
    dataFile.print(p3);
    dataFile.print(',');
    dataFile.print(p4);
    dataFile.print(',');
    dataFile.print(p5);
    dataFile.print(',');
    dataFile.println(p6);
    dataFile.close();

    CurrentTime = millis();
    ElapsedTime = CurrentTime - StartTime;
    Serial.print(ElapsedTime);
    Serial.println(" ms");
    
    // print to the serial port too:
    Serial.println(p1);
    Serial.println(p2);
    Serial.println(p3);
    Serial.println(p4);
    Serial.println(p5);
    Serial.println(p6);

    xbee.println(p1);
    xbee.println(p2);
    xbee.println(p3);
    xbee.println(p4);
    xbee.println(p5);
    xbee.println(p6);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file!");
  }
  delay(500);
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
  //delay(10);
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
