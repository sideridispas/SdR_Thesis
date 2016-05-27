#ifndef ads12xx_H
#define ads12xx_H
#define ADS1256
#include "ads1256.h"
#include "SPI.h"

class ads12xx {
public:
	ads12xx(); //default constructor - Paschalis
	ads12xx(
		const int CS,
		const int DRDY
		);
	void begin();
	void Reset(
		const int RESET_PIN
		);

	unsigned long  GetRegisterValue(
		uint8_t regAdress
		);
	
	void SendCMD(
		uint8_t cmd
		);

	void SetRegisterValue(
		uint8_t regAdress,
		uint8_t regValue
		);

	struct regValues_t
	{
		uint8_t STATUS_val = STATUS_RESET;
		uint8_t MUX_val = MUX_RESET;
		uint8_t ADCON_val = ADCON_RESET;
		uint8_t DRATE_val = DRATE_RESET;
		uint8_t IO_val = IO_RESET;
	};

	long readSingle(
		regValues_t regValues
		);

	long  GetConversion(
		);
	void calibration(
		int cal_cmd
		);
	void SetRegister(
		regValues_t regValues
		);
	float getCalibratedData(
		int mux_value,
		float a1,
		float b1,
		float a2,
		float b2
		);
	void reg_init();


private:
	int _CS;
	int _DRDY;
	int _START;
	volatile int DRDY_state;
};
#endif
