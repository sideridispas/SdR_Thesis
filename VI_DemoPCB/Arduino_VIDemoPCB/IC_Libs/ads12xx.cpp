#include "ads12xx.h"

volatile int DRDY_state = HIGH;
volatile int *IState = &DRDY_state;

// Waits untill DRDY Pin is falling (see Interrupt setup). 
// Some commands like WREG, RREG need the DRDY to be low.
void waitforDRDY() {
	while (DRDY_state) continue;
}

//Interrupt function
void DRDY_Interuppt() {
	DRDY_state = LOW;
}

void ads12xx::begin(){
	pinMode(_CS, OUTPUT);              // set the slaveSelectPin as an output:
	digitalWrite(_CS, HIGH);            // CS HIGH = no select
	pinMode(_DRDY, INPUT);             // DRDY read

	delay(500);
	SPI.begin();

	attachInterrupt(digitalPinToInterrupt(_DRDY), DRDY_Interuppt, FALLING); //Interrupt setup for DRDY detection

	delay(500);
}

ads12xx::ads12xx() {} //default constructor - Paschalis

// ads12xx setup
ads12xx::ads12xx(const int CS,const int DRDY) {
	_CS = CS;
	_DRDY = DRDY;
}


// function to get a 3byte conversion result from the adc
long ads12xx::GetConversion() {
	int32_t regData;
	waitforDRDY(); // Wait until DRDY is LOW
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); 
	digitalWrite(_CS, LOW); //Pull SS Low to Enable Communications with ADS1247
	delayMicroseconds(10); // RD: Wait 25ns for ADC12xx to get ready
	SPI.transfer(RDATA); //Issue RDATA
	delayMicroseconds(10);
	regData |= SPI.transfer(NOP);
	delayMicroseconds(10);//aaaaaaaaaaa
	regData <<= 8;
	regData |= SPI.transfer(NOP);
	delayMicroseconds(10);//aaaaaaaaaaa
	regData <<= 8;
	regData |= SPI.transfer(NOP);
	delayMicroseconds(10);
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
	noInterrupts();
	*IState = HIGH;
	interrupts();
	return regData;
}


// function to write a register value to the adc
// argumen: adress for the register to write into, value to write
void ads12xx::SetRegisterValue(uint8_t regAdress, uint8_t regValue) {

	uint8_t regValuePre = ads12xx::GetRegisterValue(regAdress);
	if (regValue != regValuePre) {
		delayMicroseconds(10);
		//Serial.println("@SetRegisterValue: wait for interrupt"); //debug_me
		waitforDRDY();
		//Serial.println("@SetRegisterValue: interrupt happened"); //debug_me
		SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with SPI_SPEED, MSB first, SPI Mode1
		digitalWrite(_CS, LOW);
		delayMicroseconds(10);
		SPI.transfer(WREG | regAdress); // send 1st command byte, address of the register
		SPI.transfer(0x00);		// send 2nd command byte, write only one register
		SPI.transfer(regValue);         // write data (1 Byte) for the register
		delayMicroseconds(10);
		digitalWrite(_CS, HIGH);
		if (regValue != ads12xx::GetRegisterValue(regAdress)) {   //Check if write was succesfull
			Serial.print("Write to Register 0x");
			Serial.print(regAdress, HEX);
			Serial.println(" failed!");
		}
		else
		SPI.endTransaction();
	}

}


//function to read a register value from the adc
//argument: adress for the register to read
unsigned long ads12xx::GetRegisterValue(uint8_t regAdress) {
	waitforDRDY();
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with 4Mhz clock, MSB first, SPI Mode0
	uint8_t bufr;
	digitalWrite(_CS, LOW);
	delayMicroseconds(10);
	SPI.transfer(RREG | regAdress); // send 1st command byte, address of the register
	SPI.transfer(0x00);			// send 2nd command byte, read only one register
	delayMicroseconds(10);
	bufr = SPI.transfer(NOP);	// read data of the register
	delayMicroseconds(10);
	digitalWrite(_CS, HIGH);
	return bufr;
	SPI.endTransaction();	
}

/*
Sends a Command to the ADC
Like SELFCAL, GAIN, SYNC, WAKEUP
*/
void ads12xx::SendCMD(uint8_t cmd) {
	waitforDRDY();
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with 4Mhz clock, MSB first, SPI Mode0
	digitalWrite(_CS, LOW);
	delayMicroseconds(10);
	SPI.transfer(cmd); 
	delayMicroseconds(10);
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
}


// function to reset the adc
void ads12xx::Reset(const int RESET_PIN) {

	//Pulse reset line.
	pinMode(RESET_PIN, OUTPUT);
	digitalWrite(RESET_PIN, HIGH);
	digitalWrite(RESET_PIN, LOW);
	delayMicroseconds(100);
	digitalWrite(RESET_PIN, HIGH); //Reset line must be high bevo continue
	delay(1); //RESET high to SPI communication start
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with  clock, MSB first, SPI Mode1
	digitalWrite(_CS, LOW);
	delayMicroseconds(10);
	//SPI.transfer(RESET); //Reset
	//delay(2); //Minimum 0.6ms required for Reset to finish.
	SPI.transfer(SDATAC); //Issue SDATAC
	delayMicroseconds(100);
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
}

float ads12xx::getCalibratedData(int mux_value, float a1, float b1, float a2, float b2){
  //returns the twice calibrated data of the conversion based on the trend line of raw data compared to DMM (y = ax + b)
  
  long data; //variable for storing the result (long) of the ADC convertions
  float Cal_data; //calibrated data variable to be returned in the end
  
  ads12xx::SetRegisterValue(MUX,mux_value); //set the MUX register to corresponding inputs
  delay(50);
  ads12xx::SendCMD(SYNC);
  delayMicroseconds(10);
  ads12xx::SendCMD(WAKEUP);
  
  data =  ads12xx::GetConversion();
  data = constrain(data, 0, 16777215);
  
  if((data >= 0) && (data <= 8388607)){
    //positive
    Cal_data = ((float)data/8388607.0)*78.0;
    Cal_data = (Cal_data-b1)/a1;
    Cal_data = (Cal_data-b2)/a2;
  }
  else if((data > 8388607) && (data <= 16777215)){
    //negative
    bitClear(data, 23);
    data = 8388607 - data;
    Cal_data = ((float)data/8388607.0)*78.0;
    Cal_data = (Cal_data-b1)/a1;
    Cal_data = (Cal_data-b2)/a2;
    Cal_data = -1*Cal_data;
  }
  return Cal_data;  
}

void ads12xx::reg_init(){
  unsigned long reg;
  
  //STATUS register (default: 48)
  reg = ads12xx::GetRegisterValue(STATUS);
  //reg = reg | B00000010; //BUFEN:1
  reg = reg & B11110001; //ORDER, ACAL:0
  ads12xx::SetRegisterValue(STATUS,reg);
  
  //ADCON register (default: 32)
  ads12xx::SetRegisterValue(ADCON,B00100111); //PGA:64
  
  //DRATE register
  ads12xx::SetRegisterValue(DRATE,B01100011); //01100011
 
  //Registers' printing
/*  for(int i=0;i<5;i++){
    reg = ads1256.GetRegisterValue(i);
    Serial.print("Reg 0x0");Serial.print(i);Serial.print(": ");
    Serial.println(reg);
  }*/
}