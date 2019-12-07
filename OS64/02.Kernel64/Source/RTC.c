#include "RTC.h"
#include "AssemblyUtility.h"
// Read current time from CMOS memory
void ReadRTCTime(BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond){
	BYTE bData;

	// CMOS memory address register(0x70)
	OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
	// Read HOUR from CMOS data register(0x71)
	bData = InPortByte(RTC_CMOSDATA);
	*pbHour = RTC_BCDTOBINARY(bData);

	// CMOS memory address register(0x70)
	OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE);
	// Read MINUTE from CMOS data register(0x71)
	bData = InPortByte(RTC_CMOSDATA);
	*pbMinute = RTC_BCDTOBINARY(bData);

	// CMOS memory address register(0x70)
	OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_SECOND);
	// Read SECOND from CMOS data register(0x71)
	bData = InPortByte(RTC_CMOSDATA);
	*pbSecond = RTC_BCDTOBINARY(bData);

}


void ReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek){
	BYTE bData;

	// CMOS memory address register(0x70)
	OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_YEAR);
	// Read YEAR from CMOS data register(0x71)
	bData = InPortByte(RTC_CMOSDATA);
	*pwYear = RTC_BCDTOBINARY(bData) + 2000;

	// CMOS memory address register(0x70)
	OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MONTH);
	// Read MONTH from CMOS data register(0x71)
	bData = InPortByte(RTC_CMOSDATA);
	*pbMonth = RTC_BCDTOBINARY(bData);

	// CMOS memory address register(0x70)
	OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
	// Read day of month from CMOS data register(0x71)
	bData = InPortByte(RTC_CMOSDATA);
	*pbDayOfMonth = RTC_BCDTOBINARY(bData);

	// CMOS memory address register(0x70)
	OutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
	// Read day of week from CMOS data register(0x71)
	bData = InPortByte(RTC_CMOSDATA);
	*pbDayOfWeek = RTC_BCDTOBINARY(bData);

}

// Return DayOfWeek
char* ConvertDayOfWeekToString(BYTE bDayOfWeek){
	static char* vpcDayOfWeekString[8] = {"Error", "Sunday", "Monday", "Tuesday",
										"Wednesday", "Thursday", "Friday", "Saturday,"};

	// Error
	if(bDayOfWeek >= 8){
		return vpcDayOfWeekString[0];
	}

	return vpcDayOfWeekString[bDayOfWeek];
}








