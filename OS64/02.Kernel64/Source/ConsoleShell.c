#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"

SHELLCOMMANDENTRY gs_vstCommandTable[] = {
		{"help", "Show Help", Help},
		{"clear", "Clear Screen", Clear},
		{"free", "Show Total Memory Size", ShowTotalMemorySize},
		{"strtod","String To Decimal/Hex Convert", StringToDecimalHex},
		{"shutdown", "Shutdown And Reboot OS", Shutdown},
		{"settimer", "Set PIT Controller Counter0", SetTimer},
		{"wait", "Wait ms Using PIT", WaitUsingPIT},
		{"rdtsc", "Read Time Stamp Counter", ReadTimeStampCounter},
		{"cpu", "Measure Processor Speed", MeasureProcessorSpeed},
		{"date", "Show Date And Time", ShowDateAndTime},
		{"createtask", "Create Task", CreateTestTask},
		{"echo", "Print parameter", Echo},
};


void StartConsoleShell(void){
	char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	int CommandBufferIndex = 0;
	BYTE bKey;
	int CursorX, CursorY;
	Printf(CONSOLESHELL_PROMPTMESSAGE);

	while(1){
		bKey = GetCh();
		// Backspace
		if(bKey == KEY_BACKSPACE){
			if(CommandBufferIndex > 0){
				GetCursor(&CursorX, &CursorY);
				PrintStringXY(CursorX - 1, CursorY, " ");
				SetCursor(CursorX - 1, CursorY);
				CommandBufferIndex--;
			}
		}
		// Enter(execute command and next line
		else if(bKey == KEY_ENTER){
			Printf("\n");

			if(CommandBufferIndex > 0){
					vcCommandBuffer[CommandBufferIndex] = '\0';
					ExecuteCommand(vcCommandBuffer);
			}

			Printf("%s", CONSOLESHELL_PROMPTMESSAGE);
			MemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
			CommandBufferIndex = 0;
		}
		else if((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) || (bKey == KEY_CAPSLOCK) ||
				(bKey == KEY_NUMLOCK) || (bKey == KEY_SCROLLLOCK)){
			;
		}
		// normal character
		else{
			// TAB->space
			if(bKey == KEY_TAB){
				bKey = ' ';
			}

			if(CommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT){
				vcCommandBuffer[CommandBufferIndex] = bKey;
				Printf("%c", bKey);
				CommandBufferIndex++;
			}
		}

	}
}


void ExecuteCommand(const char* pcCommandBuffer){
	int i, SpaceIndex;
	int CommandBufferLength, CommandLength;
	int Count;

	// Filter command parameter
	CommandBufferLength = StrLen(pcCommandBuffer);
	for(SpaceIndex = 0; SpaceIndex < CommandBufferLength; SpaceIndex++){
		if(pcCommandBuffer[SpaceIndex] == ' '){
			break;
		}
	}
	//All command entry
	Count = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
	for(i = 0; i < Count; i++){
		CommandLength = StrLen(gs_vstCommandTable[i].pcCommand);
		//
		if((CommandLength == SpaceIndex) &&
		   (MemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer, SpaceIndex)==0)){
			// Execute command function
			gs_vstCommandTable[i].pfFunction(pcCommandBuffer + SpaceIndex +1);
			break;
		}
	}
	if(i >= Count){
		Printf("'%s': command not found.\n", pcCommandBuffer);
	}
}

// Initialize parameter structure
void InitializeParameter(PARAMETERLIST* pstList, const char* pcParameter){
	pstList->pcBuffer = pcParameter;
	pstList->Length = StrLen(pcParameter);
	pstList->CurrentPosition = 0;
}

