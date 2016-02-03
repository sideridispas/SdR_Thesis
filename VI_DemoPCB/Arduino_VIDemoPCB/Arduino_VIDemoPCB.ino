/********************************************************
*    VI_PCB with DS18B20 temperature sensors logging    *
*          Paschalis Sideridis - February 2016          *
*                                                       *
*  Conections:                                          *
*    VI_PCB: SCLK |  DIN | DOUT |  DRDY |  CS           *
*            SCK     MOSI  MISO    INT     I/O          *
*            13      11    12      2       7            *
*                                                       *
*    TEMP_1WIRE_BUS: PIN 3                              *
*                                                       *
*    RTC:   SQW | CLK | MISO |  MOSI |  SS              *                                      
*           3     13    12      11      8               *
*                                                       *
********************************************************/

/*-----( Import needed libraries )-----*/
#include "IC_Libs/ads12xx.h" //ADC ADS1256 libraries
#include "IC_Libs/OneWire/OneWire.h" //Temperature sensor DS18B20 libraries
#include "IC_Libs/dallas-temperature-control/DallasTemperature.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 4
#define ADC_CS 7
#define ADC_DRDY 2
#define RTC_CS 8
#define RTC_SQW 3

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);
// Setup a ads12xx object to communicate with the ADS1256 ADC
ads12xx ads1256(ADC_CS,ADC_DRDY);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*-----( Declare Variables )-----*/
DeviceAddress Probe01 = { 0x28, 0x69, 0xBA, 0xAE, 0x07, 0x00, 0x00, 0x7F }; 
DeviceAddress Probe02 = { 0x28, 0xA8, 0xF3, 0xAF, 0x07, 0x00, 0x00, 0x42 };
DeviceAddress Probe03 = { 0x28, 0x85, 0xC0, 0xAF, 0x07, 0x00, 0x00, 0x74 };
DeviceAddress Probe04 = { 0x28, 0x39, 0xAF, 0xAF, 0x07, 0x00, 0x00, 0x70 };
DeviceAddress Probe05 = { 0x28, 0xD8, 0xC0, 0xAE, 0x07, 0x00, 0x00, 0x9F };

long data;
float f_data, V1, V2, I, P;

volatile int RTC_state = HIGH;

int temp_res = 11; //temperature sensor resolution (9-12 bits)

int visual = 1; //processing app:1 - arduino serial monitor:0

void setup() {
  // start serial port to show results
  Serial.begin(9600);
  ads1256.begin();
  reg_init();  //ADC's register initialisation
  sensors.begin(); //Initialize the Temperature measurement library
  ads1256.SendCMD(SELFCAL); //self-calibration command
  delay(100); //wait the calibration

  // set the resolution of temp sensors
  sensors.setResolution(Probe01, temp_res);
  sensors.setResolution(Probe02, temp_res);
  sensors.setResolution(Probe03, temp_res);
  sensors.setResolution(Probe04, temp_res);
  sensors.setResolution(Probe05, temp_res);

  RTC_init();
  //day(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)
  SetTimeDate(3,2,16,15,17,02); 

  //interrupt for waiting the 1Hz pulse of RTC
  attachInterrupt(digitalPinToInterrupt(RTC_SQW), RTC_Interrupt, FALLING);
}

void loop() {
  
  /************** VOLTAGE 1 MEASUREMENTS **************/

  //Get starting time
  unsigned long StartTime = millis();

  ads1256.SetRegisterValue(MUX,B00100011); //AIN2 - AIN3
  delay(50);
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1386)/1.4742;
    V1 = (f_data+0.0105)/0.9998;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1386)/1.4742;
    f_data = (f_data+0.0105)/0.9998;
    V1 = -1*f_data;
  }

  if(visual){
    Serial.print(V1,4);
    Serial.print(":");
  }
  else{
    Serial.print("V1:");
    Serial.print(V1,4);
    Serial.print(" | ");
  }
  
  /************** VOLTAGE 2 MEASUREMENTS **************/
  ads1256.SetRegisterValue(MUX,B01000101); //AIN4 - AIN5
  delay(50);
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    f_data = ((float)data/8388607.0)*78.0;
    V2 = (f_data+0.1161)/1.4968;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1161)/1.4968;
    V2 = -1*f_data;
  }
  
   if(visual){
      Serial.print(V2,4);
      Serial.print(":");
    }
    else{
      Serial.print("V2:");
      Serial.print(V2,4);
      Serial.print(" | ");
    }

  /************** CURRENT MEASUREMENTS **************/
  ads1256.SetRegisterValue(MUX,B00010000); //AIN0 - AIN1
  delay(50);
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    f_data = ((float)data/8388607.0)*78.0;
    I = (f_data-0.321)/4.9331;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data-0.321)/4.9331;
    I = -1*f_data;
  }
  if(visual){
    Serial.print(I,4);
    Serial.print(":");
  }
  else{
    Serial.print("I:");
    Serial.print(I,4);
    Serial.print(" | ");
  }

  /************** POWER CALCULATIONS **************/
  P = V1 * I;
  if(visual){
    Serial.print(P,4);
    Serial.print(":");
  }
  else{
    Serial.print("P:");
    Serial.println(P,4);
  }
  
  /************** TEMPERATURE MEASUREMENTS **************/
  // Command all devices on bus to read temperature  
  sensors.requestTemperatures();  

  if(visual){
    printTemperature(Probe01);
    Serial.print(" C:");

    printTemperature(Probe02);
    Serial.print(" C:");

    printTemperature(Probe03);
    Serial.print(" C:");

    printTemperature(Probe04);
    Serial.print(" C:");

    printTemperature(Probe05);
    Serial.print(" C:");
  }
  else{
    Serial.print("Probe 01 temperature is:   ");
    printTemperature(Probe01);
    Serial.println();
  
    Serial.print("Probe 02 temperature is:   ");
    printTemperature(Probe02);
    Serial.println();
   
    Serial.print("Probe 03 temperature is:   ");
    printTemperature(Probe03);
    Serial.println();
     
    Serial.print("Probe 04 temperature is:   ");
    printTemperature(Probe04);
    Serial.println();
    
    Serial.print("Probe 05 temperature is:   ");
    printTemperature(Probe05);
    Serial.println();
  }

  waitforRTC();
  noInterrupts();
  RTC_state = HIGH;
  interrupts();

  //Get end time
  unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;
  Serial.print(ElapsedTime);
  Serial.println("ms");
}

