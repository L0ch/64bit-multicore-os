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
	i = GETTCBOFFSET(qwID);

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
	pvStackAddress = (void*) (TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * GETTCBOFFSET(pstTask->stLink.qwID)));

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
	int i;

	InitializeTCBPool();
	for(i=0; i<TASK_MAXREADYLISTCOUNT ;i++ ){
		InitializeList(&(gs_stScheduler.vstReadyList[i]));
		gs_stScheduler.viExecuteCount[i] = 0;
	}
	InitializeList(&(gs_stScheduler.stWaitList));


	// Allocate TCB and set running task
	// Prepare TCB for Booting task (priority : HIGHEST)
	gs_stScheduler.pstRunningTask = AllocateTCB();
	gs_stScheduler.pstRunningTask->qwFlags = TASK_FLAGS_HIGHEST;

	// Initialize processor utilization structure
	gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
	gs_stScheduler.qwProcessorLoad = 0;

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
	TCB* pstTarget = NULL;
	int iTaskCount, i, j;

	// If tasks in all queues run once, fail to select task
	// -> if pstTarget is NULL, loop one more time
	for(j=0; j<2; j++){
		// Select task to schedule, from high to low priority
		for(i=0; i<TASK_MAXREADYLISTCOUNT; i++){
			iTaskCount = GetListCount(&(gs_stScheduler.vstReadyList[i]));

			// If task count > execute count , execute current priority task
			if(gs_stScheduler.viExecuteCount[i] < iTaskCount){
				pstTarget = (TCB*) RemoveListFromHead(&(gs_stScheduler.vstReadyList[i]));
				gs_stScheduler.viExecuteCount[i]++;
				break;
			}
			// Initialize execute count and concession next priority
			else{
				gs_stScheduler.viExecuteCount[i] = 0;
			}
		}
		if(pstTarget != NULL){
			break;
		}
	}
	return pstTarget;
}

// Add task to ready list of scheduler
BOOL AddTaskToReadyList(TCB* pstTask){
	BYTE bPriority;

	bPriority = GETPRIORITY(pstTask->qwFlags);
	if(bPriority >= TASK_MAXREADYLISTCOUNT){
		return FALSE;
	}
	AddListToTail(&(gs_stScheduler.vstReadyList[bPriority]), pstTask);
	return TRUE;
}

TCB* RemoveTaskFromReadyList(QWORD qwTaskID){
	TCB* pstTarget;
	BYTE bPriority;

	// Task ID not available (in wait list)
	if(GETTCBOFFSET(qwTaskID) >= TASK_MAXCOUNT){
		return NULL;
	}
	// Check task ID from TCB
	pstTarget = &(gs_stTCBPoolManager.pstStartAddress[GETTCBOFFSET(qwTaskID)]);
	if(pstTarget->stLink.qwID != qwTaskID){
		return NULL;
	}

	// Remove task in ready list
	bPriority = GETPRIORITY(pstTarget->qwFlags);

	pstTarget = RemoveList(&(gs_stScheduler.vstReadyList[bPriority]), qwTaskID);
	return pstTarget;
}

BOOL ChangePriority(QWORD qwTaskID, BYTE bPriority){
	TCB* pstTarget;

	if(bPriority > TASK_MAXREADYLISTCOUNT){
		return FALSE;
	}

	// If running task, change priority
	// Move set priority when task change
	pstTarget = gs_stScheduler.pstRunningTask;
	if(pstTarget->stLink.qwID == qwTaskID){
		SETPRIORITY(pstTarget->qwFlags, bPriority);
	}
	// If not running task, find in ready list
	else{
		pstTarget = RemoveTaskFromReadyList(qwTaskID);
		if(pstTarget == NULL){
			// Find in task pool by task ID
			pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
			if(pstTarget != NULL){
				// Set priority
				SETPRIORITY(pstTarget->qwFlags, bPriority);
			}
		}
		else{
			// Set priority and insert into ready list
			SETPRIORITY(pstTarget->qwFlags, bPriority);
			AddTaskToReadyList(pstTarget);
		}
	}
	return TRUE;
}

