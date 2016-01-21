#include "ads12xx.h"

volatile int DRDY_state = HIGH;
volatile int *IState = &DRDY_state;

// Waits untill DRDY Pin is falling (see Interrupt setup). 
// Some commands like WREG, RREG need the DRDY to be low.
void waitforDRDY() {
	//Serial.println("Waiting for DRDY interrupt..."); //debug_me
	//Serial.print("@waitforDRDY | DRDY_state:"); //debug_me
	//Serial.println(DRDY_state); //debug_me
	while (DRDY_state) continue;
	//Serial.println("Interrupt happened!!"); //debug_me
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

	//Message of succesful object creation - Paschalis
	/*Serial.println("ADS12xx begin() finished");
	Serial.print("CS:");
	Serial.print(_CS);
	Serial.print(", DRDY:");
	Serial.println(_DRDY);*/

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

	//Serial.print("GetConversion 1 | DRDY_state:"); //debug_me
	//Serial.println(DRDY_state); //debug_me
	
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
	
	//Serial.print("GetConversion 2 | DRDY_state:"); //debug_me
	//Serial.println(DRDY_state); //debug_me
	
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
	//Serial.println("@GetRegisterValue: wait for interrupt"); //debug_me
	//Serial.print("\n@GetRegisterValue_1 | DRDY_state:"); //debug_me
	//Serial.println(DRDY_state); //debug_me
	waitforDRDY();
	//Serial.print("\n@GetRegisterValue_2 | DRDY_state:"); //debug_me
	//Serial.println(DRDY_state); //debug_me
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
