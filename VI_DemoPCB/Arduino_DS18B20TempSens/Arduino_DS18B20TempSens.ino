/* YourDuino Multiple DS18B20 Temperature Sensors on 1 wire
  Connections:
  DS18B20 Pinout (Left to Right, pins down, flat side toward you)
  - Left   = Ground
  - Center = Signal (Pin 2):  (with 3.3K to 4.7K resistor to +5 or 3.3 )
  - Right  = +5 or +3.3 V

   Questions: terry@yourduino.com 
   V1.01  01/17/2013 ...based on examples from Rik Kretzinger
   
/*-----( Import needed libraries )-----*/
#include "IC_Libs/OneWire/OneWire.h"
#include "IC_Libs/dallas-temperature-control/DallasTemperature.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 2

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*-----( Declare Variables )-----*/
// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress Probe01 = { 0x28, 0x69, 0xBA, 0xAE, 0x07, 0x00, 0x00, 0x7F }; 
DeviceAddress Probe02 = { 0x28, 0xA8, 0xF3, 0xAF, 0x07, 0x00, 0x00, 0x42 };
DeviceAddress Probe03 = { 0x28, 0x85, 0xC0, 0xAF, 0x07, 0x00, 0x00, 0x74 };
DeviceAddress Probe04 = { 0x28, 0x39, 0xAF, 0xAF, 0x07, 0x00, 0x00, 0x70 };
DeviceAddress Probe05 = { 0x28, 0xD8, 0xC0, 0xAE, 0x07, 0x00, 0x00, 0x9F };


void setup()   /****** SETUP: RUNS ONCE ******/
{
  // start serial port to show results
  Serial.begin(9600);
  Serial.print("Initializing Temperature Control Library Version ");
  Serial.println(DALLASTEMPLIBVERSION);
  
  // Initialize the Temperature measurement library
  sensors.begin();
  
  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(Probe01, 12);
  sensors.setResolution(Probe02, 12);
  sensors.setResolution(Probe03, 12);
  sensors.setResolution(Probe04, 12);
  sensors.setResolution(Probe05, 12);

}//--(end setup )---

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  //delay(1000);
  Serial.println();
  Serial.print("Number of Devices found on bus = ");  
  Serial.println(sensors.getDeviceCount());   
  Serial.print("Getting temperatures... ");  
  Serial.println();   
  
  // Command all devices on bus to read temperature  
  sensors.requestTemperatures();  
  
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
   
  
}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/
void printTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00) 
   {
   Serial.print("Error getting temperature  ");
   } 
   else
   {
   Serial.print("C: ");
   Serial.print(tempC);
   //Serial.print(" F: ");
   //Serial.print(DallasTemperature::toFahrenheit(tempC));
   }
}// End printTemperature
//*********( THE END )***********

