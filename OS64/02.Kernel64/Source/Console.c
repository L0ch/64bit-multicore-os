#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"

// Structure for management console info
CONSOLEMANAGER gs_stConsoleManager = {0, };


void InitializeConsole(int X, int Y){
	// Initialize
	kMemSet(&gs_stConsoleManager, 0, sizeof(gs_stConsoleManager));

	SetCursor(X,Y);
}

// Set cursor position
// Set print char position
void SetCursor(int X, int Y){
	int LinearValue;

	// calc cursor position
	LinearValue = Y * CONSOLE_WIDTH + X;

	// Send 0x0E/0x0F to CRTC control address register(port 0x3D4)
	// Send upper/lower byte of cursor to CRTC,
	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR);
	kOutPortByte(VGA_PORT_DATA, LinearValue >> 8);

	kOutPortByte(VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR);
	kOutPortByte(VGA_PORT_DATA, LinearValue & 0xFF);

	// Update next char position
	gs_stConsoleManager.CurrentPrintOffset = LinearValue;

}


void GetCursor(int* pX, int* pY){

	// X position
	*pX = gs_stConsoleManager.CurrentPrintOffset % CONSOLE_WIDTH;
	// Y position
	*pY = gs_stConsoleManager.CurrentPrintOffset / CONSOLE_WIDTH;
}

void Printf(const char* pcFormatString, ...){
	va_list ap;
	char vcBuffer[1024];
	int NextPrintOffset;

	// variable parameter list
	// process with vsprintf
	va_start(ap, pcFormatString);
	VSPrintf(vcBuffer, pcFormatString, ap);
	va_end(ap);

	NextPrintOffset = ConsolePrintString(vcBuffer);

	SetCursor(NextPrintOffset % CONSOLE_WIDTH, NextPrintOffset / CONSOLE_WIDTH);
}

int ConsolePrintString(const char* pcBuffer){
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	int i,j;
	int Length;
	int PrintOffset;

	PrintOffset = gs_stConsoleManager.CurrentPrintOffset;

	Length = StrLen(pcBuffer);
	for(i=0; i<Length; i++){
		// Print newline character
		if(pcBuffer[i] == '\n'){
			PrintOffset += (CONSOLE_WIDTH- (PrintOffset % CONSOLE_WIDTH)); // current offset + (WIDTH - current X)
 		}
		// Move cursor multiple of 8
		else if(pcBuffer[i] == '\t'){
			PrintOffset += (8 - (PrintOffset % 8));
		}
		// Print character
		else{
			// Set char/attribute to video memory -> print
			// Move next to print position
			pstScreen[PrintOffset].bCharactor = pcBuffer[i];
			pstScreen[PrintOffset].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			PrintOffset++;
		}

		// Scroll process
		if(PrintOffset >= (CONSOLE_HEIGHT * CONSOLE_WIDTH)){

			// Copy up one line line 2 ~
			// src = video memory + width*char -> line 2
			// size = line 2 ~ end line
			kMemCpy(CONSOLE_VIDEOMEMORYADDRESS, CONSOLE_VIDEOMEMORYADDRESS + CONSOLE_WIDTH*sizeof(CHARACTER),
					(CONSOLE_HEIGHT -1)* CONSOLE_WIDTH * sizeof(CHARACTER));
			// Fill with blank
			for (j = (CONSOLE_HEIGHT -1) * CONSOLE_WIDTH ; j<(CONSOLE_HEIGHT * CONSOLE_WIDTH); j++){
				pstScreen[j].bCharactor = ' ';
				pstScreen[j].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			}
			// Set next print position
			PrintOffset = (CONSOLE_HEIGHT -1) * CONSOLE_WIDTH;

		}
	}
	return PrintOffset;

}

// Clear all screen
void ClearScreen(void){
	CHARACTER *pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	int i;

	// Fill with blank all screen
	for(i=0; i< (CONSOLE_HEIGHT * CONSOLE_WIDTH); i++){
		pstScreen[i].bCharactor = ' ';
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}

	// Set cursor 0,0
	SetCursor(0,0);

}

BYTE GetCh(void){
	KEYDATA stData;

	while(1){
		while(kGetKeyFromKeyQueue(&stData) == FALSE){
			;
		}

		if(stData.bFlags & KEY_FLAGS_DOWN){
			return stData.bASCIICode;
		}
	}
}

// Print string
void PrintStringXY(int X, int Y, const char* pcString){
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	int i;
	pstScreen += (Y*80) + X;
	for(i=0; pcString[i] != 0; i++){
		pstScreen[i].bCharactor = pcString[i];
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}
}



















