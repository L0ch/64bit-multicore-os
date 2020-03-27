#ifndef __DYNAMICMEMORY_H__
#define __DYNAMICMEMORY_H__

#include "Types.h"

// Macro
// Sort by 1MB
#define DYNAMICMEMORY_START_ADDRESS	((TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * TASK_MAXCOUNT) + 0xfffff) & 0xfffffffffff00000)
#define DYNAMICMEMORY_MIN_SIZE		(1 * 1024)

// Bitmap flag
#define DYNAMICMEMORY_EXIST			0x01
#define DYNAMICMEMORY_EMPTY			0x00

// structure
// Bitmap management
typedef struct BitmapStruct{
	BYTE* pbBitmap;
	QWORD qwExistBitCount;

} BITMAP;

// Buddy Block management
typedef struct DynamicMemoryManagerStruct{

	// Total block list, smallest block count, used memory size
	int iMaxLevelCount;
	int iBlockCountOfSmallestBlock;
	QWORD qwUsedSize;

	// Block pool start/end address
	QWORD qwStartAddress;
	QWORD qwEndAddress;

	// Block list index of allocated memory, bitmap structure address
	BYTE* pbAllocatedBlockListIndex;
	BITMAP* pstBitmapOfLevel;
} DYNAMICMEMORY;

// Function

void InitializeDynamicMemory(void);
void* AllocateMemory(QWORD qwSize);
BOOL FreeMemory(void* pvAddress);
void GetDynamicMemoryInfo(QWORD* pqwDynamicMemoryStartAddress, QWORD* pqwDynamicMemoryTotalSize,
							QWORD* pqwMetaDataSize, QWORD* pqwUsedMemorySize);
DYNAMICMEMORY* GetDynamicMemoryManager(void);

static QWORD CalcDynamicMemorySize(void);
static int CalcMetaBlockCount(QWORD qwDynamicRAMSize);
static int AllocationBuddyBlock(QWORD qwAlignedSize);
static QWORD GetBuddyBlockSize(QWORD qwSize);
static int GetBlockListIndexOfMatchSize(QWORD qwAlignedSize);
static int FindFreeBlockInBitmap(int iBlockListIndex);
static void SetFlagInBitmap(int iBlockListIndex, int iOffset, BYTE bFlag);
static BOOL FreeBuddyBlock(int iBlockListIndex, int iBlockOffset);
static BYTE GetFlagInBitmap(int iBlockListIndex, int iOffset);

#endif /*__DYNAMICMEMORY_H__*/