// Return parameter, length separated by space
int GetNextParameter(PARAMETERLIST* pstList, char* pcParameter){
	int i;
	int Length;

	// Parameter does not exist
	if(pstList->Length <= pstList->CurrentPosition){
		return 0;
	}

	// Search space
	for(i = pstList->CurrentPosition; i < pstList->Length; i++){
		if(pstList->pcBuffer[i] == ' '){
			break;
		}
	}

	// Copy parameter and return length
	MemCpy(pcParameter, pstList->pcBuffer + pstList->CurrentPosition, i);
	Length = i - pstList->CurrentPosition;
	pcParameter[Length] = '\0';

	pstList->CurrentPosition += Length + 1;
	return Length;
}




/////////////////////////////////////////////////////////
// Processing Command
////////////////////////////////////////////////////////

void Help(const char* pcCommandBuffer){
	int i;
	int Count;
	int CursorX, CursorY;
	int Length, MaxCommandLength = 0;
	Printf("=======================================\n");
	Printf("Console Shell by dw0rptr, version 0.1\n");
	Printf("=======================================\n");
	Count = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

	// calc longest command length for print sort
	for(i = 0; i < Count; i++){
		Length = StrLen(gs_vstCommandTable[i].pcCommand);
		if(Length > MaxCommandLength){
			MaxCommandLength = Length;
		}
	}

	// Print help
	for(i = 0; i < Count; i++){
		Printf("%s", gs_vstCommandTable[i].pcCommand);
		GetCursor(&CursorX, &CursorY);
		SetCursor(MaxCommandLength, CursorY);
		Printf("  - %s\n", gs_vstCommandTable[i].pcHelp);
	}
}

// Clear display
void Clear(const char* pcParameterBuffer){
	//Clear Screen and move cursor
	ClearScreen();
	SetCursor(0,1);
}

void ShowTotalMemorySize(const char* pcParameterBuffer){
	Printf("Total Memory Size : %d MB\n", GetTotalMemorySize());
}

void StringToDecimalHex(const char* pcParameterBuffer){
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	int iCount = 0;
	long lValue;

	InitializeParameter(&stList, pcParameterBuffer);

	while(1){
		iLength = GetNextParameter(&stList, vcParameter);

		// Parameter does not exist
		if(iLength == 0){

			if(iCount == 0){
				Printf("There is No Parameter\n");
				Printf("Usage: strtod [DECIMAL/HEXADECIMAL]...\n");
			}
			break;
		}

		// Print parameter info
		Printf("Parameter %d = '%s', Length = %d, ", iCount +1, vcParameter, iLength);

		// If '0x' Hex
		if(MemCmp(vcParameter, "0x", 2) == 0){
			lValue = AToI(vcParameter + 2, 16);
			Printf("HEX Value = %q\n",lValue);
		}
		// Decimal
		else{
			lValue = AToI(vcParameter, 10);
			Printf("Decimal Value = %d\n", lValue);
		}
		iCount++;
	}
}

void Shutdown(const char* pcParameterBuffer){
	Printf("System Shutdown...\n");

	Printf("Press Any Key To Reboot...");
	GetCh();
	Reboot();
}


void SetTimer(const char* pcParameterBuffer){
	char vcParameter[100];
	PARAMETERLIST stList;
	long lMillisecond;
	BOOL bPeriodic;

	InitializeParameter(&stList, pcParameterBuffer);

	// Parameter1 not exist
	if(GetNextParameter(&stList, vcParameter) == 0){
		Printf("USAGE : settimer 10(ms) 1(periodic)\n");
		return ;
	}

	// Millisecond
	lMillisecond = AToI(vcParameter, 10);

	// Parameter2 not exist
	if(GetNextParameter(&stList, vcParameter) == 0){
		Printf("USAGE : settimer 10(ms) 1(periodic)\n");
		return ;
	}
	// Periodic
	bPeriodic = AToI(vcParameter, 10);

	// Process
	InitializePIT(MSTOCOUNT(lMillisecond), bPeriodic);
	Printf("Time = %d ms, Periodic = %d Change Complete\n", lMillisecond, bPeriodic);

}

