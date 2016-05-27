#ifndef ds3234_H
#define ds3234_H
#include "SPI.h"

class ds3234 {
public:
	ds3234(const int RTC_CS);

	int RTC_init();

	int SetTimeDate(
		int d,
		int mo,
		int y,
		int h,
		int mi,
		int s
		);

	String ReadTimeDate();

private:
	int _RTC_CS;
};
#endif