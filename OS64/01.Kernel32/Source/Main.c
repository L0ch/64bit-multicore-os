#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);


void Main(void){

	DWORD i;
	DWORD dwEAX, dwEBX, dwECX, dwEDX;
	char vcVendorString[13] = {0,};

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

	// Read processor Info
	kReadCPUID( 0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX );
	*( DWORD* ) vcVendorString = dwEBX;
	*( ( DWORD* ) vcVendorString + 1 ) = dwEDX;
	*( ( DWORD* ) vcVendorString + 2 ) = dwECX;
	kPrintString(0,7, "Processor Vendor String................       ");
	kPrintString( 39, 7, vcVendorString );


	kReadCPUID( 0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX );
	kPrintString( 0, 8, "64bit Mode Support Check...............[    ]");
	if(dwEDX & (1 << 29)){
		kPrintString(40, 8, "Done");
	}
	else{
		kPrintString(40, 8, "Fail");
		kPrintString(0, 9, "This Processor Does Not Support 64bit Mode");
		while(1);
	}
	// Switch to IA-32e Mode
	kPrintString(0, 9, "Switch To IA-32e Mode");
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

