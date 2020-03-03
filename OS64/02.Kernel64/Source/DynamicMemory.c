#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"

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
	gs_stDynamicMemory.qwEndAddress = CalculateDynamicMemorySize() + DYNAMICMEMORY_START_ADDRESS;
	gs_stDynamicMemory.qwUsedSize = 0;

}








