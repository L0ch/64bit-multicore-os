#include "Task.h"
#include "Descriptor.h"
#include "Utility.h"
#include "AssemblyUtility.h"

static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

//=====================================================
//					Task pool/Task
//=====================================================

// Task pool Initialize
void InitializeTCBPool(void){
	int i;

	MemSet(&(gs_stTCBPoolManager), 0, sizeof(gs_stTCBPoolManager));

	// Set Task pool address
	gs_stTCBPoolManager.pstStartAddress = (TCB*) TASK_TCBPOOLADDRESS;
	MemSet((void*)TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);

	// Allocate ID to TCB
	for(i=0; i<TASK_MAXCOUNT; i++){
		gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;
	}

	// Initialize max TCB/allocated count
	gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
	gs_stTCBPoolManager.iAllocatedCount = 1;
}
// Allocate TCB
TCB* AllocateTCB(void){
	TCB* pstEmptyTCB;
	int i;

	if (gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount){
		return NULL;
	}
	for(i=0; i<gs_stTCBPoolManager.iMaxCount; i++){
		// If upper 32bit is 0, not allocated TCB
		if((gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID >> 32) == 0){
			pstEmptyTCB = &(gs_stTCBPoolManager.pstStartAddress[i]);
			break;
		}
	}
	// Set to Allocated TCB
	pstEmptyTCB->stLink.qwID = ((QWORD) gs_stTCBPoolManager.iAllocatedCount << 32) | i;
	gs_stTCBPoolManager.iUseCount++;
	gs_stTCBPoolManager.iAllocatedCount++;
	if(gs_stTCBPoolManager.iAllocatedCount == 0){
		gs_stTCBPoolManager.iAllocatedCount = 1;
	}
	return pstEmptyTCB;
}

// Deallocate TCB
void FreeTCB(QWORD qwID){
	int i;

	// Lower 32bit - index
	i = qwID & 0xFFFFFFFF;

	// Initialize TCB and set ID
	MemSet(&(gs_stTCBPoolManager.pstStartAddress[i].stContext), 0, sizeof(CONTEXT));
	gs_stTCBPoolManager.pstStartAddress[i].stLink.qwID = i;

	gs_stTCBPoolManager.iUseCount--;
}

// Create Task
// Allocate stack from stack pool by task ID
TCB* CreateTask(QWORD qwFlags, QWORD qwEntryPointAddress){
	TCB* pstTask;
	void* pvStackAddress;

	pstTask = AllocateTCB();
	if(pstTask == NULL){
		return NULL;
	}

	// Calc stack address by task ID (lower 32bit - stack pool offset)
	pvStackAddress = (void*) (TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * (pstTask->stLink.qwID & 0xFFFFFFFF)));

	// Set TCB
	SetUpTask(pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE);
	// Insert ready list for scheduling
	AddTaskToReadyList(pstTask);

	return pstTask;
}

void SetUpTask(TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize){
	// Initialize Context
	MemSet(pstTCB->stContext.vqRegister, 0, sizeof(pstTCB->stContext.vqRegister));

	// Initialize stack pointer register
	pstTCB->stContext.vqRegister[TASK_RSPOFFSET] = (QWORD) pvStackAddress + qwStackSize;	// RSP
	pstTCB->stContext.vqRegister[TASK_RBPOFFSET] = (QWORD) pvStackAddress + qwStackSize;	// RBP

	// Set segment selector
	pstTCB->stContext.vqRegister[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;	// CS
	pstTCB->stContext.vqRegister[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;	// DS
	pstTCB->stContext.vqRegister[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;	// ES
	pstTCB->stContext.vqRegister[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;	// FS
	pstTCB->stContext.vqRegister[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;	// GS
	pstTCB->stContext.vqRegister[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;	// SS


	// Set RIP register
	pstTCB->stContext.vqRegister[TASK_RIPOFFSET] = qwEntryPointAddress;

	// Set IF(bit 9) 1 to enable interrupt
	pstTCB->stContext.vqRegister[TASK_RFLAGSOFFSET] |= 0x0200;

	// ID/Stack, Flags
	//pstTCB->qwID = qwID;
	pstTCB->pvStackAddress = pvStackAddress;
	pstTCB->qwStackSize = qwStackSize;
	pstTCB->qwFlags = qwFlags;
}


//=====================================================
//						Scheduler
//=====================================================

// Initialize scheduler
void InitializeScheduler(void){
	InitializeTCBPool();

	InitializeList(&(gs_stScheduler.stReadyList));

	// Allocate TCB and set running task
	// Prepare TCB for Booting task
	gs_stScheduler.pstRunningTask = AllocateTCB();
}

// Set running task
void SetRunningTask(TCB* pstTask){
	gs_stScheduler.pstRunningTask = pstTask;
}

// Return running task
TCB* GetRunningTask(void){
	return gs_stScheduler.pstRunningTask;
}

// Get next task from task list
TCB* GetNextTaskToRun(void){
	if(GetListCount(&(gs_stScheduler.stReadyList)) == 0){
		return NULL;
	}

	return (TCB*) RemoveListFromHead(&(gs_stScheduler.stReadyList));
}

// Add task to ready list of scheduler
void AddTaskToReadyList(TCB* pstTask){
	AddListToTail(&(gs_stScheduler.stReadyList), pstTask);
}

// Change task
// Do not call when Interrupt/exception occur
void Schedule(void){
	TCB* pstRunningTask, *pstNextTask;
	BOOL bPreviousFlag;

	// Task does not exist to change
	if(GetListCount(&(gs_stScheduler.stReadyList)) == 0){
		return ;
	}

	// Interrupt disable
	bPreviousFlag = SetInterruptFlag(FALSE);
	// Get next task
	pstNextTask = GetNextTaskToRun();
	if(pstNextTask == NULL){

		SetInterruptFlag(bPreviousFlag);
		return ;
	}


	pstRunningTask = gs_stScheduler.pstRunningTask;
	AddTaskToReadyList(pstRunningTask);

	// Update processor usage time
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

	// Set running task to next task and change context
	gs_stScheduler.pstRunningTask = pstNextTask;
	SwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));


	// Interrupt enable
	SetInterruptFlag(bPreviousFlag);

}

BOOL ScheduleInInterrupt(void){
	TCB* pstRunningTask, * pstNextTask;
	char* pcContextAddress;

	// Task does not exist to change
	pstNextTask = GetNextTaskToRun();
	if(pstNextTask == NULL){
		return FALSE;
	}
	// Processing task switch
	// Overwrite context saved by interrupt handler with other context
	pcContextAddress = (char*) IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

	// Get current task and copy context in IST
	// Add to ready list
	pstRunningTask = gs_stScheduler.pstRunningTask;
	MemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
	AddTaskToReadyList(pstRunningTask);

	// Set running task to next task
	// Copy context to IST
	gs_stScheduler.pstRunningTask = pstNextTask;
	MemCpy(pcContextAddress, &(pstNextTask->stContext), sizeof(CONTEXT));

	// Update processor usage time
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
	return TRUE;

}

// -1ms
void DecreaseProcessorTime(void){
	if(gs_stScheduler.iProcessorTime > 0){
		gs_stScheduler.iProcessorTime--;
	}
}

BOOL IsProcessorTimeExpired(void){
	if(gs_stScheduler.iProcessorTime <=0){
		return TRUE;
	}
	return FALSE;
}




