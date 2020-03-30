#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"
#include "Synchronization.h"

static DYNAMICMEMORY gs_stDynamicMemory;

// Initialize Dynamic memory area
void InitializeDynamicMemory(void){
	QWORD qwDynamicMemorySize;
	int i,j;
	BYTE* pbCurrentBitmapPosition;
	int iBlockCountOfLevel, iMetaBlockCount;

	// Calculate block management memory size in minimum blocks
	qwDynamicMemorySize = CalcDynamicMemorySize();
	iMetaBlockCount = CalcMetaBlockCount(qwDynamicMemorySize);

	// Set meta info of area except meta block count for management
	gs_stDynamicMemory.iBlockCountOfSmallestBlock = (qwDynamicMemorySize / DYNAMICMEMORY_MIN_SIZE) - iMetaBlockCount;

	// Calculate max block list count
	for(i=0; (gs_stDynamicMemory.iBlockCountOfSmallestBlock >> i) > 0; i++){
		;
	}
	gs_stDynamicMemory.iMaxLevelCount = i;

	// Initialize index of block list area
	gs_stDynamicMemory.pbAllocatedBlockListIndex = (BYTE*) DYNAMICMEMORY_START_ADDRESS;
	for(i=0; i<gs_stDynamicMemory.iBlockCountOfSmallestBlock; i++){
		gs_stDynamicMemory.pbAllocatedBlockListIndex[i] = 0xFF;
	}

	// Set bitmap structure start address
	gs_stDynamicMemory.pstBitmapOfLevel = (BITMAP*) (DYNAMICMEMORY_START_ADDRESS +
							(sizeof(BYTE) * gs_stDynamicMemory.iBlockCountOfSmallestBlock));
	// Set bitmap address
	pbCurrentBitmapPosition = ((BYTE*) gs_stDynamicMemory.pstBitmapOfLevel +
							(sizeof(BITMAP) * gs_stDynamicMemory.iMaxLevelCount));
	// Create bitmap for each block list
	// Largest block/remaining block in initial state, set others empty
	for(j=0; j<gs_stDynamicMemory.iMaxLevelCount; j++){
		gs_stDynamicMemory.pstBitmapOfLevel[j].pbBitmap = pbCurrentBitmapPosition;
		gs_stDynamicMemory.pstBitmapOfLevel[j].qwExistBitCount = 0;
		iBlockCountOfLevel = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> j;

		// Left over 8 blocks, can combine into parent block -> set empty
		for(i=0; i<iBlockCountOfLevel/8; i++){
			*pbCurrentBitmapPosition = 0x00;
			pbCurrentBitmapPosition++;
		}

		// If remaining blocks are odd, last block cannot combined
		if((iBlockCountOfLevel % 8) != 0){
			*pbCurrentBitmapPosition = 0x00;

			i = iBlockCountOfLevel % 8;
			if((i%2) == 1){
				*pbCurrentBitmapPosition |= (DYNAMICMEMORY_EXIST << (i - 1));
				gs_stDynamicMemory.pstBitmapOfLevel[j].qwExistBitCount = 1;
			}
			pbCurrentBitmapPosition++;
		}

	}
	// Set block pool address and used memory size
	gs_stDynamicMemory.qwStartAddress = DYNAMICMEMORY_START_ADDRESS + iMetaBlockCount * DYNAMICMEMORY_MIN_SIZE;
	gs_stDynamicMemory.qwEndAddress = CalcDynamicMemorySize() + DYNAMICMEMORY_START_ADDRESS;
	gs_stDynamicMemory.qwUsedSize = 0;

}

// Get dynamic memory area size
static QWORD CalcDynamicMemorySize(void){
	QWORD qwRAMSize;

	// Max RAM size limit is 3GB
	qwRAMSize = (GetTotalMemorySize() * 1024 * 1024);
	if(qwRAMSize > (QWORD) 3* 1024 * 1024 * 1024){
		qwRAMSize = (QWORD) 3* 1024 * 1024 * 1024;
	}
	return qwRAMSize - DYNAMICMEMORY_START_ADDRESS;

}

// Get meta info size for management dynamic memory
static int CalcMetaBlockCount(QWORD qwDynamicRAMSize){
	long lBlockCountOfSmallestBlock;
	DWORD dwSizeOfAllocatedBlockListIndex;
	DWORD dwSizeOfBitmap;
	long i;

	// Calculate bitmap/allocated block list index
	// based on smallest number of blocks
	lBlockCountOfSmallestBlock = qwDynamicRAMSize / DYNAMICMEMORY_MIN_SIZE;
	dwSizeOfAllocatedBlockListIndex = lBlockCountOfSmallestBlock * sizeof(BYTE);

	// Get bitmap size
	dwSizeOfBitmap = 0;
	for(i=0; (lBlockCountOfSmallestBlock >> i) > 0; i++){
		// For block list bitmap pointer
		dwSizeOfBitmap += sizeof(BITMAP);
		// Block list bitmap size (round up byte)
		dwSizeOfBitmap += ((lBlockCountOfSmallestBlock >> i) +7 ) / 8;
	}

	// Return used memory size (round up smallest block size)
	return (dwSizeOfAllocatedBlockListIndex + dwSizeOfBitmap + DYNAMICMEMORY_MIN_SIZE - 1) / DYNAMICMEMORY_MIN_SIZE;

}


