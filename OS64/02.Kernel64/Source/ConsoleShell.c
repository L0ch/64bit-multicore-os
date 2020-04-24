#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "Synchronization.h"
#include "DynamicMemory.h"
#include "HDD.h"

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
		{"createtask", "Create Task, ex)createtask [TYPE] [COUNT]", CreateTestTask},
		{"echo", "Print parameter", Echo},
		{"chpri", "Change Task Priority. ex)chpri [ID] [PRIORITY]", ChangeTaskPriority},
		{"task", "Show Task List", ShowTaskList},
		{"kill", "End Task, ex)kill [ID] or 0xFFFFFFFF(All Task)", KillTask},
		{"cpuload", "Show Processor Load", CPULoad},
		{"testmutex", "Test Mutex Function", TestMutex},
		{"testthread", "Test Thread And Process Function", TestThread},
		{"testpie", "Test PIE Calculation", TestPIE},
		{"dynamicmeminfo", "Show Dynamic Memory Information", ShowDynamicMemoryInfo},
		{"testalloc", "Test Sequential Allocation & Free", TestSequentialAllocation},
		{"testrandalloc", "Test Random Allocation & Free", TestRandomAllocation},
		{"hddinfo", "Show HDD Information", ShowHDDInformation},
		{"readsector", "Read HDD Sector, ex)readsector [LBA] [COUNT]", ReadSector},
		{"writesector", "Write HDD Sector, ex)writesector [LBA] [COOUNT]", WriteSector},
};


void StartConsoleShell(void){
	char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	int CommandBufferIndex = 0;
	BYTE bKey;
	int CursorX, CursorY;
	PrintPrompt(CONSOLESHELL_PROMPTMESSAGE);

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

			PrintPrompt(CONSOLESHELL_PROMPTMESSAGE);
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

static void Help(const char* pcCommandBuffer){
	int i;
	int Count;
	int CursorX, CursorY;
	int Length, MaxCommandLength = 0;
	Printf("=======================================\n");
	Printf("Console Shell by dw0rptr, version 0.1\n");
	Printf("=======================================\n");
	Count = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

	// Calculate longest command length for print sort
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

		if((i != 0) && ((i % 20) == 0)){
			Printf("Press any key to continue...(press 'q' to exit)");
			if(GetCh() == 'q'){
				Printf("\n");
				break;
			}
			Printf("\n");
		}
	}
}

// Clear display
static void Clear(const char* pcParameterBuffer){
	//Clear Screen and move cursor
	ClearScreen();
	SetCursor(0,1);
}

static void ShowTotalMemorySize(const char* pcParameterBuffer){
	Printf("Total Memory Size : %d MB\n", GetTotalMemorySize());
}

