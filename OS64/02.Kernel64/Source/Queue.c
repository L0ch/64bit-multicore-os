#include "Queue.h"

// Initialize Queue
void kInitializeQueue(QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, int iDataSize){
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
BOOL kIsQueueFull(const QUEUE*pstQueue){
	if((pstQueue->iGetIndex == pstQueue->iPutIndex) && (pstQueue->bLastOperationPut == TRUE)){
		return TRUE;
	}
	return FALSE;
}

// Return is Queue empty
BOOL kIsQueueEmpty(const QUEUE* pstQueue){
	if((pstQueue->iGetIndex == pstQueue->iPutIndex) && (pstQueue->bLastOperationPut == FALSE)){
		return TRUE;
	}
	return FALSE;
}

// Insert Queue
BOOL kPutQueue(QUEUE* pstQueue, const void* pvData){
	// If Queue is full, Do not Insert
	if(kIsQueueFull(pstQueue) == TRUE){
		return FALSE;
	}
	// Copy
	kMemCpy((char*)pstQueue->pvQueueArray + (pstQueue->iDataSize*pstQueue->iPutIndex), pvData, pstQueue->iDataSize);

	// Increase insert index, if over than max index of queue, reset to 0
	pstQueue->iPutIndex = (pstQueue->iPutIndex+1) % pstQueue->iMaxDataCount;
	// Save last operation
	pstQueue->bLastOperationPut = TRUE;
	return TRUE;

}

// Remove Queue
BOOL kGetQueue(QUEUE* pstQueue, const void* pvData){
	if(kIsQueueEmpty(pstQueue) == TRUE){
		return FALSE;
	}
	// Copy
	kMemCpy(pvData, (char*)pstQueue->pvQueueArray + (pstQueue->iDataSize*pstQueue->iGetIndex), pstQueue->iDataSize);
	// Increase remove index, if over than max index of queue, reset to 0
	pstQueue->iGetIndex = (pstQueue->iGetIndex+1) % pstQueue->iMaxDataCount;
	//Save last operation
	pstQueue->bLastOperationPut = FALSE;
	return TRUE;

}










