#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"


SHELLCOMMANDENTRY gs_vstCommandTable[] = {
		{"help", "Show Help", Help},
		{"clear", "Clear Screen", Clear},
		{"free", "Show Total Memory Size", ShowTotalMemorySize},
		{"strtod","String To Decimal/Hex Convert", StringToDecimalHex},
		{"shutdown", "Shutdown And Reboot OS", Shutdown},
};


void StartConsoleShell(void){
	char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	int CommandBufferIndex = 0;
	BYTE bKey;
	int CursorX, CursorY;
	Printf(CONSOLESHELL_PROMPTMESSAGE);

	while(1){
		bKey = GetCh();
		// Backspace
		if(bKey == KEY_BACKSPACE){
			if(CommandBufferIndex > 0){
				GetCursor(&CursorX, &CursorY);
				PrintStringXY(CursorX - 1, CursorY, " ");
				SetCursor(CursorX - 1, CursorY);
				CommandBufferIndex--;
			}
		}
		// Enter(execute command and next line
		else if(bKey == KEY_ENTER){
			Printf("\n");

			if(CommandBufferIndex > 0){
					vcCommandBuffer[CommandBufferIndex] = '\0';
					ExecuteCommand(vcCommandBuffer);
			}

			Printf("%s", CONSOLESHELL_PROMPTMESSAGE);
			MemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
			CommandBufferIndex = 0;
		}
		else if((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) || (bKey == KEY_CAPSLOCK) ||
				(bKey == KEY_NUMLOCK) || (bKey == KEY_SCROLLLOCK)){
			;
		}
		// normal character
		else{
			// TAB->space
			if(bKey == KEY_TAB){
				bKey = ' ';
			}

			if(CommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT){
				vcCommandBuffer[CommandBufferIndex] = bKey;
				Printf("%c", bKey);
				CommandBufferIndex++;
			}
		}

	}
}


void ExecuteCommand(const char* pcCommandBuffer){
	int i, SpaceIndex;
	int CommandBufferLength, CommandLength;
	int Count;

	// Filter command parameter
	CommandBufferLength = StrLen(pcCommandBuffer);
	for(SpaceIndex = 0; SpaceIndex < CommandBufferLength; SpaceIndex++){
		if(pcCommandBuffer[SpaceIndex] == ' '){
			break;
		}
	}
	//All command entry
	Count = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
	for(i = 0; i < Count; i++){
		CommandLength = StrLen(gs_vstCommandTable[i].pcCommand);
		//
		if((CommandLength == SpaceIndex) &&
		   (MemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer, SpaceIndex)==0)){
			// Execute command function
			gs_vstCommandTable[i].pfFunction(pcCommandBuffer + SpaceIndex +1);
			break;
		}
	}
	if(i >= Count){
		Printf("'%s': command not found.\n", pcCommandBuffer);
	}
}

// Initialize parameter structure
void InitializeParameter(PARAMETERLIST* pstList, const char* pcParameter){
	pstList->pcBuffer = pcParameter;
	pstList->Length = StrLen(pcParameter);
	pstList->CurrentPosition = 0;
}

// Return parameter, length separated by space
int GetNextParameter(PARAMETERLIST* pstList, char* pcParameter){
	int i;
	int Length;

	// Exit no parameter
	if(pstList->Length <= pstList->CurrentPosition){
		return 0;
	}

	// Search space
	for(i = pstList->CurrentPosition; i < pstList->Length; i++){
		if(pstList->pcBuffer[i] == ' '){
			break;
		}
	}

	// Copy parameter and return length
	MemCpy(pcParameter, pstList->pcBuffer + pstList->CurrentPosition, i);
	Length = i - pstList->CurrentPosition;
	pcParameter[Length] = '\0';

	pstList->CurrentPosition += Length + 1;
	return Length;
}




/////////////////////////////////////////////////////////
// Processing Command
////////////////////////////////////////////////////////

void Help(const char* pcCommandBuffer){
	int i;
	int Count;
	int CursorX, CursorY;
	int Length, MaxCommandLength = 0;
	Printf("=======================================\n");
	Printf("Console Shell by dw0rptr, version 0.1\n");
	Printf("=======================================\n");
	Count = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

	// calc longest command length for print sort
	for(i = 0; i < Count; i++){
		Length = StrLen(gs_vstCommandTable[i].pcCommand);
		if(Length > MaxCommandLength){
			MaxCommandLength = Length;
		}
	}

	// Print help
	for(i = 0; i < Count; i++){
		Printf("%s", gs_vstCommandTable[i].pcCommand);
		GetCursor(&CursorX, &CursorY);
		SetCursor(MaxCommandLength, CursorY);
		Printf("  - %s\n", gs_vstCommandTable[i].pcHelp);
	}
}

// Clear display
void Clear(const char* pcParameterBuffer){
	//Clear Screen and move cursor
	ClearScreen();
	SetCursor(0,1);
}

void ShowTotalMemorySize(const char* pcParameterBuffer){
	Printf("Total Memory Size : %d MB\n", GetTotalMemorySize());
}

void StringToDecimalHex(const char* pcParameterBuffer){
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	int iCount = 0;
	long lValue;

	InitializeParameter(&stList, pcParameterBuffer);

	while(1){
		iLength = GetNextParameter(&stList, vcParameter);

		// Parameter not exist
		if(iLength == 0){

			if(iCount == 1){
				Printf("There is No Parameter\n");
				Printf("Usage: strtod [DECIMAL/HEXADECIMAL]...\n");
			}
			break;
		}

		// Print parameter info
		Printf("Parameter %d = '%s', Length = %d, ", iCount +1, vcParameter, iLength);

		// If '0x' Hex
		if(MemCmp(vcParameter, "0x", 2) == 0){
			lValue = AToI(vcParameter + 2, 16);
			Printf("HEX Value = %q\n",lValue);
		}
		// Decimal
		else{
			lValue = AToI(vcParameter, 10);
			Printf("Decimal Value = %d\n", lValue);
		}
		iCount++;
	}
}

void Shutdown(const char* pcParameterBuffer){
	Printf("System Shutdown...\n");

	Printf("Press Any Key To Reboot...");
	GetCh();
	Reboot();
}