static void StringToDecimalHex(const char* pcParameterBuffer){
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

static void Shutdown(const char* pcParameterBuffer){
	Printf("System Shutdown...\n");

	Printf("Press Any Key To Reboot...");
	GetCh();
	Reboot();
}


static void SetTimer(const char* pcParameterBuffer){
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

static void WaitUsingPIT(const char* pcParameterBuffer){
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


static void ReadTimeStampCounter(const char* pcParameterBuffer){
	QWORD qwTSC;

	qwTSC = ReadTSC();
	Printf("Time Stamp Counter = %d\n", qwTSC);
}

// Measure processor performance
static void MeasureProcessorSpeed(const char* pcParameterBuffer){
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
static void ShowDateAndTime(const char* pcParameterBuffer){
	BYTE bSecond, bMinute, bHour;
	BYTE bDayOfWeek, bDayOfMonth, bMonth;
	WORD wYear;

	// Read Date/Time from RTC controller
	ReadRTCTime(&bHour, &bMinute, &bSecond);
	ReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);

	Printf("%d/%d/%d %s, ", wYear, bMonth, bDayOfMonth, ConvertDayOfWeekToString(bDayOfWeek));
	Printf(" %d:%d:%d\n", bHour, bMinute, bSecond);



}


static void TestTask1(void){
	BYTE bData;
	int i = 0, iX = 0, iY = 0, iMargin, j;
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;

	// Use task ID as screen offset
	pstRunningTask = GetRunningTask();
	iMargin = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) % 10;

	for(j=0; j<20000; j++){
		switch(i){
		case 0:
			iX++;
			if(iX >= (CONSOLE_WIDTH - iMargin)){
				i = 1;
			}
			break;
		case 1:
			iY++;
			if(iY >= (CONSOLE_HEIGHT - iMargin)){
				i = 2;
			}
			break;
		case 2:
			iX--;
			if(iX < iMargin){
				i = 3;
			}
			break;
		case 3:
			iY--;
			if(iY<iMargin){
				i = 0;
			}
			break;
		}
		pstScreen[iY * CONSOLE_WIDTH + iX].bCharactor = bData;
		pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
		bData++;
		//Schedule();
	}

	ExitTask();


}

//
static void TestTask2(void){
	int i = 0, iOffset;
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;
	TCB* pstRunningTask;
	char vcData[4] = {'-', '\\', '|', '/'};

	// Use task ID as screen offset
	pstRunningTask = GetRunningTask();
	iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

	while(1){
		pstScreen[iOffset].bCharactor = vcData[i % 4];
		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
		i++;

		//Schedule();

	}

}

// Create task
static void CreateTestTask(const char* pcParameterBuffer){
	PARAMETERLIST stList;
	char vcType[30];
	char vcCount[30];
	int i;

	InitializeParameter(&stList, pcParameterBuffer);
	GetNextParameter(&stList, vcType);
	GetNextParameter(&stList, vcCount);

	switch(AToI(vcType, 10)){
	// Create type 1 task
	case 1:
		for(i=0; i<AToI(vcCount, 10); i++){
			if(CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)TestTask1) == NULL){
				break;
			}
		}
		Printf("Task1 %d Created\n", i);
		break;
	// Create type 2 task
	case 2:
	default:
		for(i=0; i<AToI(vcCount, 10); i++){
			if(CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)TestTask2) == NULL){
				break;
			}
		}
		Printf("Task2 %d Created\n", i);
		break;
	}

}
// Change Task Priority
static void ChangeTaskPriority(const char* pcParameterBuffer){
	PARAMETERLIST stList;
	char vcID[30];
	char vcPriority[30];
	QWORD qwID;
	BYTE bPriority;

	InitializeParameter(&stList, pcParameterBuffer);
	GetNextParameter(&stList, vcID);
	GetNextParameter(&stList, vcPriority);

	// Hex format
	if(MemCmp(vcID, "0x", 2) == 0){
		qwID = AToI(vcID + 2, 16);
	}
	// Decimal format
	else{
		qwID = AToI(vcID, 10);
	}
	bPriority = AToI(vcPriority, 10);

	// Change priority
	Printf("Change Task Priority ID [0x%q] Priority[%d] ", qwID, bPriority);
	if(ChangePriority(qwID, bPriority) == TRUE){
		Printf("Success\n");
	}
	else{
		Printf("Fail\n");
	}
}

static void ShowTaskList(const char* pcParameterBuffer){
	int i;
	TCB* pstTCB;
	int iCount = 0;

	Printf("========== Task Total Count [%d] ==========\n", GetTaskCount());
	for(i=0; i<TASK_MAXCOUNT; i++){
		pstTCB = GetTCBInTCBPool(i);
		if((pstTCB->stLink.qwID >> 32) != 0){
			if((iCount != 0) && ((iCount % 10) == 0)){
				Printf("Press any key to continue...(exit : q)");
				if(GetCh() == 'q'){
					Printf("\n");
					break;
				}
				Printf("\n");
			}
			Printf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n",
					1 + iCount++, pstTCB->stLink.qwID, GETPRIORITY(pstTCB->qwFlags),
					pstTCB->qwFlags, GetListCount(&(pstTCB->stChildThreadList)));
			Printf("     Parent PID[0x%Q], Memory Address[0x%Q], Size[0x%Q]\n", pstTCB->qwParentProcessID,
					pstTCB->pvMemoryAddress, pstTCB->qwMemorySize);

		}
	}

}

static void KillTask(const char* pcParameterBuffer){
	PARAMETERLIST stList;
	char vcID[30];
	QWORD qwID;
	TCB* pstTCB;
	int i;

	InitializeParameter(&stList, pcParameterBuffer);
	GetNextParameter(&stList, vcID);

	if(MemCmp(vcID, "0x", 2) == 0){
		qwID = AToI(vcID + 2, 16);
	}
	else{
		qwID = AToI(vcID, 10);
	}

	// Kill specific task
	if(qwID != 0xFFFFFFFF){
		pstTCB = GetTCBInTCBPool(GETTCBOFFSET(qwID));
		qwID = pstTCB->stLink.qwID;

		// Except system task
		if(((qwID >> 32) != 0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00)){
			Printf("Kill Task ID [0x%q] ", qwID);
			if(EndTask(qwID) == TRUE){
				Printf("Success\n");
			}
			else{
				Printf("Fail\n");
			}
		}
		else{
			Printf("[ERROR] Task does not exist or system task\n");
		}

	}
	// Kill all task except shell/idle task
	else{
		for(i=0; i<TASK_MAXCOUNT; i++){
			pstTCB = GetTCBInTCBPool(i);
			qwID = pstTCB->stLink.qwID;
			if((qwID >> 32) != 0 && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00)){
				Printf("Kill Task ID [0x%q] ", qwID);
				if(EndTask(qwID) == TRUE){
					Printf("Success\n");
				}
				else{
					Printf("Fail\n");
				}
			}
		}
	}



}