void WaitUsingPIT(const char* pcParameterBuffer){
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	long lMillisecond;
	int i;

	InitializeParameter(&stList, pcParameterBuffer);

	if(GetNextParameter(&stList, vcParameter) == 0){
		Printf("USAGE : wait 100(ms)\n");
		return ;
	}

	lMillisecond = AToI(vcParameter, 10);
	Printf("%d ms Sleep Start...\n", lMillisecond);


	DisableInterrupt();
	for(i = 0; i < lMillisecond / 30; i++){
		WaitUsingDirectPIT(MSTOCOUNT(30));

	}
	WaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 30));
	EnableInterrupt();

	Printf("%d ms Sleep Complete\n", lMillisecond);

	//Restore timer
	InitializePIT(MSTOCOUNT(1), TRUE);

}


void ReadTimeStampCounter(const char* pcParameterBuffer){
	QWORD qwTSC;

	qwTSC = ReadTSC();
	Printf("Time Stamp Counter = %d\n", qwTSC);
}

// Measure processor performance
void MeasureProcessorSpeed(const char* pcParameterBuffer){
	int i;
	QWORD qwLastTSC, qwTotalTSC = 0;

	Printf("Now Measuring Processor performance ");

	// Measure processor speed using difference of time stamp counter during 10s
	DisableInterrupt();
	for(i = 0; i < 200; i++){
		qwLastTSC = ReadTSC();
		WaitUsingDirectPIT(MSTOCOUNT(50)); // 50ms * 200
		qwTotalTSC += ReadTSC() - qwLastTSC;
		if(i%5 == 0){
			Printf(".");
		}
	}

	// Restore Timer
	InitializePIT(MSTOCOUNT(1), TRUE);
	EnableInterrupt();

	Printf("\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000);
}


// Show Date/Time Information stored RTC controller
void ShowDateAndTime(const char* pcParameterBuffer){
	BYTE bSecond, bMinute, bHour;
	BYTE bDayOfWeek, bDayOfMonth, bMonth;
	WORD wYear;

	// Read Date/Time from RTC controller
	ReadRTCTime(&bHour, &bMinute, &bSecond);
	ReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);

	Printf("%d/%d/%d %s, ", wYear, bMonth, bDayOfMonth, ConvertDayOfWeekToString(bDayOfWeek));
	Printf(" %d:%d:%d\n", bHour, bMinute, bSecond);



}


static TCB gs_vstTask[2] = {0, };
static QWORD gs_vstStack[1024] = {0, };
void TestTask(void){
	int i = 0;

	while(1){
		Printf("[%d] This message is from TestTask. Press any key to switch ConsoleShell\n", i++);
		GetCh();

		SwitchContext(&(gs_vstTask[1].stContext), &(gs_vstTask[0].stContext));

	}
}

void CreateTestTask(const char* pcParameterBuffer){
	KEYDATA stData;
	int i = 0;

	SetUpTask(&(gs_vstTask[1]), 1, 0, (QWORD)TestTask, &(gs_vstStack), sizeof(gs_vstStack));

	while(1){
		Printf("[%d] This message is from ConsoleShell. Press any key to sitch TestTask\n", i++);
		Printf("Press q If want exit Task test\n");
		if(GetCh() == 'q'){
			break;
		}

		SwitchContext(&(gs_vstTask[0].stContext), &(gs_vstTask[1].stContext));
	}
}

void Echo(const char* pcParameterBuffer){
	char vcParameter[100];
	PARAMETERLIST stList;
	BOOL bAppendNewLine = TRUE;
	int iLength;
	InitializeParameter(&stList, pcParameterBuffer);



	// Do not append newline
	iLength = GetNextParameter(&stList, vcParameter);
	if(MemCmp(vcParameter, "-n", 2) == 0){
		bAppendNewLine = FALSE;
		Printf("%s",pcParameterBuffer+iLength);
	}
	else{
		Printf("%s",pcParameterBuffer);
	}

	if(bAppendNewLine){
		Printf("\n");
	}


}












