#include "Types.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);
void Main(void){
	DWORD i;


	kPrintString(0, 3, "Kernel32 Started");

	kPrintString(0, 4, "Minimum Memory Size Check..............[    ]");
	if(kIsMemoryEnough() == FALSE){
		kPrintString(40, 4, "Fail");
		kPrintString(0, 5, "Not Enough Memory. System Requires Over 64Mbyte Memory" );
		while(1);
	}
	else{
		kPrintString(40, 4, "Done");
	}
	kPrintString(0,5, "IA-32e Kernel Area Initialization......[    ]");
	if(kInitializeKernel64Area()==FALSE){
		kPrintString(40, 5, "Fail");
		kPrintString(0, 6, "A-32e Kernel Area Initialization Fail");
		while(1);
	}
	else{
		kPrintString(40, 5, "Done");
	}

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

BOOL kIsMemoryEnough(void){
	DWORD* pdwCurrentAddress;

	pdwCurrentAddress = (DWORD*) 0x100000;

	while((DWORD) pdwCurrentAddress < 0x4000000){
		*pdwCurrentAddress = 0xdeadbeef;

		if (*pdwCurrentAddress != 0xdeadbeef){
			return FALSE;
		}

		pdwCurrentAddress += 0x100000;
	}
	return TRUE;
}

