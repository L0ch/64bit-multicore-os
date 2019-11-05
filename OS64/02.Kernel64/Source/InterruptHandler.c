#include "InterruptHandler.h"
#include "PIC.h"


void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode){
	char vcBuffer[3] = {0, };
	// Print interrupt vector
	vcBuffer[0] = '0' + iVectorNumber / 10;
	vcBuffer[1] = '0' + iVectorNumber % 10;

	kPrintString(0,0,"Exception Occur");
	kPrintString(0,1,"Vector:");
	kPrintString(10,1, vcBuffer);

	while(1);
}

void kCommonInterruptHandler(int iVectorNumber){
	char vcBuffer[] = "[INT:  , ]";
	static int g_iCommonInterruptCount=0;

	// Print interrupt vector
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8]= '0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
	kPrintString(70,0,vbBuffer);

	// Send EOI
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR); // Vector number - 0x20 = IRQ Number
}


void kKeyboardHandler(int iVectorNumber){
	char vcBuffer[] = "[INT:  , ]";
	static int g_iKeyboardInterruptCount = 0;

	// Print interrupt vector
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8]= '0' + g_iKeyboardInterruptCount;
	g_iKeyboardInterruptCount = (g_iKeyboardInterruptCount + 1) % 10;
	kPrintString(0,0,vbBuffer);

	// Send EOI
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}















