#ifndef __SYNCHRONIZATION_H__
#define __SYNCHRONIZATION_H__

#include "Types.h"

// Structure

#pragma pack(push, 1)

typedef struct MutexStruct{
	volatile QWORD qwTaskID;
	volatile DWORD dwLockCount;

	volatile BOOL bLockFlag;

	BYTE vbPadding[3];
} MUTEX;

#pragma pack(pop)

// Function
BOOL LockForSystemData(void);
void UnlockForSystemData(BOOL bInterruptFlag);
void InitializeMutex(MUTEX* pstMudex);
void Lock(MUTEX* pstMutex);
void Unlock(MUTEX* pstMutex);

#endif /*__SYNCHRONIZATION_H__*/
