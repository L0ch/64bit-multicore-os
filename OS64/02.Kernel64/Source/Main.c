#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"

void kPrintString(int iX, int iY, const char* pcString);


void Main(void){

	char vcTemp[2] = {0, };
	BYTE bFlags;
	BYTE bTemp;
	int i = 0;

	kPrintString(0,10, "Switch To IA-32e Mode Success");
	kPrintString(0,11, "IA-32e Kernel Start..........................[Done]");

	kPrintString(0,12, "GDT Initialize And Switch For IA-32e Mode....[    ]");
	kInitializeGDTTableAndTSS();
	kLoadGDTR(GDTR_STARTADDRESS);
	kPrintString(46,12,"Done");

	kPrintString(0,13, "TSS Segment Load.............................[    ]");
	kLoadTR(GDT_TSSSEGMENT);
	kPrintString(46,13,"Done");

	kPrintString(0,14, "IDT Initialize...............................[    ]");
	kInitializeIDTTables();
	kLoadIDTR(IDTR_STARTADDRESS);
	kPrintString(46,14,"Done");

	kPrintString(0,15, "Keyboard Activate............................[    ]");

	//kPrintString(0,13, "TEST");
	if(kActivateKeyboard() == TRUE){
		kPrintString(46,15,"Done");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else{
		kPrintString(46,15,"Fail");
		while(1);
	}

	kPrintString(0,16, "PIC Controller And Interrupt Initialize......[    ]");
	kInitializePIC();
	kMaskPICInterrupt(0);
	kEnableInterrupt();
	kPrintString(46,16,"Done");

	while(1){
		if(kIsOutputBufferFull() == TRUE){
			bTemp = kGetKeyboardScanCode();

			if(kConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags) == TRUE){
				if(bFlags & KEY_FLAGS_DOWN){
					kPrintString(i++, 17, vcTemp);
					if(vcTemp[0] == '0'){
						// Divide Error Exception
						// Execute dummyHandler
						bTemp = bTemp / 0;
					}
				}
			}
		}
	}

}


void kPrintString(int iX, int iY, const char* pcString){
	CHARACTER* pstScreen = (CHARACTER*) 0xB8000;
	int i;

	pstScreen += (iY*80) +iX;
	for(i=0; pcString[i] != 0; i++ ){
		pstScreen[i].bCharactor = pcString[i];
	}

}