static void CPULoad(const char* pcParameterBuffer){
	Printf("Processor Load : %d%%\n", GetProcessorLoad());
}

// For test
static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

// Test Mutex
static void PrintNumberTask(void){
	int i,j;
	QWORD qwTickCount;

	qwTickCount = GetTickCount();
	while((GetTickCount() - qwTickCount) < 50){
		Schedule();
	}

	// Print number with loop
	for(i=0; i<5; i++){
		Lock(&gs_stMutex);
		Printf("Task ID [0x%q] Value[%d]\n", GetRunningTask()->stLink.qwID, gs_qwAdder);
		gs_qwAdder += 1;
		Unlock(&gs_stMutex);

		// Wait for task to change
		for(j = 0; j<30000; j++);
	}

	// Wait for all task to exit
	qwTickCount = GetTickCount();
	while((GetTickCount() - qwTickCount) < 1000){
		Schedule();
	}

	ExitTask();

}

// Create task for test Mutex
static void TestMutex(const char* pcParameterBuffer){
	int i;

	gs_qwAdder = 1;

	InitializeMutex(&gs_stMutex);

	// Create 3 task for test Mutex
	for(i=0; i<3; i++){
		CreateTask(TASK_FLAGS_LOW| TASK_FLAGS_THREAD, 0, 0, (QWORD)PrintNumberTask);
	}
	Printf("Wait Until %d Task End...\n", i);
	GetCh();
}

static void CreateThreadTask(void){
	int i;
	for(i=0; i<3; i++){
		CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)TestTask2);
	}

	while(1){
		Sleep(1);
	}
}

static void TestThread(const char* pcParameterBuffer){
	TCB* pstProcess;
	pstProcess = CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, (void*)0xEEEEEEEE, 0x1000, (QWORD)CreateThreadTask);

	if(pstProcess != NULL){
		Printf("Process [0x%Q] Create Success\n", pstProcess->stLink.qwID);
	}
	else{
		Printf("Process Create Fail\n");
	}
}

static volatile QWORD gs_qwRandomValue = 0;

QWORD Random(void){
	gs_qwRandomValue = (gs_qwRandomValue * 412153 + 5571031) >> 16;
	return gs_qwRandomValue;
}

static void FPUTestTask(void){
	double dValue1;
	double dValue2;

	TCB* pstRunningTask;
	QWORD qwCount = 0;
	QWORD qwRandomValue;
	int i;
	int iOffset;
	char vcData[4] = {'-', '\\', '|', '/'};
	CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS;

	pstRunningTask = GetRunningTask();

	// Get Task ID to use offset
	iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

	while(1){
		dValue1 = 1;
		dValue2 = 1;

		for(i=0; i<10; i++){
			qwRandomValue = Random();
			dValue1 *= (double) qwRandomValue;
			dValue2 *= (double) qwRandomValue;

			Sleep(1);
			qwRandomValue = Random();
			dValue1 /= (double) qwRandomValue;
			dValue2 /= (double) qwRandomValue;

		}
		if(dValue1 != dValue2){
			Printf("Value Is Not Same [%f] != [%f]\n", dValue1, dValue2);
			break;
		}
		qwCount++;

		pstScreen[iOffset].bCharactor = vcData[qwCount % 4];
		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;

	}

}

static void TestPIE(const char* pcParameterBuffer){
	double dResult;
	int i;

	Printf("PIE Calculation Test\n");
	Printf("Result: 355 / 113 = ");
	dResult = (double) 355 / 113;
	Printf("%d.%d%d\n", (QWORD) dResult, ((QWORD) (dResult * 10) % 10), ((QWORD)(dResult * 100) % 10));

	for(i=0; i<100; i++){
		CreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD) FPUTestTask);
	}

}

