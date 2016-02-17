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
#include "IC_Libs/ds3234_RTC/ds3234.h"
#include <Wire.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 4 //Pin for the 1-wire bus that temp sensors are using for data
#define ADC_CS 7 //Chip select for the ADS1256 ADC
#define ADC_DRDY 2 //Data Ready pin for the ADS1256 ADC
#define RTC_CS 8 //Chip select for the DS3234 RTC
#define RTC_SQW 3 //Pin for the ouput of the Square Wave pulse of RTC (1Hz measuring clocking)
#define DATA_READY_PIN 6 //Pin for outputting the flag "Data ready" (all datastrings) to be readen by the master arduino

/*-----( Declare objects )-----*/
// Setup a ads12xx object to communicate with the ADS1256 ADC
ads12xx ads1256(ADC_CS,ADC_DRDY);
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire1(ONE_WIRE_BUS_PIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temp_sensors(&oneWire1);
// Setup a ds3234 object
ds3234 rtc(RTC_CS);

/*-----( Declare Variables )-----*/
// DS18B20 Temperature sensors addresses
DeviceAddress Probe01 = { 0x28, 0x69, 0xBA, 0xAE, 0x07, 0x00, 0x00, 0x7F }; 
DeviceAddress Probe02 = { 0x28, 0xA8, 0xF3, 0xAF, 0x07, 0x00, 0x00, 0x42 };
DeviceAddress Probe03 = { 0x28, 0x85, 0xC0, 0xAF, 0x07, 0x00, 0x00, 0x74 };
DeviceAddress Probe04 = { 0x28, 0x39, 0xAF, 0xAF, 0x07, 0x00, 0x00, 0x70 };
DeviceAddress Probe05 = { 0x28, 0xD8, 0xC0, 0xAE, 0x07, 0x00, 0x00, 0x9F };
DeviceAddress Probe06 = { 0x28, 0x60, 0x06, 0x81, 0x07, 0x00, 0x00, 0x6D };
DeviceAddress Probe07 = { 0x28, 0x50, 0x2C, 0x81, 0x07, 0x00, 0x00, 0xC5 };

float f_data, V1, V2, I, P, tempC1, tempC2, tempC3, tempC4, tempC5, tempC6, tempC7; //f_data: float version of ADC data, rest: data after conversion and calculations
String dataString1, dataString2, dataString3, dataString4; //Data strings used for the data transfer to master arduino through I2C bus
volatile int RTC_state = HIGH; //interrupt flag for RTC's 1Hz pulse
volatile int data_ready = LOW; //flag for i2c communication (data ready to send to master)
byte LastMasterCommand = 0; //id of data packet to transmit to master
int temp_res = 11; //temperature sensor' output data resolution (9-12 bits)

void setup() {
  Serial.begin(9600); //start serial port to show results
  ads1256.begin(); //begin init for ADC chip
  ads1256.reg_init();  //ADC's register initialisation
  temp_sensors.begin(); //initialize the Temperature measurement library
  
  delay(200); //wait for voltage reference to be stable
  ads1256.SendCMD(SELFCAL); //ADC self-calibration command
  
  // set the resolution of temp sensors
  temp_sensors.setResolution(Probe01, temp_res);
  temp_sensors.setResolution(Probe02, temp_res);
  temp_sensors.setResolution(Probe03, temp_res);
  temp_sensors.setResolution(Probe04, temp_res);
  temp_sensors.setResolution(Probe05, temp_res);
  temp_sensors.setResolution(Probe06, temp_res);
  temp_sensors.setResolution(Probe07, temp_res);

  rtc.RTC_init();//initialize of RTC chip
  //Seting the timestamp [day(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)]
  rtc.SetTimeDate(16,2,16,12,29,00); 

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

  unsigned long StartTime = millis();  //Get starting time
  
  //clearing the datastrings before storing the new data
  dataString1 = "";
  dataString2 = "";
  dataString3 = "";
  dataString4 = "";
  
  data_ready = LOW; //clear the "data ready" flag. It will be set when all datastrings are updated with the new data

  // VOLTAGE 1 MEASUREMENTS
  V1 = ads1256.getCalibratedData(B00100011, 1.4824, -0.0525, 0.999, -0.039);
  
  // VOLTAGE 2 MEASUREMENTS
  V2 = ads1256.getCalibratedData(B01000101, 1.5042, -0.1683, 0.9934, -0.0473);

  // CURRENT MEASUREMENTS
  I = ads1256.getCalibratedData(B00010000, 4.9424, 0.1816, 0.9994, 0.0205);
 
  // POWER CALCULATIONS
  P = V1 * I;
  
  // TEMPERATURE MEASUREMENTS  
  temp_sensors.requestTemperatures(); //Command all devices on bus to read temperature

  tempC1 = temp_sensors.printTemperature(Probe01);
  tempC2 = temp_sensors.printTemperature(Probe02);
  tempC3 = temp_sensors.printTemperature(Probe03);
  tempC4 = temp_sensors.printTemperature(Probe04);
  tempC5 = temp_sensors.printTemperature(Probe05);
  tempC6 = temp_sensors.printTemperature(Probe06);
  tempC7 = temp_sensors.printTemperature(Probe07);

  // DATASTRING FILLING
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
  dataString3.concat(rtc.ReadTimeDate());
  dataString3.concat(",");

  dataString4.concat(",");
  dataString4 = dataString4 + String(tempC6,2);
  dataString4.concat(",");
  dataString4 = dataString4 + String(tempC7,2);
  dataString4.concat(",");

  Serial.print(dataString1);
  Serial.print(dataString3);
  Serial.print(dataString2);
  Serial.println(dataString4);

  //Datastrings ready to be transfered to master
  digitalWrite(DATA_READY_PIN, LOW); //falling edge trigger interrupt
  delay(10);
  digitalWrite(DATA_READY_PIN, HIGH); //restore pin to high

  //FREE TIME: here we are just waiting for the second to be completed

  /*waitforRTC(); //Waiting for the 1Hz pulse to arrive
  noInterrupts();
  RTC_state = HIGH; //restoring the volatile interrupt flag
  interrupts();*/

  //Get end time
  unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;
  Serial.print(ElapsedTime);
  Serial.println(" ms");
}

/*-----( Declare User-written Functions )-----*/
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

      case 3:   // Return 3rd packet
        Wire.write(char_array3);
      break;
    }
    LastMasterCommand = 0;
}

