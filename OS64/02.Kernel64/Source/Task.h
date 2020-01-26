#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"

// Macro
// SS, RSP, RFLAGS, CS, RIP, 19 register
#define TASK_REGISTERCOUNT		24
#define TASK_REGISTERSIZE		8

// Register offset of Context structure
#define TASK_GSOFFSET		0
#define TASK_FSOFFSET		1
#define TASK_ESOFFSET		2
#define TASK_DSOFFSET		3
#define TASK_R15OFFSET		4
#define TASK_R14OFFSET		5
#define TASK_R13OFFSET		6
#define TASK_R12OFFSET		7
#define TASK_R11OFFSET		8
#define TASK_R1OOFFSET		9
#define TASK_R9OFFSET		10
#define TASK_R8OFFSET		11
#define TASK_RSIOFFSET		12
#define TASK_RDIOFFSET		13
#define TASK_RDXOFFSET		14
#define TASK_RCXOFFSET		15
#define TASK_RBXOFFSET		16
#define TASK_RAXOFFSET		17
#define TASK_RBPOFFSET		18
#define TASK_RIPOFFSET		19
#define TASK_CSOFFSET		20
#define TASK_RFLAGSOFFSET	21
#define TASK_RSPOFFSET		22
#define TASK_SSOFFSET		23

// TCB pool address - after 8MB (size < 1MB)
#define TASK_TCBPOOLADDRESS		0x800000
#define TASK_MAXCOUNT			1024

// Stack pool address - after 8MB + TCB size * TASK_MAXCOUNT (size = TASK_MAXCOUNT * 8KB)
#define TASK_STACKPOOLADDRESS	(TASK_TCBPOOLADDRESS + sizeof(TCB) * TASK_MAXCOUNT)
#define TASK_STACKSIZE			8192
#define TASK_INVALIDID			0xFFFFFFFFFFFFFFFF
// Maximum processor time task can use
#define TASK_PROCESSORTIME		5

#define TASK_MAXREADYLISTCOUNT	5

// Task Priority
#define TASK_FLAGS_HIGHEST		0
#define TASK_FLAGS_HIGH			1
#define TASK_FLAGS_MEDIUM		2
#define TASK_FLAGS_LOW			3
#define TASK_FLAGS_LOWEST		4
#define TASK_FLAGS_WAIT			0xFF

// Task flag
#define TASK_FLAGS_ENDTASK		0x8000000000000000
#define TASK_FLAGS_SYSTEM		0x4000000000000000
#define TASK_FLAGS_PROCESS		0x2000000000000000
#define TASK_FLAGS_THREAD		0x1000000000000000
#define TASK_FLAGS_IDLE			0x0800000000000000

// Function macro
#define GETPRIORITY(x) 				((x) & 0xFF)
#define SETPRIORITY(x, priority)	((x) = ((x) & 0xFFFFFFFFFFFFFF00) | (priority))
#define GETTCBOFFSET(x)				((x) & 0xFFFFFFFF)
// Get TCB address from stThreadLink
#define GETTCBFROMTHREADLINK(x)		(TCB*) ((QWORD)(x) - offsetof(TCB, stThreadLink))

// Structure
#pragma pack(push, 1)


typedef struct ContextStruct{
	QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;


// Control task status
// Add padding to be multiple of 16 (FPU)
typedef struct TaskControlBlockStruct{
	// Next Node
	LINKEDLIST stLink;

	// Flag
	QWORD qwFlags;

	void* pvMemoryAddress;
	QWORD qwMemorySize;


	/////////////////////////////////
	//     		Thread Info
	/////////////////////////////////
	// Child thread address/ID
	LINKEDLIST stThreadLink;

	// Parent process ID
	QWORD qwParentProcessID;


	QWORD vqwFPUContext[512 / 8];

	// Child thread list
	LIST stChildThreadList;

	// Context
	CONTEXT stContext;

	// Address/Size of stack
	void* pvStackAddress;
	QWORD qwStackSize;

	// Is FPU used
	BOOL bFPUUsed;

	// Padding
	char vcPadding[11];
} TCB;

// Task pool status manager structure
typedef struct TCBPoolManagerStruct{
	// Task pool info
	TCB* pstStartAddress;
	int iMaxCount;
	int iUseCount;

	// TCB allocated count
	int iAllocatedCount;
} TCBPOOLMANAGER;

// Scheduler status manager structure
typedef struct SchedulerStruct{
	// Running Task
	TCB* pstRunningTask;

	// Processor time task can use
	int iProcessorTime;

	// Task list ready to run
	LIST vstReadyList[TASK_MAXREADYLISTCOUNT];

	// Task to end
	LIST stWaitList;

	// Task execute count for each priority
	int viExecuteCount[TASK_MAXREADYLISTCOUNT];

	// Processor utilization rate
	QWORD qwProcessorLoad;

	// Processor time in Idle Task
	QWORD qwSpendProcessorTimeInIdleTask;

	// Last task ID use FPU
	QWORD qwLastFPUUsedTaskID;

} SCHEDULER;


#pragma pack(pop)




// Function
//=================================
//  Task pool / Task
//=================================
static void InitializeTCBPool(void);
static TCB* AllocateTCB(void);
static void FreeTCB(QWORD qwID);
TCB* CreateTask(QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress);
static void SetUpTask(TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize);

//=================================
// Scheduler
//=================================
void InitializeScheduler(void);
void SetRunningTask(TCB* pstTask);
TCB* GetRunningTask(void);
static TCB* GetNextTaskToRun(void);
static BOOL AddTaskToReadyList(TCB* pstTask);
void Schedule(void);
BOOL ScheduleInInterrupt(void);
void DecreaseProcessorTime(void);
BOOL IsProcessorTimeExpired(void);
static TCB* RemoveTaskFromReadyList(QWORD qwTaskID);
BOOL ChangePriority(QWORD qwID, BYTE bPeriority);
BOOL EndTask(QWORD qwTaskID);
void ExitTask(void);
int GetReadyTaskCount(void);
int GetTaskCount(void);
TCB* GetTCBInTCBPool(int iOffset);
BOOL IsTaskExist(QWORD qwID);
QWORD GetProcessorLoad(void);
static TCB* GetProcessByThread(TCB* pstThread);


//=================================
// Idle Task
//=================================
void IdleTask(void);
void HaltProcessorByLoad(void);

//=================================
// FPU
//=================================
QWORD GetLastFPUUsedTaskID(void);
void SetLastFPUUsedTaskID(QWORD qwTaskID);

#endif /*__TASK_H__*/


