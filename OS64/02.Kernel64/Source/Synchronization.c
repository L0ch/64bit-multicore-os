#include "Synchronization.h"
#include "AssemblyUtility.h"
#include "Utility.h"
#include "Task.h"

BOOL LockForSystemData(void){
	return SetInterruptFlag(FALSE);
}

void UnlockForSystemData(BOOL bInterruptFlag){
	SetInterruptFlag(bInterruptFlag);
}

void InitialzieMutex(MUTEX* pstMutex){
	pstMutex->bLockFlag = FALSE;
	pstMutex->dwLockCount = 0;
	pstMutex->qwTaskID = TASK_INVALIDID;
}

void Lock(MUTEX* pstMutex){
	// If already locked
	if(TestAndSet(&(pstMutex->bLockFlag), 0, 1) == FALSE){
		// If task itself lock, increase count
		if(pstMutex->qwTaskID == GetRunningTask()->stLink.qwID){
			pstMutex->dwLockCount++;
			return ;
		}

		// If other task lock, wait until unlocked
		while(TestAndSet((&pstMutex->bLockFlag), 0, 1) == FALSE){
			Schedule();
		}
	}

	// Set lock, processing bLockFlag in TestAndSet() (Line 21)
	pstMutex->dwLockCount = 1;
	pstMutex->qwTaskID = GetRunningTask()->stLink.qwID;
}

void Unlock(MUTEX* pstMutex){
	// Fail - the task did not lock mutex
	if( (pstMutex->bLockFlag == FALSE) || (pstMutex->qwTaskID != GetRunningTask()->stLink.qwID) ){
		return ;
	}

	if(pstMutex->dwLockCount > 1){
		pstMutex->dwLockCount--;
		return ;
	}
	// Set unlock
	pstMutex->qwTaskID = TASK_INVALIDID;
	pstMutex->dwLockCount = 0;
	pstMutex->bLockFlag = FALSE;




}


