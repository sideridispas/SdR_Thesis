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
#include <Wire.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 4 //Pin for the 1-wire bus that temp sensors are using for data
#define ADC_CS 7 //Chip select for the ADS1256 ADC
#define ADC_DRDY 2 //Data Ready pin for the ADS1256 ADC
#define RTC_CS 8 //Chip select for the DS3234 RTC
#define RTC_SQW 3 //Pin for the ouput of the Square Wave pulse of RTC (1Hz measuring clocking)
#define DATA_READY_PIN 6 //Pin for outputting the flag "Data ready" (all datastrings) to be readen by the master arduino

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);
// Setup a ads12xx object to communicate with the ADS1256 ADC
ads12xx ads1256(ADC_CS,ADC_DRDY);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*-----( Declare Variables )-----*/
// DS18B20 Temperature sensors addresses
DeviceAddress Probe01 = { 0x28, 0x69, 0xBA, 0xAE, 0x07, 0x00, 0x00, 0x7F }; 
DeviceAddress Probe02 = { 0x28, 0xA8, 0xF3, 0xAF, 0x07, 0x00, 0x00, 0x42 };
DeviceAddress Probe03 = { 0x28, 0x85, 0xC0, 0xAF, 0x07, 0x00, 0x00, 0x74 };
DeviceAddress Probe04 = { 0x28, 0x39, 0xAF, 0xAF, 0x07, 0x00, 0x00, 0x70 };
DeviceAddress Probe05 = { 0x28, 0xD8, 0xC0, 0xAE, 0x07, 0x00, 0x00, 0x9F };

long data; //variable for storing the result (long) of the ADC convertions
float f_data, V1, V2, I, P, tempC1, tempC2, tempC3, tempC4, tempC5; //f_data: float version of ADC data, rest: readen data after conversion and calculations
String dataString1, dataString2, dataString3; //Data strings used for the data transfer to master arduino through I2C bus
volatile int RTC_state = HIGH; //interrupt flag for RTC's 1Hz pulse
volatile int data_ready = LOW; //flag for i2c communication (data ready to send to master)
byte LastMasterCommand = 0; //id of data packet to transmit to master
int temp_res = 11; //temperature sensor' output data resolution (9-12 bits)
int visual = 1; //processing app:1 - arduino serial monitor:0


void setup() {
  Serial.begin(9600); //start serial port to show results
  ads1256.begin(); //begin init for ADC chip
  reg_init();  //ADC's register initialisation
  sensors.begin(); //initialize the Temperature measurement library

  delay(200); //wait for voltage reference to be stable
  ads1256.SendCMD(SELFCAL); //ADC self-calibration command
  
  // set the resolution of temp sensors
  sensors.setResolution(Probe01, temp_res);
  sensors.setResolution(Probe02, temp_res);
  sensors.setResolution(Probe03, temp_res);
  sensors.setResolution(Probe04, temp_res);
  sensors.setResolution(Probe05, temp_res);

  RTC_init();//initialize of RTC chip
  //Seting the timestamp [day(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)]
  SetTimeDate(16,2,16,12,29,00); 

  //interrupt for waiting the 1Hz pulse of RTC
  attachInterrupt(digitalPinToInterrupt(RTC_SQW), RTC_Interrupt, FALLING);

  Wire.begin(5); //i2c bus initialization
  Wire.onReceive(receiveCommand); //first received data from master is the command (next datastring to transfer ID)
  Wire.onRequest(slavesRespond); //sending back the requested datastring

  pinMode(DATA_READY_PIN,OUTPUT); // data ready pin that triggers the interrupt on master side
  digitalWrite(DATA_READY_PIN, HIGH);
  
  delay(100); //wait the system stability
}