static void Echo(const char* pcParameterBuffer){
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


static void ShowDynamicMemoryInfo(const char* pcParameterBuffer){
	QWORD qwStartAddress, qwTotalSize, qwMetaSize, qwUsedSize;

	GetDynamicMemoryInfo(&qwStartAddress, &qwTotalSize, &qwMetaSize, &qwUsedSize);

	Printf("========== Dynamic Memory Information ==========\n");
	Printf("Start Address: [0x%Q]\n", qwStartAddress);
	Printf("Total Size:    [0x%Q]byte, [%d]MB\n", qwTotalSize, qwTotalSize / 1024 / 1024);
	Printf("Meta Size:     [0x%Q]byte, [%d]KB\n", qwMetaSize, qwMetaSize / 1024);
	Printf("Used Size:     [0x%Q]byte, [%d]KB\n", qwUsedSize, qwUsedSize / 1024);
}

static void TestSequentialAllocation(const char* pcParameterBuffer){
	DYNAMICMEMORY* pstMemory;
	long i, j, k;
	QWORD* pqwBuffer;

	Printf("========== Dynamic Memory Test ==========\n");
	pstMemory = GetDynamicMemoryManager();

	for(i=0; i<pstMemory->iMaxLevelCount; i++){
		Printf("Block List [%d] Test Start...\n", i);
		Printf("Allocation and Verification: ");

		// Allocate all blocks in each block list
		for(j=0; j<(pstMemory->iBlockCountOfSmallestBlock >> i); j++){
			pqwBuffer = AllocateMemory(DYNAMICMEMORY_MIN_SIZE << i);
			if(pqwBuffer == NULL){
				Printf("\nFailed to Allocate\n");
				return ;
			}

			for(k=0; k<(DYNAMICMEMORY_MIN_SIZE << i) / 8; k++){
				pqwBuffer[k] = k;
			}
			for(k=0; k<(DYNAMICMEMORY_MIN_SIZE << i)/8; k++){
				if(pqwBuffer[k] != k){
					Printf("\nFailed to Verify\n");
					return ;
				}
			}
			Printf(".");
		}

		Printf("\nFree: ");
		// Free all blocks
		for(j=0; j<(pstMemory->iBlockCountOfSmallestBlock >> i); j++){
			if(FreeMemory((void*)(pstMemory->qwStartAddress + (DYNAMICMEMORY_MIN_SIZE << i) * j)) == FALSE){
				Printf("\nFailed to Free\n");
				return ;
			}
			Printf(".");
		}
		Printf("\n");
	}
	Printf("Test Complete.\n");
}


static void RandomAllocationTask(void){
	TCB* pstTask;
	QWORD qwMemorySize;
	char vcBuffer[200];
	BYTE* pbAllocationBuffer;
	int i, j;
	int iY;

	pstTask = GetRunningTask();
	iY = (pstTask->stLink.qwID) % 15 + 9;

	for(j=0; j<10; j++){
		do{
			// 1KB ~ 32MB rand
			qwMemorySize = ((Random() % (32 * 1024)) + 1) * 1024;
			pbAllocationBuffer = AllocateMemory(qwMemorySize);

			if(pbAllocationBuffer == 0){
				Sleep(1);
			}
		} while(pbAllocationBuffer == 0);

		SPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Allocation Success", pbAllocationBuffer, qwMemorySize);
		PrintStringXY(20, iY, vcBuffer);
		Sleep(200);

		SPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Write...     ", pbAllocationBuffer, qwMemorySize);
		PrintStringXY(20, iY, vcBuffer);
		for(i=0; i<qwMemorySize / 2; i++){
			pbAllocationBuffer[i] = Random() & 0xFF;
			pbAllocationBuffer[i + (qwMemorySize / 2)] = pbAllocationBuffer[i];
		}
		Sleep(200);
		SPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Verify...     ", pbAllocationBuffer, qwMemorySize);
		PrintStringXY(20, iY, vcBuffer);
		for(i=0; i<qwMemorySize / 2; i++){
			if(pbAllocationBuffer[i] != pbAllocationBuffer[i + (qwMemorySize / 2)]){
				Printf("Task ID[0x%Q] Verify Failed\n", pstTask->stLink.qwID);
				ExitTask();
			}
		}
		FreeMemory(pbAllocationBuffer);
		Sleep(200);
	}
	ExitTask();
}


static void TestRandomAllocation(const char* pcParameterBuffer){
	int i;
	for(i=0; i<1000; i++){
		CreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD, 0, 0, (QWORD) RandomAllocationTask );
	}
}

