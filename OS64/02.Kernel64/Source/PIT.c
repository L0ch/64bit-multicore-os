#include "PIT.h"
#include "AssemblyUtility.h"


void InitializePIT(WORD wCount, BOOL bPeriodic){
	// Initialize PIT control register(0x43)
	// Mode 0, binary counter
	OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);

	// If repeated timer, set mode 2
	if(bPeriodic == TRUE){
		// Mode 2, binary counter
		OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
	}

	// Set count to Counter 0(0x40, LSB -> MSB)
	OutPortByte(PIT_PORT_COUNTER0, wCount);
	OutPortByte(PIT_PORT_COUNTER0, wCount >> 8);

}

// Return current counter 0 value
WORD ReadCounter0(void){
	BYTE bHighByte, bLowByte;
	WORD wTemp = 0;

	// Send LATCH command, read counter 0
	OutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);

	// LSB -> MSB
	bLowByte = InPortByte(PIT_PORT_COUNTER0);
	bHighByte = InPortByte(PIT_PORT_COUNTER0);

	wTemp = bHighByte;
	wTemp = (wTemp << 8) | bLowByte;
	return wTemp;

}

// Wait for count
// Need to reset PIT controller after call function
// Before call this function, disable interrupt
// Measure up to 50ms
void WaitUsingDirectPIT(WORD wCount){
	WORD wLastCounter0;
	WORD wCurrentCounter0;

	// Repeat count 0~0xFFFF
	InitializePIT(0, TRUE);

	wLastCounter0 = ReadCounter0();
	while(1){
		wCurrentCounter0 = ReadCounter0();
		if(((wLastCounter0 - wCurrentCounter0) & 0xFFFF) >= wCount){
			break;
		}
	}
}

