#include "Types.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
void Main(void){
	DWORD i;


	kPrintString(0, 3, "Kernel32 Started");
	kInitializeKernel64Area();
	kPrintString(0,4, "IA-32e Kernel Area Initialization Complete");

	while(1);
}

void kPrintString(int iX, int iY, const char* pcString){
	CHARACTER* pstScreen = (CHARACTER*) 0xB8000;
	int i;

	pstScreen += (iY*80) +iX;
	for(i=0; pcString[i] != 0; i++ ){
		pstScreen[i].bCharactor = pcString[i];
	}

}

//Initialize Kernel64 Area to 0
BOOL kInitializeKernel64Area(void){
	DWORD* pdwCurrentAddress;

	// 1MB~6MB AREA
	pdwCurrentAddress = (DWORD*) 0x100000;
	while((DWORD) pdwCurrentAddress < 0x600000){
		*pdwCurrentAddress = 0x00;
		// if Current Address is not 0x00, problem to read address
		if(*pdwCurrentAddress != 0){
			return FALSE;
		}
		//Next address
		pdwCurrentAddress++;
	}

	return TRUE;
}


