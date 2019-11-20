#include "Queue.h"
#include "Utility.h"

// Initialize Queue
void InitializeQueue(QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, int iDataSize){
	// Save Max Queue index/buffer address
	pstQueue->iMaxDataCount = iMaxDataCount;
	pstQueue->iDataSize = iDataSize;
	pstQueue->pvQueueArray = pvQueueBuffer;

	// Initialize insert/remove index
	pstQueue->iPutIndex = 0;
	pstQueue->iPutIndex = 0;
	pstQueue->bLastOperationPut = FALSE;
}

// Return is Queue full
BOOL IsQueueFull(const QUEUE*pstQueue){
	if((pstQueue->iGetIndex == pstQueue->iPutIndex) && (pstQueue->bLastOperationPut == TRUE)){
		return TRUE;
	}
	return FALSE;
}

// Return is Queue empty
BOOL IsQueueEmpty(const QUEUE* pstQueue){
	if((pstQueue->iGetIndex == pstQueue->iPutIndex) && (pstQueue->bLastOperationPut == FALSE)){
		return TRUE;
	}
	return FALSE;
}

// Insert Queue
BOOL PutQueue(QUEUE* pstQueue, const void* pvData){
	// If Queue is full, Do not Insert
	if(IsQueueFull(pstQueue) == TRUE){
		return FALSE;
	}
	// Copy
	MemCpy((char*)pstQueue->pvQueueArray + (pstQueue->iDataSize*pstQueue->iPutIndex), pvData, pstQueue->iDataSize);

	// Increase insert index, if over than max index of queue, reset to 0
	pstQueue->iPutIndex = (pstQueue->iPutIndex+1) % pstQueue->iMaxDataCount;
	// Save last operation
	pstQueue->bLastOperationPut = TRUE;
	return TRUE;

}

// Remove Queue
BOOL GetQueue(QUEUE* pstQueue, void* pvData){
	if(IsQueueEmpty(pstQueue) == TRUE){
		return FALSE;
	}
	// Copy
	MemCpy(pvData, (char*)pstQueue->pvQueueArray + (pstQueue->iDataSize*pstQueue->iGetIndex), pstQueue->iDataSize);
	// Increase remove index, if over than max index of queue, reset to 0
	pstQueue->iGetIndex = (pstQueue->iGetIndex+1) % pstQueue->iMaxDataCount;
	//Save last operation
	pstQueue->bLastOperationPut = FALSE;
	return TRUE;

}