/*-----( Declare User-written Functions )-----*/

void reg_init(){
  unsigned long reg;
  
  //STATUS register (default: 48)
  reg = ads1256.GetRegisterValue(STATUS);
  //reg = reg | B00000010; //BUFEN:1
  reg = reg & B11110001; //ORDER, ACAL:0
  ads1256.SetRegisterValue(STATUS,reg);
  
  //ADCON register (default: 32)
  ads1256.SetRegisterValue(ADCON,B00100111); //PGA:64
  
  //DRATE register
  ads1256.SetRegisterValue(DRATE,B01100011); //01100011
 
  //Registers' printing
/*  for(int i=0;i<5;i++){
    reg = ads1256.GetRegisterValue(i);
    Serial.print("Reg 0x0");Serial.print(i);Serial.print(": ");
    Serial.println(reg);
  }*/
}

void printTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00) 
   {
   Serial.print("ERR");
   } 
   else
   {
   //Serial.print("C: ");
   Serial.print(tempC);
   //Serial.print(" F: ");
   //Serial.print(DallasTemperature::toFahrenheit(tempC));
   }
}

int RTC_init(){ 
    pinMode(RTC_CS,OUTPUT); // chip select
    // start the SPI library:
    SPI.begin();
    SPI.setBitOrder(MSBFIRST); 
    SPI.setDataMode(SPI_MODE1); // both mode 1 & 3 should work 
    //set control register 
    digitalWrite(RTC_CS, LOW);  
    SPI.transfer(0x8E);
    SPI.transfer(0x60); //60= disable Osciallator and Battery SQ wave @1hz, temp compensation, Alarms disabled
    digitalWrite(RTC_CS, HIGH);
    delay(10);
}

int SetTimeDate(int d, int mo, int y, int h, int mi, int s){ 
  int TimeDate [7]={s,mi,h,0,d,mo,y};
  for(int i=0; i<=6;i++){
    if(i==3)
      i++;
    int b= TimeDate[i]/10;
    int a= TimeDate[i]-b*10;
    if(i==2){
      if (b==2)
        b=B00000010;
      else if (b==1)
        b=B00000001;
    } 
    TimeDate[i]= a+(b<<4);
      
    digitalWrite(RTC_CS, LOW);
    SPI.transfer(i+0x80); 
    SPI.transfer(TimeDate[i]);        
    digitalWrite(RTC_CS, HIGH);
  }
}

String ReadTimeDate(){
  String temp;
  int TimeDate [7]; //second,minute,hour,null,day,month,year    
  for(int i=0; i<=6;i++){
    if(i==3)
      i++;
    digitalWrite(RTC_CS, LOW);
    SPI.transfer(i+0x00); 
    unsigned int n = SPI.transfer(0x00);        
    digitalWrite(RTC_CS, HIGH);
    int a=n & B00001111;    
    if(i==2){ 
      int b=(n & B00110000)>>4; //24 hour mode
      if(b==B00000010)
        b=20;        
      else if(b==B00000001)
        b=10;
      TimeDate[i]=a+b;
    }
    else if(i==4){
      int b=(n & B00110000)>>4;
      TimeDate[i]=a+b*10;
    }
    else if(i==5){
      int b=(n & B00010000)>>4;
      TimeDate[i]=a+b*10;
    }
    else if(i==6){
      int b=(n & B11110000)>>4;
      TimeDate[i]=a+b*10;
    }
    else{ 
      int b=(n & B01110000)>>4;
      TimeDate[i]=a+b*10; 
      }
  }
  temp.concat(TimeDate[4]);
  temp.concat("/") ;
  temp.concat(TimeDate[5]);
  temp.concat("/") ;
  temp.concat(TimeDate[6]);
  temp.concat("     ") ;
  temp.concat(TimeDate[2]);
  temp.concat(":") ;
  temp.concat(TimeDate[1]);
  temp.concat(":") ;
  temp.concat(TimeDate[0]);
  return(temp);
}

void RTC_Interrupt(){
  RTC_state = LOW;  
}

void waitforRTC() {
  while (RTC_state) continue;
}
