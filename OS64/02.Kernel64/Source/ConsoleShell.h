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

void Help(const char* pcParameterBuffer);
void Clear(const char* pcParameterBuffer);
void ShowTotalMemorySize(const char* pcParameterBuffer);
void StringToDecimalHex(const char* pcParameterBuffer);
void Shutdown(const char* pcParameterBuffer);

#endif /*__CONSOLESHELL_H__*/
