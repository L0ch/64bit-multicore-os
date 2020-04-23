#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HDD.h"

void CommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode){
	char vcBuffer[3] = {0, };
	// Print interrupt vector
	vcBuffer[0] = '0' + iVectorNumber / 10;
	vcBuffer[1] = '0' + iVectorNumber % 10;

	PrintStringXY(0,0, "==============Exception Occur==============");
	PrintStringXY(0,1, "                                           ");
	PrintStringXY(0,1, "Vector: ");
	PrintStringXY(8,1, vcBuffer);
	PrintStringXY(0,2, "===========================================");

	while(1);
}

void CommonInterruptHandler(int iVectorNumber){
	char vcBuffer[] = "[INT:  , ]";
	static int g_iCommonInterruptCount=0;

	// Print interrupt vector
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8]= '0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
	PrintStringXY(70,0,vcBuffer);

	// Send EOI
	SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR); // Vector number - 0x20 = IRQ Number
}


void KeyboardHandler(int iVectorNumber){
	char vcBuffer[] = "[INT:  , ]";
	static int g_iKeyboardInterruptCount = 0;
	BYTE bTemp;

	// Print interrupt vector
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8]= '0' + g_iKeyboardInterruptCount;
	g_iKeyboardInterruptCount = (g_iKeyboardInterruptCount + 1) % 10;
	PrintStringXY(70,1,vcBuffer);

	if(IsOutputBufferFull() == TRUE){
		bTemp = GetKeyboardScanCode();
		ConvertScanCodeAndPutQueue(bTemp);
	}

	// Send EOI
	SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}


// Timer interrupt handler
void TimerHandler(int iVectorNumber){
	char vcBuffer[] = "[INT:  , ]";
	static int g_iTimerinterruptCount = 0;

	// Print interrupt vector number
	vcBuffer[5] = '0'+iVectorNumber/10;
	vcBuffer[6] = '0'+iVectorNumber%10;

	// Print count
	vcBuffer[8] = '0'+g_iTimerinterruptCount;
	g_iTimerinterruptCount = (g_iTimerinterruptCount +1) % 10;
	PrintStringXY(70, 0, vcBuffer);

	// Send EOI
	SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
	// Increase Timer count
	g_qwTickCount++;

	// Decrease processor time used by task
	DecreaseProcessorTime();
	if(IsProcessorTimeExpired() == TRUE){
		ScheduleInInterrupt();
	}
}

// Device Not Available Exception Handler
void DeviceNotAvailableHandler(int iVectorNumber){
	TCB* pstFPUTask, * pstCurrentTask;
	QWORD qwLastFPUTaskID;

	// Print message
	char vcBuffer[] = "[EXC:  , ]";
	static int g_iFPUInterruptCount = 0;

	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8] = '0' + g_iFPUInterruptCount;
	g_iFPUInterruptCount = (g_iFPUInterruptCount + 1) % 10;
	PrintStringXY(0, 0, vcBuffer);
	ClearTS();

	// Get Task ID use FPU
	qwLastFPUTaskID = GetLastFPUUsedTaskID();
	pstCurrentTask = GetRunningTask();

	// Current Task used FPU last
	if(qwLastFPUTaskID == pstCurrentTask->stLink.qwID){
		return ;
	}
	// Save FPU status of previous task
	else if(qwLastFPUTaskID != TASK_INVALIDID){
		pstFPUTask = GetTCBInTCBPool(GETTCBOFFSET(qwLastFPUTaskID));
		if((pstFPUTask != NULL) && (pstFPUTask->stLink.qwID) == qwLastFPUTaskID){
			SaveFPUContext(pstFPUTask->vqwFPUContext);
		}
	}
	// Current task never used FPU -> initialize FPU
	// Used FPU -> restore FPU
	if(pstCurrentTask->bFPUUsed == FALSE){
		InitializeFPU();
		pstCurrentTask->bFPUUsed = TRUE;
	}
	else{
		LoadFPUContext(pstCurrentTask->vqwFPUContext);
	}

	// Change Last FPU used task id to current task
	SetLastFPUUsedTaskID(pstCurrentTask->stLink.qwID);
}


void HDDHandler(int iVectorNumber){
	char vcBuffer[] = "[INT:  , ]";
	static int g_iHDDInterruptCount = 0;
	BYTE bTemp;

	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;

	vcBuffer[8] = '0' + g_iHDDInterruptCount;
	g_iHDDInterruptCount = (g_iHDDInterruptCount + 1) % 10;
	PrintStringXY(10, 0, vcBuffer);

	// IRQ 14
	if(iVectorNumber - PIC_IRQSTARTVECTOR == 14){
		SetHDDInterruptFlag(TRUE, TRUE);
	}
	// IRQ 15
	else{
		SetHDDInterruptFlag(FALSE, TRUE);
	}
	// EOI
	SendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}