// Change task
// Do not call when Interrupt/exception occur
void Schedule(void){
	TCB* pstRunningTask, *pstNextTask;
	BOOL bPreviousFlag;

	// Task does not exist to change
	if(GetReadyTaskCount() < 0){
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

	// Update running task and switch context
	pstRunningTask = gs_stScheduler.pstRunningTask;
	gs_stScheduler.pstRunningTask = pstNextTask;

	// If switch from idle task, increase processor usage time
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE){
		gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;
	}

	// Update processor usage time
	gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

	// If task end flag available, don't need to save context.
	// Insert into wait list and switch context
	if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK){
		AddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
		SwitchContext(NULL, &(pstNextTask->stContext));
	}
	else{
		AddTaskToReadyList(pstRunningTask);
		SwitchContext(&(pstRunningTask->stContext), &(pstNextTask->stContext));
	}

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


	pstRunningTask = gs_stScheduler.pstRunningTask;
	// Set running task to next task
	gs_stScheduler.pstRunningTask = pstNextTask;

	// If switch from idle task, increase processor usage time
	if((pstRunningTask->qwFlags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE){
		gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
	}

	// If task end flag available, don't need to save context.
	// Insert into wait list
	if(pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK){
		AddListToTail(&(gs_stScheduler.stWaitList), pstRunningTask);
	}
	// Get current task and copy context in IST
	// Add to ready list
	else{
		MemCpy(&(pstRunningTask->stContext), pcContextAddress, sizeof(CONTEXT));
		AddTaskToReadyList(pstRunningTask);
	}

	// Copy context to IST
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

// End task
BOOL EndTask(QWORD qwTaskID){
	TCB* pstTarget;
	BYTE bPriority;

	// If running task, set EndTask bit and switch task
	pstTarget = gs_stScheduler.pstRunningTask;
	if(pstTarget->stLink.qwID == qwTaskID){
		pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
		SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);

		Schedule();
		// Never execute this code
		while(1);
	}
	// If task is not running, find in ready list and insert into wait list
	else{
		pstTarget = RemoveTaskFromReadyList(qwTaskID);
		// does not exist in ready list
		if(pstTarget == NULL){
			pstTarget = GetTCBInTCBPool(GETTCBOFFSET(qwTaskID));
			if(pstTarget != NULL){
				pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
				SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
			}
			return FALSE;
		}
		pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
		SETPRIORITY(pstTarget->qwFlags, TASK_FLAGS_WAIT);
		AddListToTail(&(gs_stScheduler.stWaitList), pstTarget);
	}
	return TRUE;
}

// Task exit itself
void ExitTask(void){
	EndTask(gs_stScheduler.pstRunningTask->stLink.qwID);
}

// Return task count in ready queue
int GetReadyTaskCount(void){
	int iTotalCount = 0;
	int i;

	// Check all ready queue
	for(i=0; i<TASK_MAXREADYLISTCOUNT; i++){
		iTotalCount += GetListCount(&(gs_stScheduler.vstReadyList[i]));
	}

	return iTotalCount;
}

// Return total task count
int GetTaskCouont(void){
	int iTotalCount;

	// task count in ready queue + wait task + running task(1)
	iTotalCount = GetReadyTaskCount();
	iTotalCount += GetListCount(&(gs_stScheduler.stWaitList)) + 1;

	return iTotalCount;
}

TCB* GetTCBInTCBPool(int iOffset){
	if((iOffset < -1) || (iOffset > TASK_MAXCOUNT)){
		return NULL;
	}
	return &(gs_stTCBPoolManager.pstStartAddress[iOffset]);
}

// Return is task exist
BOOL IsTaskExist(QWORD qwID){
	TCB* pstTCB;

	pstTCB = GetTCBInTCBPool(GETTCBOFFSET(qwID));
	if((pstTCB == NULL) || (pstTCB->stLink.qwID != qwID)){
		return FALSE;
	}
	return TRUE;
}

QWORD GetProcessorLoad(void){
return gs_stScheduler.qwProcessorLoad;
}


//==============================================================
//						Idle Task
//==============================================================

// Idle task
// Free TCB in wait task
// Calculate processor load %
void IdleTask(void){
	TCB* pstTask;
	QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
	QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;

	// For calc processor load
	qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
	qwLastMeasureTickCount = GetTickCount();

	while(1){
		qwCurrentMeasureTickCount = GetTickCount();
		qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

		// Processor load percentage
		if(qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0){
			gs_stScheduler.qwProcessorLoad = 0;
		}
		// Idle task /
		else{
			gs_stScheduler.qwProcessorLoad = 100 - (qwCurrentMeasureTickCount - qwLastSpendTickInIdleTask) *
												100 / (qwCurrentMeasureTickCount - qwLastMeasureTickCount);
		}

		qwLastMeasureTickCount = qwCurrentMeasureTickCount;
		qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

		HaltProcessorByLoad();

		if(GetListCount(&(gs_stScheduler.stWaitList)) >= 0){
			while(1){
				pstTask  = RemoveListFromHead(&(gs_stScheduler.stWaitList));
				if(pstTask == NULL){
					break;
				}
				Printf("IDLE: Task ID[0x%q] is completely ended.\n", pstTask->stLink.qwID);
				FreeTCB(pstTask->stLink.qwID);
			}
		}
		Schedule();
	}
}


// Halt processor by processor load(percentage)
void HaltProcessorByLoad(void){
	if(gs_stScheduler.qwProcessorLoad < 40){
		Hlt();
		Hlt();
		Hlt();
	}
	else if(gs_stScheduler.qwProcessorLoad < 80){
		Hlt();
		Hlt();
	}
	else if(gs_stScheduler.qwProcessorLoad < 95){
		Hlt();
	}
}