void loop() {
  
  //clearing the datastrings before storing the new data
  dataString1 = "";
  dataString2 = "";
  dataString3 = "";
  
  data_ready = LOW; //clear the "data ready" flag. It will be set when all datastrings are updated with the new data

  unsigned long StartTime = millis();  //Get starting time
  
  /************** VOLTAGE 1 MEASUREMENTS **************/
  ads1256.SetRegisterValue(MUX,B00100011); //set the MUX register to inputs AIN2 - AIN3 for V1
  delay(50);
  delayMicroseconds(10);
  ads1256.SendCMD(SYNC);
  delayMicroseconds(10);
  ads1256.SendCMD(WAKEUP);
  
  data =  ads1256.GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    /*f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1386)/1.4742;
    V1 = (f_data+0.0105)/0.9998;*/
    V1 = ((float)data/8388607.0)*78.0;
    V1 = (V1+0.0525)/1.4824;
    V1 = (V1+0.039)/0.999;
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
    /*f_data = ((float)data/8388607.0)*78.0;
    V2 = (f_data+0.1161)/1.4968;*/
    V2 = ((float)data/8388607.0)*78.0;
    V2 = (V2 + 0.1683)/1.5042;
    V2 = (V2 -0.0473)/0.9934;
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
    /*f_data = ((float)data/8388607.0)*78.0;
    I = (f_data-0.321)/4.9331;*/
    I = ((float)data/8388607.0)*78.0;
    I = (I-0.1816)/4.9424;
    I = (I-0.0205)/0.9994;
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
    tempC1 = printTemperature(Probe01);
    Serial.print(tempC1);
    Serial.print(" C:");

    tempC2 = printTemperature(Probe02);
    Serial.print(tempC2);
    Serial.print(" C:");

    tempC3 = printTemperature(Probe03);
    Serial.print(tempC3);
    Serial.print(" C:");

    tempC4 = printTemperature(Probe04);
    Serial.print(tempC4);
    Serial.print(" C:");

    tempC5 = printTemperature(Probe05);
    Serial.print(tempC5);
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


  /*--------( prepare the dataStrings )---------*/
  dataString1 = dataString1 + String(V1,4);
  dataString1.concat(",");
  dataString1 = dataString1 + String(V2,4);
  dataString1.concat(",");
  dataString1 = dataString1 + String(I,4);
  dataString1.concat(",");

  dataString2.concat(",");
  dataString2 = dataString2 + String(tempC1,2);
  dataString2.concat(",");
  dataString2 = dataString2 + String(tempC2,2);
  dataString2.concat(",");
  dataString2 = dataString2 + String(tempC3,2);
  dataString2.concat(",");
  dataString2 = dataString2 + String(tempC4,2);
  dataString2.concat(",");
  dataString2 = dataString2 + String(tempC5,2);

  dataString3.concat(",");
  dataString3 = dataString3 + String(P,4);
  dataString3.concat(",");
  dataString3.concat(ReadTimeDate());
  dataString3.concat(",");
  Serial.println(ReadTimeDate());

  
  //now is time for data sending
  digitalWrite(DATA_READY_PIN, LOW);
  delay(10);
  digitalWrite(DATA_READY_PIN, HIGH);
  
  waitforRTC();
  noInterrupts();
  RTC_state = HIGH;
  interrupts();

  //Get end time
  unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;
  Serial.print(ElapsedTime);
  Serial.print("ms:");


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

float printTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00) 
   {
   return(99.99);
   } 
   else
   {
   return(tempC);
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
    SPI.transfer(B01100000); //60= disable Osciallator and Battery SQ wave @1hz, temp compensation, Alarms disabled
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
  temp.concat(" ") ;
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

void receiveCommand(int howMany){
  LastMasterCommand = Wire.read(); // 1 byte (maximum 256 commands)
}

void slavesRespond(){
    int str_len = dataString1.length() + 1; 
    char char_array1[str_len];
    dataString1.toCharArray(char_array1, str_len);

    str_len = dataString2.length() + 1; 
    char char_array2[str_len];
    dataString2.toCharArray(char_array2, str_len);

    str_len = dataString3.length() + 1; 
    char char_array3[str_len];
    dataString3.toCharArray(char_array3, str_len);
    
    switch(LastMasterCommand){
      case 0:   // No new command was received
        Wire.write("S"); //Stay still
      break;
      
      case 1:   // Return 1st packet
        Wire.write(char_array1);
      break;
  
      case 2:   // Return 2nd packet
        Wire.write(char_array2);
      break;

      case 3:   // Return 2nd packet
        Wire.write(char_array3);
      break;
    }
    LastMasterCommand = 0;
}
