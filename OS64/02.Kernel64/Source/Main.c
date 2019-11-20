#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "AssemblyUtility.h"
#include "Utility.h"

void Main(void){

	int CursorX, CursorY;

	InitializeConsole(0,10);

	Printf("Switch To IA-32e Mode Success\n");
	Printf("IA-32e Kernel Start..........................[Done]\n");
	Printf("Initialize Console...........................[Done]\n");

	GetCursor(&CursorX, &CursorY);
	Printf("GDT Initialize And Switch For IA-32e Mode....[    ]");
	InitializeGDTTableAndTSS();
	LoadGDTR(GDTR_STARTADDRESS);
	SetCursor(46, CursorY++);
	Printf("Done\n");

	Printf("TSS Segment Load.............................[    ]");
	LoadTR(GDT_TSSSEGMENT);
	SetCursor(46, CursorY++);
	Printf("Done\n");


	Printf("IDT Initialize...............................[    ]");
	InitializeIDTTables();
	LoadIDTR(IDTR_STARTADDRESS);
	SetCursor(46, CursorY++);
	Printf("Done\n");

	Printf("Total Memory Size Check......................[    ]");
	CheckTotalMemorySize();
	SetCursor(46, CursorY++);
	Printf("Done], %d MB\n",GetTotalMemorySize());

	Printf("Keyboard Activate............................[    ]");

	//Printf(0,13, "TEST");
	if(InitializeKeyboard() == TRUE){
		SetCursor(46, CursorY++);
		Printf("Done\n");
		ChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else{
		SetCursor(46, CursorY++);
		Printf("Fail\n");
		while(1);
	}

	Printf("PIC Controller And Interrupt Initialize......[    ]");
	InitializePIC();
	MaskPICInterrupt(0);
	EnableInterrupt();
	SetCursor(46, CursorY++);
	Printf("Done\n");

	StartConsoleShell();

}
