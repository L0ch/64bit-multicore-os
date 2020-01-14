#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>

// Number of times PIT controller occurred
volatile QWORD g_qwTickCount = 0;


// Fill Memory with specific value
void MemSet( void* pvDestination, BYTE bData, int iSize ){
	int i;

	for(i=0; i<iSize; i++){
		((char*)pvDestination)[i] = bData;
	}
}

// Copy
int MemCpy( void* pvDestination, const void* pvSource, int iSize ){
	int i;

	for(i=0; i<iSize; i++){
		((char*)pvDestination)[i] = ((char*)pvSource)[i];
	}
	return iSize;
}

// Compare
int MemCmp( const void* pvDestination, const void* pvSource, int iSize ){
	int i;
	char cTemp;

	for(i=0; i<iSize; i++){
		cTemp = ((char*)pvDestination)[i] - ((char*)pvSource)[i];
		if(cTemp != 0){
			return (int)cTemp;
		}
	}
	return 0;
}
// Change interrupt flag, return previous interrupt flag status
BOOL SetInterruptFlag(BOOL bEnableInterrupt){
	QWORD qwRFLAGS;

	qwRFLAGS = ReadRFLAGS();
	if(bEnableInterrupt == TRUE){
		EnableInterrupt();
	}
	else{
		DisableInterrupt();
	}

	if(qwRFLAGS & 0x0200){
		return TRUE;
	}
	return FALSE;
}

int StrLen(const char* pcBuffer){
	int i;
	i = 0;
	while(1){
		if(pcBuffer[i] == '\0'){
			break;
		}
		i++;
	}
	return i;
}

// Total Memory Size(MB)
static int gs_qwTotalMemoryMBSize = 0;

void CheckTotalMemorySize(void){
	DWORD* pdwCurrentAddress;
	DWORD dwPreviousValue;

	// Scan from 64MB(0x4000000)
	// 1MB units
	pdwCurrentAddress = (DWORD*) 0x4000000;
	while(1){
		dwPreviousValue = *pdwCurrentAddress;
		// Write/Read 0xdeadbeef, valid memory if success to access
		*pdwCurrentAddress = 0xdeadbeef;
		if(*pdwCurrentAddress != 0xdeadbeef){
			break;
		}
		// restore previous memory
		*pdwCurrentAddress = dwPreviousValue;
		// Next 1MB
		pdwCurrentAddress += (0x400000/4);
	}

	// Calc MB units
	gs_qwTotalMemoryMBSize = (QWORD)pdwCurrentAddress / 0x100000;
}


QWORD GetTotalMemorySize(void){
	return gs_qwTotalMemoryMBSize;
}

long AToI(const char* pcBuffer, int Radix){
	long lResult;

	switch(Radix){
	case 16:
		lResult = HexStringToQword(pcBuffer);
		break;

	case 10:
		lResult = DecimalStringToLong(pcBuffer);
		break;
	}
	return lResult;

}


// Hex string -> QWORD
QWORD HexStringToQword(const char* pcBuffer){
	QWORD qwValue = 0;
	int i;

	for(i = 0; pcBuffer[i] != '\0'; i++){
		qwValue *= 16;
		if(('A' <= pcBuffer[i]) && (pcBuffer[i] <= 'Z')){
			qwValue += (pcBuffer[i] - 'A') + 10;
		}
		else if(('a' <= pcBuffer[i]) && (pcBuffer[i] <= 'z')){
			qwValue += (pcBuffer[i] - 'a') + 10;
		}
		else{
			qwValue += pcBuffer[i] - '0';
		}
	}

	return qwValue;

}

// Decimal string -> long
long DecimalStringToLong(const char* pcBuffer){
	long lValue = 0;
	int i = 0;

	if(pcBuffer[0] == '-'){
		i = 1;
	}

	for(; pcBuffer[i] != '\0'; i++){
		lValue *= 10;
		lValue += pcBuffer[i] - '0';
	}
	if(pcBuffer[0] == '-'){
		lValue = -lValue;
	}
	return lValue;
}


int IToA(long lValue, char* pcBuffer, int iRadix){
	int iReturn;

	switch(iRadix){
	case 16:
		iReturn = HexToString(lValue, pcBuffer);
		break;
	case 10:
	default:
		iReturn = DecimalToString(lValue, pcBuffer);
		break;
	}
	return iReturn;
}

