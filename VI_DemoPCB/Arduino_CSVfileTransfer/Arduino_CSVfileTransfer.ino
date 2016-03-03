//i2c Master(UNO)
#include <SD.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#define SD_CS 10 //SD Card module Chip select

SoftwareSerial xbee(5, 6); // RX, TX

String a = "";
char temp;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  xbee.begin(115200);
  
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CS)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  delay(100);


  unsigned long StartTime = millis();  //Get starting time

  File dataFile = SD.open("5min.csv");
  // if the file is available, write to it:
  if (dataFile){
    Serial.println("file:");
  
    unsigned long CurrentTime = millis();
    unsigned long ElapsedTime = CurrentTime - StartTime;
    Serial.print(ElapsedTime);
    Serial.println(" ms");   
    
    // read from the file until there's nothing else in it:
    while (dataFile.available()){
      
      temp = dataFile.read();

      if(temp != '\n'){
        a = a + temp;        
      }else{
        xbee.println(a);
        a = "";
      }
    }
  }else {
    Serial.println("error opening file!");
  }

  unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;
  Serial.print(ElapsedTime);
  Serial.println(" ms");  
}

void loop()
{
  //hohohoho
}
