/**************************************************
  VI_PCB with DS18B20 temperature sensors logging
  Paschalis Sideridis - February 2016 

  Conections:
    VI_PCB: SCLK |  DIN | DOUT |  DRDY |  CS
            SCK     MOSI  MISO    INT     I/O
            13      11    12      2       7

    TEMP_1WIRE_BUS: PIN 3
**************************************************/

/*-----( Import needed libraries )-----*/
#include "IC_Libs/ads12xx.h" //ADC ADS1256 libraries
#include "IC_Libs/OneWire/OneWire.h" //Temperature sensor DS18B20 libraries
#include "IC_Libs/dallas-temperature-control/DallasTemperature.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 3

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*-----( Declare Variables )-----*/
DeviceAddress Probe01 = { 0x28, 0x69, 0xBA, 0xAE, 0x07, 0x00, 0x00, 0x7F }; 
DeviceAddress Probe02 = { 0x28, 0xA8, 0xF3, 0xAF, 0x07, 0x00, 0x00, 0x42 };
DeviceAddress Probe03 = { 0x28, 0x85, 0xC0, 0xAF, 0x07, 0x00, 0x00, 0x74 };
DeviceAddress Probe04 = { 0x28, 0x39, 0xAF, 0xAF, 0x07, 0x00, 0x00, 0x70 };
DeviceAddress Probe05 = { 0x28, 0xD8, 0xC0, 0xAE, 0x07, 0x00, 0x00, 0x9F };

long data;
float f_data;
ads12xx ads1256(7,2); //CS:7, DRDY:2
int temp_res = 10; //temperature sensor resolution (9-12 bits)

int visual = 1; //processing app:1 - arduino serial monitor:0

void setup() {
  // start serial port to show results
  Serial.begin(9600);
  ads1256.begin();
  reg_init();  //register initialisation
  sensors.begin(); //Initialize the Temperature measurement library
  ads1256.SendCMD(SELFCAL); //self-calibration command
  delay(100); //wait the calibration

  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(Probe01, temp_res);
  sensors.setResolution(Probe02, temp_res);
  sensors.setResolution(Probe03, temp_res);
  sensors.setResolution(Probe04, temp_res);
  sensors.setResolution(Probe05, temp_res);
}

void loop() {

  f_data = 0;
  data = 0;
  
  /************** VOLTAGE 1 MEASUREMENTS **************/
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
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1386)/1.4742;
    f_data = -1*f_data;
  }

  if(visual){
    Serial.print(f_data,4);
    Serial.print(":");
  }
  else{
    Serial.print("V1:");
    Serial.print(f_data,4);
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
    f_data = (f_data+0.1161)/1.4968;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data+0.1161)/1.4968;
    f_data = -1*f_data;
  }
  
   if(visual){
      Serial.print(f_data,4);
      Serial.print(":");
    }
    else{
      Serial.print("V2:");
      Serial.print(f_data,4);
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
    f_data = (f_data-0.321)/4.9331;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    f_data = ((float)data/8388607.0)*78.0;
    f_data = (f_data-0.321)/4.9331;
    f_data = -1*f_data;
  }
  if(visual){
    Serial.print(f_data,4);
    Serial.print(":");
  }
  else{
    Serial.print("I:");
    Serial.println(f_data,4);
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
    Serial.println(" C");
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
  

  
  //delay(500);
}

/*-----( Declare User-written Functions )-----*/


void reg_init(){
  unsigned long reg;
  
  /***** REGISTER INITIALISATION ******/
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
