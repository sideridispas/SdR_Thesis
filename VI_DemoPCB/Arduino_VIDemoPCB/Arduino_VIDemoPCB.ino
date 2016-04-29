/********************************************************
*    VI_PCB with DS18B20 temperature sensors logging    *
*          Paschalis Sideridis - February 2016          *
*                                                       *
*  Conections:                                          *
*    VI_PCB: SCLK |  DIN | DOUT |  DRDY |  CS           *
*            SCK     MOSI  MISO    INT     I/O          *
*            13      11    12      2       7            *
*                                                       *
*    TEMP_1WIRE_BUS: PIN 4                              *
*                                                       *
*    RTC:   SQW | CLK | MISO |  MOSI |  SS              *                                      
*           3     13    12      11      8               *
*                                                       *
********************************************************/

/*-----( Import needed libraries )-----*/
#include "IC_Libs/ads12xx.h" //ADC ADS1256 libraries
#include "IC_Libs/OneWire/OneWire.h" //Temperature sensor DS18B20 libraries
#include "IC_Libs/dallas-temperature-control/DallasTemperature.h" //Temperature sensor DS18B20 libraries
#include "IC_Libs/ds3234_RTC/ds3234.h" //Real time clock DS3234 Library
#include <Wire.h> //I2C Library for Master-Slave communication

/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 4 //Pin for the 1-wire bus that temp sensors are using for data
#define ONE_WIRE_BUS_PIN2 5
#define ADC_CS 7 //Chip select for the ADS1256 ADC
#define ADC_DRDY 2 //Data Ready pin for the ADS1256 ADC
#define RTC_CS 8 //Chip select for the DS3234 RTC
#define RTC_SQW 3 //Pin for the ouput of the Square Wave pulse of RTC (1Hz measuring clocking)
#define DATA_READY_PIN 6 //Pin for outputting the flag "Data ready" (all datastrings) to be readen by the master arduino
#define LED_PIN A1 //LED for "working board" indication

/*-----( Declare objects )-----*/
// Setup a ads12xx object to communicate with the ADS1256 ADC
ads12xx ads1256(ADC_CS,ADC_DRDY);
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temp_sensors(&oneWire);
// Setup a ds3234 object
ds3234 rtc(RTC_CS);

/*-----( Declare Variables )-----*/
// DS18B20 Temperature sensors addresses
DeviceAddress Probe01 = { 0x28, 0x69, 0xBA, 0xAE, 0x07, 0x00, 0x00, 0x7F }; 
DeviceAddress Probe02 = { 0x28, 0xA8, 0xF3, 0xAF, 0x07, 0x00, 0x00, 0x42 };
DeviceAddress Probe03 = { 0x28, 0x85, 0xC0, 0xAF, 0x07, 0x00, 0x00, 0x74 };
DeviceAddress Probe04 = { 0x28, 0x39, 0xAF, 0xAF, 0x07, 0x00, 0x00, 0x70 };
DeviceAddress Probe05 = { 0x28, 0xD8, 0xC0, 0xAE, 0x07, 0x00, 0x00, 0x9F };
DeviceAddress Probe06 = { 0x28, 0xBE, 0x45, 0x73, 0x07, 0x00, 0x00, 0x6B };
DeviceAddress Probe07 = { 0x28, 0xAB, 0x5C, 0x73, 0x07, 0x00, 0x00, 0x54 };
DeviceAddress Probe08 = { 0x28, 0xCA, 0x68, 0x74, 0x07, 0x00, 0x00, 0xBD };
DeviceAddress Probe09 = { 0x28, 0xAA, 0xE3, 0x72, 0x07, 0x00, 0x00, 0xCC };
DeviceAddress Probe10 = { 0x28, 0xBA, 0x50, 0x73, 0x07, 0x00, 0x00, 0x19 };
DeviceAddress Probe11 = { 0x28, 0x81, 0x7F, 0x73, 0x07, 0x00, 0x00, 0x9B };
DeviceAddress Probe12 = { 0x28, 0x38, 0xD3, 0x73, 0x07, 0x00, 0x00, 0x18 };
DeviceAddress Probe13 = { 0x28, 0x3F, 0xD2, 0x73, 0x07, 0x00, 0x00, 0x50 };
DeviceAddress Probe14 = { 0x28, 0xE2, 0x67, 0x74, 0x07, 0x00, 0x00, 0xC5 };
DeviceAddress Probe15 = { 0x28, 0xE0, 0xF2, 0x73, 0x07, 0x00, 0x00, 0x48 };
DeviceAddress Probe16 = { 0x28, 0x55, 0x48, 0x74, 0x07, 0x00, 0x00, 0xD0 };
DeviceAddress Probe17 = { 0x28, 0x0A, 0x99, 0x72, 0x07, 0x00, 0x00, 0x40 };
DeviceAddress Probe18 = { 0x28, 0x89, 0xE6, 0x73, 0x07, 0x00, 0x00, 0x7E };
DeviceAddress Probe19 = { 0x28, 0xBA, 0x6B, 0x74, 0x07, 0x00, 0x00, 0x6B };
DeviceAddress Probe20 = { 0x28, 0x40, 0xB1, 0x73, 0x07, 0x00, 0x00, 0xB3 };


