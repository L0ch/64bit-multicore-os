#ifndef __RTC_H__
#define __RTC_H__

#include "Types.h"

// Macro

// I/O port
#define RTC_CMOSADDRESS			0x70
#define RTC_CMOSDATA			0x71


// CMOS memory address
#define RTC_ADDRESS_SECOND		0x00
#define RTC_ADDRESS_MINUTE		0x02
#define RTC_ADDRESS_HOUR		0x04
#define RTC_ADDRESS_DAYOFWEEK	0x06
#define RTC_ADDRESS_DAYOFMONTH	0x07
#define RTC_ADDRESS_MONTH		0x08
#define RTC_ADDRESS_YEAR		0x09

// BCD -> Binary format macro (tens)
#define RTC_BCDTOBINARY(x)	((((x) >> 4) * 10) + ((x) & 0x0F))

// Function
void ReadRTCTime(BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond);
void ReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek);
char* ConvertDayOfWeekToString(BYTE bDayOfWeek);

#endif /*__RTC_H__*/



