#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"

void Main(void){

	int CursorX, CursorY;

	InitializeConsole(0,10);

	Printf("Switch To IA-32e Mode Success\n");
	Printf("IA-32e Kernel Start..........................[Done]\n");
	Printf("Initialize Console...........................[Done]\n");

	GetCursor(&CursorX, &CursorY);
	Printf("GDT Initialize And Switch For IA-32e Mode....[    ]");
	kInitializeGDTTableAndTSS();
	kLoadGDTR(GDTR_STARTADDRESS);
	SetCursor(46, CursorY++);
	Printf("Done\n");

	Printf("TSS Segment Load.............................[    ]");
	kLoadTR(GDT_TSSSEGMENT);
	SetCursor(46, CursorY++);
	Printf("Done\n");


	Printf("IDT Initialize...............................[    ]");
	kInitializeIDTTables();
	kLoadIDTR(IDTR_STARTADDRESS);
	SetCursor(46, CursorY++);
	Printf("Done\n");

	Printf("Total Memory Size Check......................[    ]");
	CheckTotalMemorySize();
	SetCursor(46, CursorY++);
	Printf("Done], %d MB\n",GetTotalMemorySize());

	Printf("Keyboard Activate............................[    ]");

	//Printf(0,13, "TEST");
	if(kInitializeKeyboard() == TRUE){
		SetCursor(46, CursorY++);
		Printf("Done\n");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else{
		SetCursor(46, CursorY++);
		Printf("Fail\n");
		while(1);
	}

	Printf("PIC Controller And Interrupt Initialize......[    ]");
	kInitializePIC();
	kMaskPICInterrupt(0);
	kEnableInterrupt();
	SetCursor(46, CursorY++);
	Printf("Done\n");

	StartConsoleShell();

}
