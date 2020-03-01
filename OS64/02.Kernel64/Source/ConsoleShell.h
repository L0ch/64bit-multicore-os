#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT	300
#define CONSOLESHELL_PROMPTMESSAGE			"OSLAKE$ "

// Define Function pointer type
typedef void (* CommandFunction)(const char* pcParameter);

#pragma pack(push,1)

typedef struct ShellCommandEntryStruct{
	char* pcCommand;
	char* pcHelp;
	CommandFunction pfFunction;
}SHELLCOMMANDENTRY;

typedef struct ParameterListStruct{
	const char* pcBuffer;
	int Length;
	int CurrentPosition;
}PARAMETERLIST;

#pragma pack(pop)

void StartConsoleShell(void);
void ExecuteCommand(const char* pcCommandBuffer);
void InitializeParameter(PARAMETERLIST* pstList, const char* pcParameter);
int GetNextParameter(PARAMETERLIST* pstList, char* pcParameter);


// Shell command function
static void Help(const char* pcParameterBuffer);
static void Clear(const char* pcParameterBuffer);
static void ShowTotalMemorySize(const char* pcParameterBuffer);
static void StringToDecimalHex(const char* pcParameterBuffer);
static void Shutdown(const char* pcParameterBuffer);
static void SetTimer(const char* pcParameterBuffer);
static void WaitUsingPIT(const char* pcParameterBuffer);
static void ReadTimeStampCounter(const char* pcParameterBuffer);
static void MeasureProcessorSpeed(const char* pcParameterBuffer);
static void ShowDateAndTime(const char* pcParameterBuffer);
static void TestTask1(void);
static void TestTask2(void);
static void CreateTestTask(const char* pcParameterBuffer);
static void Echo(const char* pcParameterBuffer);
static void ChangeTaskPriority(const char* pcParameterBuffer);
static void ShowTaskList(const char* pcParameterBuffer);
static void KillTask(const char* pcParameterBuffer);
static void CPULoad(const char* pcParameterBuffer);
static void TestMutex(const char* pcParameterBuffer);
static void CreateThreadTask(void);
static void TestThread(const char* pcParameterBuffer);
static void TestPIE(const char* pcParameterBuffer);

#endif /*__CONSOLESHELL_H__*/
