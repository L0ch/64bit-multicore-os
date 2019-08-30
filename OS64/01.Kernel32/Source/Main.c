#include "Types.h"
#include "Page.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);


void Main(void){

	DWORD i;

	kPrintString(0, 3, "Kernel32 Started");

	kPrintString(0, 4, "Minimum Memory Size Check..............[    ]");
	if(kIsMemoryEnough() == FALSE){		//If memory less than 64Mbyte
		kPrintString(40, 4, "Fail");
		kPrintString(0, 5, "Not Enough Memory. System Requires Over 64Mbyte Memory" );
		while(1);
	}
	else{
		kPrintString(40, 4, "Done");
	}
	kPrintString(0,5, "IA-32e Kernel Area Initialize..........[    ]");
	if(kInitializeKernel64Area()==FALSE){	//If can't access Kernel Area
		kPrintString(40, 5, "Fail");
		kPrintString(0, 6, "A-32e Kernel Area Initialization Fail");
		while(1);
	}
	else{
		kPrintString(40, 5, "Done");
	}

	kPrintString(0, 6, "IA-32e Page Tables Initialize..........[    ]");
	kInitializePageTables();
	kPrintString( 40, 6, "Done");

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
	//Start 1Mbyte
	pdwCurrentAddress = (DWORD*) 0x100000;

	//Until 64Mbyte
	while((DWORD) pdwCurrentAddress < 0x4000000){
		*pdwCurrentAddress = 0xdeadbeef;

		// If Current Address is not 0xdeadbeef(can't read address), memory less than 64Mbyte
		if (*pdwCurrentAddress != 0xdeadbeef){
			return FALSE;
		}
		// next 1MB
		pdwCurrentAddress += 0x100000;
	}
	return TRUE;
}

