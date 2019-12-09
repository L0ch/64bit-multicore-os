#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"

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

// Structure
#pragma pack(push, 1)


typedef struct ContextStruct{
	QWORD vqRegister[TASK_REGISTERCOUNT];
} CONTEXT;


// Control task status
typedef struct TaskControlBlockStruct{
	// Context
	CONTEXT stContext;

	// ID/Flags
	QWORD qwID;
	QWORD qwFlags;

	// Address/Size of stack
	void* pvStackAddress;
	QWORD qwStackSize;
} TCB;

#pragma pack(pop)



// Function
void SetUpTask(TCB* pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize);

#endif /*__TASK_H__*/

