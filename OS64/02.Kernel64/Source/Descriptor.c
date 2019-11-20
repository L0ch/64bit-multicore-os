#include "Descriptor.h"
#include "Utility.h"
#include "ISR.h"
  ////////////////////////////////////////
 //              GDT & TSS             //
////////////////////////////////////////

// Initialize GDT
void InitializeGDTTableAndTSS(void){
	GDTR* pstGDTR;
	GDTENTRY8* pstEntry;
	TSSSEGMENT* pstTSS;
	int i;

	// Set GDTR
	pstGDTR = (GDTR*) GDTR_STARTADDRESS;
	pstEntry = (GDTENTRY8*)(GDTR_STARTADDRESS + sizeof(GDTR));
	pstGDTR->wLimit = GDT_TABLESIZE - 1;
	pstGDTR->qwBaseAddress = (QWORD)pstEntry;
	// Set TSS area
	pstTSS = (TSSSEGMENT*) ( (QWORD)pstEntry + GDT_TABLESIZE );
	//Create 4 segment for NULL, 64bit Code/Data, TSS
	SetGDTEntry8( &(pstEntry[0]), 0, 0, 0, 0, 0 );
	SetGDTEntry8( &(pstEntry[1]), 0, 0xFFFFF, GDT_FLAGS_UPPER_CODE, GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE );
	SetGDTEntry8( &(pstEntry[2]), 0, 0xFFFFF, GDT_FLAGS_UPPER_DATA, GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA );
	SetGDTEntry16( (GDTENTRY16*)&(pstEntry[3]), (QWORD)pstTSS, sizeof(TSSSEGMENT)-1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS);

	// Initialize TSS
	InitializeTSSSegment(pstTSS);
}

// Set 8byte GDT Entry for CODE/SEGMENT descriptor
void SetGDTEntry8(GDTENTRY8* pstEntry, DWORD dwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType){
	pstEntry->wLowerLimit = dwLimit & 0xFFFF;
	pstEntry->wLowerBaseAddress = dwBaseAddress & 0xFFFF;
	pstEntry->bUpperBaseAddress1 = (dwBaseAddress >> 16) & 0xFF;
	pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
	pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0x0F) | bUpperFlags;
	pstEntry->bUpperBaseAddress2 = (dwBaseAddress >> 24) & 0xFF;
}

// Set 16byte GDT Entry for TSS descriptor
void SetGDTEntry16(GDTENTRY16* pstEntry, QWORD qwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType){
	pstEntry->wLowerLimit = dwLimit & 0xFFFF;
	pstEntry->wLowerBaseAddress = qwBaseAddress & 0xFFFF;
	pstEntry->bMiddleBaseAddress1 = (qwBaseAddress >> 16) & 0xFF;
	pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
	pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0x0F) | bUpperFlags;
	pstEntry->bMiddleBaseAddress2 = (qwBaseAddress >> 24) & 0xFF;
	pstEntry->dwUpperBaseAddress = qwBaseAddress >> 32;
	pstEntry->dwReserved = 0;
}

// Initialize TSS
void InitializeTSSSegment(TSSSEGMENT* pstTSS){
	MemSet(pstTSS, 0, sizeof(TSSSEGMENT));
	pstTSS->qwIST[0] = IST_STARTADDRESS + IST_SIZE;
	// IO Map: Not used
	pstTSS->wIOMapBaseAddress = 0xFFFF;
}

////////////////////////////////////////
//              IDT                   //
////////////////////////////////////////

//Initialize IDT
void InitializeIDTTables(void){
	IDTR* pstIDTR;
	IDTENTRY* pstEntry;
	int i;

	pstIDTR = (IDTR*) IDTR_STARTADDRESS;
	pstEntry = (IDTENTRY*) (IDTR_STARTADDRESS + sizeof(IDTR));
	pstIDTR->qwBaseAddress = (QWORD)pstEntry;
	pstIDTR->wLimit = IDT_TABLESIZE - 1;

	//================================================
	// Exception ISR
	//================================================
	SetIDTEntry( &(pstEntry[0]), ISRDivideError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[1]), ISRDebug, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[2]), ISRNMI, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[3]), ISRBreakPoint, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[4]), ISROverflow, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[5]), ISRBoundRangeExceeded, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[6]), ISRInvalidOpcode, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[7]), ISRDeviceNotAvailable, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[8]), ISRDoubleFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[9]), ISRCoprocessorSegmentOverrun, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[10]), ISRInvalidTSS, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[11]), ISRSegmentNotPresent, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[12]), ISRStackSegmentFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[13]), ISRGeneralProtection, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[14]), ISRPageFault, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[15]), ISR15, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[16]), ISRFPUError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[17]), ISRAlignmentCheck, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[18]), ISRMachineCheck, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[19]), ISRSIMDError, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
	SetIDTEntry( &(pstEntry[20]), ISRETCException, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );

	for(i=21; i<32; i++){
		SetIDTEntry( &(pstEntry[i]), ISRETCException, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	}


	/*//Connect 0~99 vector to DummyHandler
	for(i=0; i<IDT_MAXENTRYCOUNT; i++){
		SetIDTEntry(&(pstEntry[i]), DummyHandler, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	}*/

	//================================================
	// Interrupt ISR
	//================================================
	SetIDTEntry( &(pstEntry[32]), ISRTimer, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[33]), ISRKeyboard, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[34]), ISRSlavePIC, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[35]), ISRSerial2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[36]), ISRSerial1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[37]), ISRParallel2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[38]), ISRFloppy, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[39]), ISRParallel1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[40]), ISRRTC, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[41]), ISRReserved, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[42]), ISRNotUsed1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[43]), ISRNotUsed2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[44]), ISRMouse, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[45]), ISRCoprocessor, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[46]), ISRHDD1, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	SetIDTEntry( &(pstEntry[47]), ISRHDD2, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);

	for(i=48; i<IDT_MAXENTRYCOUNT; i++){
		SetIDTEntry( &(pstEntry[i]), ISRETCInterrupt, 0x08, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
	}


}

//Set IDT Gate descriptor
void SetIDTEntry(IDTENTRY* pstEntry, void* pvHandler, WORD wSelector, BYTE bIST, BYTE bFlags, BYTE bType){
	pstEntry->wLowerBaseAddress = (QWORD) pvHandler & 0xFFFF;
	pstEntry->wSegmentSelector = wSelector;
	pstEntry->bIST = bIST & 0x3;
	pstEntry->bTypeAndFlags = bType | bFlags;
	pstEntry->wMiddleBaseAddress = ((QWORD) pvHandler >> 16 ) & 0xFFFF;
	pstEntry->dwUpperBaseAddress = (QWORD) pvHandler >> 32;
	pstEntry->dwReserved = 0;
}

/*
//dummy
void DummyHandler(void){
	PrintString(0, 0, "=====================================================================");
	PrintString(0, 1, "                 Dummy Interrupt Handler Execute                     ");
	PrintString(0, 2, "                  Interrupt or Exception Occur                       ");
	PrintString(0, 3, "=====================================================================");

	while(1);
}*/