// Hex -> String
int HexToString(QWORD qwValue, char* pcBuffer){
	QWORD i;
	QWORD qwCurrentValue;

	// if 0
	if(qwValue == 0){
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	// Insert buffer  1 digit, 16, 256...
	for(i = 0; qwValue > 0; i++){
		qwCurrentValue = qwValue % 16;
		//ABCDEF
		if(qwCurrentValue >= 10){
			pcBuffer[i] = 'A' + (qwCurrentValue - 10);
		}
		//0~9
		else{
			pcBuffer[i] = '0' + qwCurrentValue;
		}
		qwValue /= 16;
	}
	pcBuffer[i] = '\0';
	// Reverse string to ... 256, 16, 1 digit
	ReverseString(pcBuffer);
	return i;
}

int DecimalToString(long lValue, char* pcBuffer){
	long i=0;

	if(lValue == 0){
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	if(lValue < 0){
		i = 1;
		pcBuffer[0] = '-';
		lValue = -lValue;
	}

	while(lValue > 0){
		pcBuffer[i] = '0' + lValue%10;
		lValue = lValue / 10;
		i++;
	}

	pcBuffer[i] = '\0';
	if(pcBuffer[0] == '-'){
		// If negative, reverse without sign
		ReverseString(&(pcBuffer[1]));
	}
	else{
		ReverseString(pcBuffer);
	}
	return i;

}

void ReverseString(char* pcBuffer){
	int iLength;
	int i;
	char cTemp;

	iLength = StrLen(pcBuffer);

	// Flip left/right
	for(i = 0; i < iLength/2; i++){
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}



// sprintf() internal implementation
int SPrintf(char * pcBuffer, const char* pcFormatString, ...){
	va_list ap;
	int Result;

	va_start(ap, pcFormatString);
	Result = VSPrintf(pcBuffer, pcFormatString, ap);
	va_end(ap);

	return Result;
}

int VSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap){
	QWORD i, j;
	int BufferIndex = 0;
	int FormatLength, CopyLength;
	char* pcCopyString;
	QWORD qwValue;
	int iValue;

	FormatLength = StrLen(pcFormatString);
	for(i = 0; i < FormatLength; i++){
		if(pcFormatString[i] == '%'){
			// % next char
			i++;
			switch(pcFormatString[i]){
			// String
			case 's':
				// Type casting to string
				pcCopyString = (char*) (va_arg(ap,char*));
				CopyLength = StrLen(pcCopyString);
				// Copy to print buffer
				// Buffer index + copied length
				MemCpy(pcBuffer + BufferIndex, pcCopyString, CopyLength);
				BufferIndex += CopyLength;
				break;
			// Character
			case 'c':
				// Type casting to char
				// Copy to print buffer
				pcBuffer[BufferIndex] = (char)(va_arg(ap,int));
				// Move Index
				BufferIndex++;
				break;
			// Decimal(integer)
			case 'd':
			case 'i':
				// Type casting to int
				iValue = (int)(va_arg(ap,int));
				// Copy to print buffer
				// Buffer index + copied length
				BufferIndex += IToA(iValue, pcBuffer + BufferIndex, 10);
				break;
			// 4byte Hex
			case 'x':
			case 'X':
				// Type casting to DWORD
				qwValue = (DWORD)(va_arg(ap, DWORD)) & 0xFFFFFFFF;
				// Copy to print buffer
				// Buffer index + copied length
				BufferIndex += IToA(qwValue, pcBuffer + BufferIndex, 16);
				break;

			case 'q':
			case 'Q':
			case 'p':
				qwValue = (QWORD)(va_arg(ap, QWORD));
				BufferIndex += IToA(qwValue, pcBuffer + BufferIndex, 16);
				break;
			default:
				pcBuffer[BufferIndex] = pcFormatString[i];
				BufferIndex++;
				break;

			}
		}
		else{
			pcBuffer[BufferIndex] = pcFormatString[i];
			BufferIndex++;
		}
	}
	// + NULL, return length
	pcBuffer[BufferIndex] = '\0';
	return BufferIndex;



}

QWORD GetTickCount(void){
	return g_qwTickCount;
}


