float V1, V2, I, P, tempC[20]; //data after conversion and calculations
String dataString1,dataString2,dataString3,dataString4,dataString5,dataString6; //Data strings used for the data transfer to master arduino through I2C bus
volatile int RTC_state = HIGH; //interrupt flag for RTC's 1Hz pulse
volatile int data_ready = LOW; //flag for i2c communication (data ready to send to master)
byte LastMasterCommand = 0; //id of data packet to transmit to master
int temp_res = 11; //temperature sensor' output data resolution (9-12 bits)


void setup() {
  Serial.begin(9600); //start serial port to show results
  ads1256.begin(); //begin init for ADC chip
  ads1256.reg_init();  //ADC's register initialisation
  temp_sensors.begin(); //initialize the Temperature measurement library
  temp_sensors.setWaitForConversion(LOW); //flag of temperature library (check DallasTemperature above setWaitForConversion)
  
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
  temp_sensors.setResolution(Probe08, temp_res);
  temp_sensors.setResolution(Probe09, temp_res);
  temp_sensors.setResolution(Probe10, temp_res);
  temp_sensors.setResolution(Probe11, temp_res);
  temp_sensors.setResolution(Probe12, temp_res);
  temp_sensors.setResolution(Probe13, temp_res);
  temp_sensors.setResolution(Probe14, temp_res);
  temp_sensors.setResolution(Probe15, temp_res);
  temp_sensors.setResolution(Probe16, temp_res);
  temp_sensors.setResolution(Probe17, temp_res);
  temp_sensors.setResolution(Probe18, temp_res);
  temp_sensors.setResolution(Probe19, temp_res);
  temp_sensors.setResolution(Probe20, temp_res);



  rtc.RTC_init();//initialize of RTC chip
  //Seting the timestamp [day(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)]
  //rtc.SetTimeDate(1,1,1,00,00,00); 

  //interrupt for waiting the 1Hz pulse of RTC
  attachInterrupt(digitalPinToInterrupt(RTC_SQW), RTC_Interrupt, FALLING);

  Wire.begin(5); //i2c bus initialization - slave address 5
  Wire.onReceive(receiveCommand); //first received data from master is the command (next datastring to transfer ID)
  Wire.onRequest(slavesRespond); //sending back the requested datastring

  pinMode(DATA_READY_PIN,OUTPUT); // "data ready" pin that triggers the interrupt on master side
  digitalWrite(DATA_READY_PIN, LOW); //initialy the pin is HIGH (inverted logic because of falling edge interrupt)

  pinMode(LED_PIN,OUTPUT); // LED indicator
  
  for (int i=0;i<5;i++){
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
  digitalWrite(LED_PIN, HIGH); //Turn on as start up indicator
  delay(400);
  digitalWrite(LED_PIN, LOW); //Turn on as start up indicator
}

void loop() {

  unsigned long StartTime = millis();  //Get starting time
  temp_sensors.requestTemperatures(); //Command all devices on bus to read temperature

  digitalWrite(LED_PIN, HIGH); //Turn on indicator
  
  //clearing the datastrings before storing the new data
  dataString1 = "";
  dataString2 = "";
  dataString3 = "";
  dataString4 = "";
  dataString5 = "";
  dataString6 = "";
  
  data_ready = LOW; //clear the "data ready" flag. It will be set when all datastrings are updated with the new data

  // VOLTAGE 1 MEASUREMENTS
  V1 = ads1256.getCalibratedData(B00100011, 1.4824, -0.0525, 0.999, -0.039); //inputs 2&3, a1=1.4824, b1=-0.0525, a2=0.999, b2=-0.039

  digitalWrite(LED_PIN, LOW); //Turn off indicator after some "random" time
  
  // VOLTAGE 2 MEASUREMENTS
  V2 = ads1256.getCalibratedData(B01000101, 1.5042, -0.1683, 0.9934, -0.0473); //inputs 4&5, a1=1.5042, b1=-0.1683, a2=0.9934, b2=-0.0473

  // CURRENT MEASUREMENTS
  I = ads1256.getCalibratedData(B00010000, 4.9424, 0.1816, 0.9994, 0.0205); //inputs 0&1, a1=4.9424, b1=0.1816, a2=0.9994, b2=0.0205
  
  // POWER CALCULATIONS
  P = V1 * I;

  

  // TEMPERATURE MEASUREMENTS
  delay(100); //wait for the temperature sensors' conversion (11bits waiting is 375ms - 190ms electrical measurements)
  
  tempC[0] = temp_sensors.printTemperature(Probe01);
  tempC[1] = temp_sensors.printTemperature(Probe02);
  tempC[2] = temp_sensors.printTemperature(Probe03);
  tempC[3] = temp_sensors.printTemperature(Probe04);
  tempC[4] = temp_sensors.printTemperature(Probe05);
  tempC[5] = temp_sensors.printTemperature(Probe06);
  tempC[6] = temp_sensors.printTemperature(Probe07);
  tempC[7] = temp_sensors.printTemperature(Probe08);
  tempC[8] = temp_sensors.printTemperature(Probe09);
  tempC[9] = temp_sensors.printTemperature(Probe10);
  tempC[10] = temp_sensors.printTemperature(Probe11);
  tempC[11] = temp_sensors.printTemperature(Probe12);
  tempC[12] = temp_sensors.printTemperature(Probe13);
  tempC[13] = temp_sensors.printTemperature(Probe14);
  tempC[14] = temp_sensors.printTemperature(Probe15);
  tempC[15] = temp_sensors.printTemperature(Probe16);
  tempC[16] = temp_sensors.printTemperature(Probe17);
  tempC[17] = temp_sensors.printTemperature(Probe18);
  tempC[18] = temp_sensors.printTemperature(Probe19);
  tempC[19] = temp_sensors.printTemperature(Probe20);

  //delay(220); //simulate the time reading sensors 8-20

  // DATASTRING FILLING
  dataString1.concat(rtc.ReadTimeDate());
  dataString1.concat(",");
  dataString1 = dataString1 + String(I,4);
  for (int i=dataString1.length();i<32;i++){
    dataString1.concat("e");
  }
  
  dataString2 = dataString2 + String(V1,4);
  dataString2.concat(",");
  dataString2 = dataString2 + String(V2,4);
  dataString2.concat(",");
  dataString2 = dataString2 + String(P,4);
  for (int i=dataString2.length();i<32;i++){
    dataString2.concat("e");
  }

  dataString3 = dataString3 + String(tempC[0],2);
  dataString3.concat(",");
  dataString3 = dataString3 + String(tempC[1],2);
  dataString3.concat(",");
  dataString3 = dataString3 + String(tempC[2],2);
  dataString3.concat(",");
  dataString3 = dataString3 + String(tempC[3],2);
  dataString3.concat(",");
  dataString3 = dataString3 + String(tempC[4],2);
  for (int i=dataString3.length();i<32;i++){
    dataString3.concat("e");
  }

  dataString4 = dataString4 + String(tempC[5],2);
  dataString4.concat(",");
  dataString4 = dataString4 + String(tempC[6],2);
  dataString4.concat(",");
  dataString4 = dataString4 + String(tempC[7],2);
  dataString4.concat(",");
  dataString4 = dataString4 + String(tempC[8],2);
  dataString4.concat(",");
  dataString4 = dataString4 + String(tempC[9],2);
  for (int i=dataString4.length();i<32;i++){
    dataString4.concat("e");
  }

  dataString5 = dataString5 + String(tempC[10],2);
  dataString5.concat(",");
  dataString5 = dataString5 + String(tempC[11],2);
  dataString5.concat(",");
  dataString5 = dataString5 + String(tempC[12],2);
  dataString5.concat(",");
  dataString5 = dataString5 + String(tempC[13],2);
  dataString5.concat(",");
  dataString5 = dataString5 + String(tempC[14],2);
  for (int i=dataString5.length();i<32;i++){
    dataString5.concat("e");
  }

  dataString6 = dataString6 + String(tempC[15],2);
  dataString6.concat(",");
  dataString6 = dataString6 + String(tempC[16],2);
  dataString6.concat(",");
  dataString6 = dataString6 + String(tempC[17],2);
  dataString6.concat(",");
  dataString6 = dataString6 + String(tempC[18],2);
  dataString6.concat(",");
  dataString6 = dataString6 + String(tempC[19],2);
  for (int i=dataString6.length();i<32;i++){
    dataString6.concat("e");
  }

  
  Serial.println(dataString1);
  Serial.println(dataString2);
  Serial.println(dataString3);
  Serial.println(dataString4);
  Serial.println(dataString5);
  Serial.println(dataString6);

  //Datastrings ready to be transfered to master
  digitalWrite(DATA_READY_PIN, HIGH); //falling edge trigger interrupt
  delay(20);
  digitalWrite(DATA_READY_PIN, LOW); //restore pin to high

  //FREE TIME: here we are just waiting for the second to be completed

  waitforRTC(); //Waiting for the 1Hz pulse to arrive
  noInterrupts();
  RTC_state = HIGH; //restoring the volatile interrupt flag
  interrupts();

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
    int str_len,d,m,y,h,mi,s;
            
    switch(LastMasterCommand){
      case 0:   // Set RTC time and date command
        Serial.print("\nSetting time... ");
        d = Wire.read();
        m = Wire.read();
        y = Wire.read();
        h = Wire.read();
        mi = Wire.read();
        s = Wire.read();
        rtc.SetTimeDate(d,m,y,h,mi,s);
        Serial.println("Time set\n");
      break;
      
      case 1:   // Return 1st packet
        str_len = dataString1.length() + 1; 
        char char_array1[str_len];
        dataString1.toCharArray(char_array1, str_len);
        Wire.write(char_array1);
      break;
  
      case 2:   // Return 2nd packet
        str_len = dataString2.length() + 1; 
        char char_array2[str_len];
        dataString2.toCharArray(char_array2, str_len);
        Wire.write(char_array2);
      break;

      case 3:   // Return 3rd packet
        str_len = dataString3.length() + 1; 
        char char_array3[str_len];
        dataString3.toCharArray(char_array3, str_len);
        Wire.write(char_array3);
      break;

      case 4:   // Return 4th packet
        str_len = dataString4.length() + 1; 
        char char_array4[str_len];
        dataString4.toCharArray(char_array4, str_len);
        Wire.write(char_array4);
      break;
  
      case 5:   // Return 5th packet
        str_len = dataString5.length() + 1; 
        char char_array5[str_len];
        dataString5.toCharArray(char_array5, str_len);
        Wire.write(char_array5);
      break;

      case 6:   // Return 6th packet
        str_len = dataString6.length() + 1; 
        char char_array6[str_len];
        dataString6.toCharArray(char_array6, str_len);
        Wire.write(char_array6);

        //Second LED pulse to know that there is I2C communication
        digitalWrite(LED_PIN, HIGH); //Turn on indicator
        delay(30);
        digitalWrite(LED_PIN, LOW); //Turn on indicator
      break;
    }
    LastMasterCommand = 0;
}

