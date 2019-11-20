#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "Types.h"

// structure
#pragma pack(push,1)

typedef struct QueueManagerStruct{
	// Each data size of Queue
	int iDataSize;
	// Max index of Queue
	int iMaxDataCount;

	// Queue buffer point and insert/remove index
	void* pvQueueArray;
	int iPutIndex;
	int iGetIndex;

	// Last Operation
	// TRUE : INSERT
	// FALSE : REMOVE
	BOOL bLastOperationPut;
}QUEUE;

#pragma pack(pop)

//Function
void InitializeQueue(QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, int iDataSize);
BOOL IsQueueFull(const QUEUE* pstQueue);
BOOL IsQueueEmpty(const QUEUE* pstQueue);
BOOL PutQueue(QUEUE* pstQueue, const void* pvData);
BOOL GetQueue(QUEUE* pstQueue, void* pvData);

#endif /*__QUEUE_H__*/