//////////////////////////////////////////////
// Support only Primary Master HDD
//////////////////////////////////////////////
static void ShowHDDInformation(const char* pcParameterBuffer){
	HDDINFORMATION stHDD;
	char vcBuffer[100];

	if(ReadHDDInformation(TRUE, TRUE, &stHDD) == FALSE){
		Printf("HDD Information Read Fail\n");
		return ;
	}

	Printf("================ Primary Master HDD Information ================\n");

	// Model Number
	MemCpy(vcBuffer, stHDD.vwModelNumber, sizeof(stHDD.vwModelNumber));
	vcBuffer[sizeof(stHDD.vwModelNumber) - 1] = '\0';
	Printf("Model Number:\t %s\n", vcBuffer);

	// Serial Number
	MemCpy(vcBuffer, stHDD.vwSerialNumber, sizeof(stHDD.vwSerialNumber));
	vcBuffer[sizeof(stHDD.vwSerialNumber) - 1] = '\0';
	Printf("Serial Number:\t %s\n", vcBuffer);

	// Head, Cylinder, Sector of each Cylinder
	Printf("Head Count:\t %d\n", stHDD.wNumberOfHead);
	Printf("Cylinder Count:\t %d\n", stHDD.wNumberOfCylinder);
	Printf("Sector Count:\t %d\n", stHDD.wNumberOfSectorPerCylinder);

	// Total Sector
	Printf("Total Sector:\t %d Sector, %dMB\n", stHDD.dwTotalSectors, stHDD.dwTotalSectors / 2 / 1024);
}

// Read as sector count from LBA
static void ReadSector(const char* pcParameterBuffer){
	PARAMETERLIST stList;
	char vcLBA[50], vcSectorCount[50];
	DWORD dwLBA;
	int iSectorCount;
	char* pcBuffer;
	int i, j;
	BYTE bData;
	BOOL bExit = FALSE;

	// Extract LBA address/sector count
	InitializeParameter(&stList, pcParameterBuffer);
	if((GetNextParameter(&stList, vcLBA) == 0) || (GetNextParameter(&stList, vcSectorCount) == 0)){
		Printf("ex) readsector [LBA] [COUNT]\n");
		return;
	}
	dwLBA = AToI(vcLBA, 10);
	iSectorCount = AToI(vcSectorCount, 10);

	pcBuffer = AllocateMemory(iSectorCount * 512);
	if(ReadHDDSector(TRUE, TRUE, dwLBA, iSectorCount, pcBuffer) == iSectorCount){
		Printf("LBA [%d], [%d] Sector Read Success.", dwLBA, iSectorCount);
		for(j=0; j<iSectorCount; j++){
			for(i=0; i<512; i++){
				if(!((j ==0 ) && (i == 0)) && ((i % 256) == 0)){
					Printf("\nPress any key to continue... (press 'q' to exit)");
					if(GetCh() == 'q'){
						bExit = TRUE;
						break;
					}
				}
				if((i % 16) == 0){
					Printf("\n[LBA:%d, Offset:%d]\t", dwLBA + j, i);
				}
				bData = pcBuffer[j * 512 + i] & 0xFF;
				if(bData < 16){
					Printf("0");
				}
				Printf("%X ", bData);
			}
			if(bExit == TRUE){
				break;
			}
		}
		Printf("\n");
	}
	else{
		Printf("Read Fail");
	}
	FreeMemory(pcBuffer);
}

static void WriteSector(const char* pcParameterBuffer){
	PARAMETERLIST stList;
	char vcLBA[50], vcSectorCount[50];
	DWORD dwLBA;
	int iSectorCount;
	char* pcBuffer;
	int i,j;
	BOOL bExit = FALSE;
	BYTE bData;
	static DWORD s_dwWriteCount = 0;

	InitializeParameter(&stList, pcParameterBuffer);
	if((GetNextParameter(&stList, vcLBA) == 0) || (GetNextParameter(&stList, vcSectorCount) == 0)){
		Printf("ex) writesector [LBA] [COUNT]\n");
		return;
	}
	dwLBA = AToI(vcLBA, 10);
	iSectorCount = AToI(vcSectorCount, 10);
	s_dwWriteCount++;

	pcBuffer = AllocateMemory(iSectorCount * 512);
	for(j=0; j<iSectorCount; j++){
		for(i=0; i<512; i+=8){
			*(DWORD*) &(pcBuffer[j * 512 + i]) = dwLBA + j;
			*(DWORD*) &(pcBuffer[j * 512 + i + 4]) = s_dwWriteCount;
		}
	}

	if(WriteHDDSector(TRUE, TRUE, dwLBA, iSectorCount, pcBuffer) != iSectorCount){
		Printf("Write Fail\n");
		return ;
	}
	Printf("LBA [%d], [%d] Sector Write Success.\n", dwLBA, iSectorCount);
	FreeMemory(pcBuffer);
}