// Allocate memory
void* AllocateMemory(QWORD qwSize){
	QWORD qwAlignedSize;
	QWORD qwRelativeAddress;
	long lOffset;
	int iSizeArrayOffset;
	int iIndexOfBlockList;

	// Get buddy block size
	qwAlignedSize = GetBuddyBlockSize(qwSize);
	if(qwAlignedSize == 0){
		return NULL;
	}

	// Not enough space
	if(gs_stDynamicMemory.qwStartAddress + gs_stDynamicMemory.qwUsedSize +
			qwAlignedSize > gs_stDynamicMemory.qwEndAddress) return NULL;

	// Get block list index
	lOffset = AllocationBuddyBlock(qwAlignedSize);
	if(lOffset == -1){
		return NULL;
	}

	iIndexOfBlockList = GetBlockListIndexOfMatchSize(qwAlignedSize);

	// Use block list index when free memory
	qwRelativeAddress = qwAlignedSize * lOffset;
	iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;

	gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] = (BYTE) iIndexOfBlockList;
	gs_stDynamicMemory.qwUsedSize += qwAlignedSize;

	return (void*)(qwRelativeAddress + gs_stDynamicMemory.qwStartAddress);

}

// Return sorted size by nearest buddy block
static QWORD GetBuddyBlockSize(QWORD qwSize){
	long i;

	for(i=0; i<gs_stDynamicMemory.iMaxLevelCount; i++){
		if(qwSize <= (DYNAMICMEMORY_MIN_SIZE << i)){
			return (DYNAMICMEMORY_MIN_SIZE << i);
		}
	}
	return 0;
}


// Allocate using buddy block
// param1 : buddy block size
static int AllocationBuddyBlock(QWORD qwAlignedSize){
	int iBlockListIndex, iFreeOffset;
	int i;
	BOOL bPreviousInterruptFlag;

	// Search block list index satisfying block size
	iBlockListIndex = GetBlockListIndexOfMatchSize(qwAlignedSize);
	if(iBlockListIndex == -1){
		return -1;
	}

	// Start Critical Section
	bPreviousInterruptFlag = LockForSystemData();

	// Select block
	for(i=iBlockListIndex; i<gs_stDynamicMemory.iMaxLevelCount; i++){
		// Validation in bitmap
		iFreeOffset = FindFreeBlockInBitmap(i);
		if(iFreeOffset != -1){
			break;
		}
	}
	// Validation failed
	if(iFreeOffset == -1){
		// End Critical Section
		UnlockForSystemData(bPreviousInterruptFlag);
		return -1;
	}
	SetFlagInBitmap(i, iFreeOffset, DYNAMICMEMORY_EMPTY);

	// Spirit if higher block
	if(i > iBlockListIndex){
		// Left block->empty
		// Right block->exist
		for(i=i-1; i>=iBlockListIndex; i--){
			SetFlagInBitmap(i, iFreeOffset*2, DYNAMICMEMORY_EMPTY);
			SetFlagInBitmap(i, iFreeOffset*2+1, DYNAMICMEMORY_EXIST);
			// Spirit left block
			iFreeOffset = iFreeOffset * 2;
		}
	}
	// End Critical Section
	UnlockForSystemData(bPreviousInterruptFlag);
	return iFreeOffset;
}

// Return nearest block list index
static int GetBlockListIndexOfMatchSize(QWORD qwAlignedSize){
	int i;

	for(i=0; i<gs_stDynamicMemory.iMaxLevelCount; i++){
		if(qwAlignedSize <= (DYNAMICMEMORY_MIN_SIZE << i)){
			return i;
		}
	}
	return -1;
}

static int FindFreeBlockInBitmap(int iBlockListIndex){
	int i, iMaxCount;
	BYTE* pbBitmap;
	QWORD* pqwBitmap;

	// Fail if data not exist in bitmap
	if(gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount == 0){
		return -1;
	}

	// Search bitmap as total blocks
	iMaxCount = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> iBlockListIndex;
	pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
	for(i=0; i<iMaxCount; ){

		// Check in 64bit units
		if(((iMaxCount - i)/64) > 0){
			pqwBitmap = (QWORD*) &(pbBitmap[i / 8]);
			// if 0, except
			if(*pqwBitmap == 0){
				i += 64;
				continue;
			}
		}
		if((pbBitmap[i / 8] & (DYNAMICMEMORY_EXIST << (i % 8))) != 0){
			return i;
		}
		i++;
	}
	return i;
}

// Set flag in bitmap
static void SetFlagInBitmap(int iBlockListIndex, int iOffset, BYTE bFlag){
	BYTE* pbBitmap;

	pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
	if(bFlag == DYNAMICMEMORY_EXIST){
		// If data does not exist, increase bit count
		if((pbBitmap[iOffset/8] & (0x01 << (iOffset%8))) == 0){
			gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount++;
		}
		pbBitmap[iOffset/8] |= (0x01 << (iOffset%8));
	}
	else{
		// If data exist, decrease bit count
		if((pbBitmap[iOffset/8] & (0x01 << (iOffset%8))) !=0){
			gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].qwExistBitCount--;
		}
		pbBitmap[iOffset/8] &= ~(0x01 << (iOffset%8));
	}

}

// Free allocated memory
BOOL FreeMemory(void* pvAddress){
	QWORD qwRelativeAddress;
	int iSizeArrayOffset;
	QWORD qwBlockSize;
	int iBlockListIndex;
	int iBitmapOffset;

	if(pvAddress == NULL){
		return FALSE;
	}

	// Convert address to based on block pool
	// Find block size
	qwRelativeAddress = ((QWORD)pvAddress) - gs_stDynamicMemory.qwStartAddress;
	iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;

	// If not allocated
	if(gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] == 0xFF){
		return FALSE;
	}

	// Get block list index block belongs to
	// Initialize block list
	iBlockListIndex = (int) gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset];
	gs_stDynamicMemory.pbAllocatedBlockListIndex[iSizeArrayOffset] = 0xFF;
	// Calculate allocated block size
	qwBlockSize = DYNAMICMEMORY_MIN_SIZE << iBlockListIndex;

	// Get block offset, free block
	iBitmapOffset = qwRelativeAddress / qwBlockSize;
	if(FreeBuddyBlock(iBlockListIndex, iBitmapOffset) == TRUE){
		gs_stDynamicMemory.qwUsedSize -= qwBlockSize;
		return TRUE;
	}
	return FALSE;
}


// Free buddy block in block list
static BOOL FreeBuddyBlock(int iBlockListIndex, int iBlockOffset){
	int iBuddyBlockOffset;
	int i;
	BOOL bFlag;
	BOOL bPreviousInterruptFlag;

	// Start critical section
	bPreviousInterruptFlag = LockForSystemData();

	// Combine adjacent block
	for(i=iBlockListIndex; i<gs_stDynamicMemory.iMaxLevelCount; i++){
		// Set block exist
		SetFlagInBitmap(i, iBlockOffset, DYNAMICMEMORY_EXIST);

		// Check adjacent block
		if((iBlockOffset % 2) == 0){
			iBuddyBlockOffset = iBlockOffset + 1;
		}
		else{
			iBuddyBlockOffset = iBlockOffset - 1;
		}
		bFlag = GetFlagInBitmap(i, iBuddyBlockOffset);

		// If adjacent block is empty, can not be combined
		if(bFlag == DYNAMICMEMORY_EMPTY){
			break;
		}

		// If adjacent block is exist, combine blocks
		SetFlagInBitmap(i, iBuddyBlockOffset, DYNAMICMEMORY_EMPTY);
		SetFlagInBitmap(i, iBlockOffset, DYNAMICMEMORY_EMPTY);

		// Upper level block list offset
		iBlockOffset = iBlockOffset / 2;
	}
	// End critical section
	UnlockForSystemData(bPreviousInterruptFlag);
	return TRUE;
}


// Return flag in bitmap of block
static BYTE GetFlagInBitmap(int iBlockListIndex, int iOffset){
	BYTE* pbBitmap;

	pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[iBlockListIndex].pbBitmap;
	if((pbBitmap[iOffset / 8] & (0x01 << (iOffset % 8))) != 0x00){
		return DYNAMICMEMORY_EXIST;
	}
	return DYNAMICMEMORY_EMPTY;
}

void GetDynamicMemoryInfo(QWORD* pqwDynamicMemoryStartAddress, QWORD* pqwDynamicMemoryTotalSize,
							QWORD* pqwMetaDataSize, QWORD* pqwUsedMemorySize){
	*pqwDynamicMemoryStartAddress = DYNAMICMEMORY_START_ADDRESS;
	*pqwDynamicMemoryTotalSize = CalcDynamicMemorySize();
	*pqwMetaDataSize = CalcMetaBlockCount(*pqwDynamicMemoryTotalSize) * DYNAMICMEMORY_MIN_SIZE;
	*pqwUsedMemorySize = gs_stDynamicMemory.qwUsedSize;
}

DYNAMICMEMORY* GetDynamicMemoryManager(void){
	return &gs_stDynamicMemory;
}



