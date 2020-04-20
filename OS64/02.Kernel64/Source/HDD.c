#include "HDD.h"
#include "Utility.h"
#include "AssemblyUtility.h"
#include "Console.h"
static HDDMANAGER gs_stHDDManager;

// Initialize HDD
BOOL InitializeHDD(void){

	// Initialize Mutex
	InitializeMutex(&(gs_stHDDManager.stMutex));

	// Initialize Interrupt flag
	gs_stHDDManager.bPrimaryInterruptOccur = FALSE;
	gs_stHDDManager.bSecondaryInterruptOccur = FALSE;

	// Activate HDD controller interrupt
	OutPortByte(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);
	OutPortByte(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);

	// Get HDD info
	if(ReadHDDInformation(TRUE, TRUE, &(gs_stHDDManager.stHDDInformation)) == FALSE){
		gs_stHDDManager.bHDDDetected = FALSE;
		gs_stHDDManager.bCanWrite = FALSE;
		return FALSE;
	}

	// Use only QEMU for test
	gs_stHDDManager.bHDDDetected = TRUE;

	if(MemCmp(gs_stHDDManager.stHDDInformation.vwModelNumber, "QEMU", 4) == 0){
		gs_stHDDManager.bCanWrite = TRUE;
	}
	else{
		gs_stHDDManager.bCanWrite = FALSE;
	}
	return TRUE;
}

// Return HDD status
static BYTE ReadHDDStatus(BOOL bPrimary){
	if(bPrimary == TRUE){
		// Status register of first PATA (0x1F7)
		return InPortByte(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS);
	}
	// Status register of second PATA (0x177)
	return InPortByte(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS);
}

// Wait until HDD is not busy
static BOOL WaitForHDDNoBusy(BOOL bPrimary){
	QWORD qwStartTickCount;
	BYTE bStatus;

	qwStartTickCount = GetTickCount();

	// Wait for wait time(500ms)
	while((GetTickCount() - qwStartTickCount) <= HDD_WAITTIME){
		//
		bStatus = ReadHDDStatus(bPrimary);

		// Check Busy bit (bit7), exit loop when HDD is not busy
		if((bStatus & HDD_STATUS_BUSY) != HDD_STATUS_BUSY){
			return TRUE;
		}
		Sleep(1);
	}

	return FALSE;
}

// Wait until HDD is ready
static BOOL WaitForHDDReady(BOOL bPrimary){
	QWORD qwStartTickCount;
	BYTE bStatus;


	// Wait for wait time(500ms)
	qwStartTickCount = GetTickCount();
	while((GetTickCount() - qwStartTickCount) <= HDD_WAITTIME){
		bStatus = ReadHDDStatus(bPrimary);

		// Check ready bit (bit6), exit loop when HDD is ready
		if((bStatus & HDD_STATUS_READY) == HDD_STATUS_READY){
			return TRUE;
		}
		Sleep(1);
	}
	return FALSE;
}


// Set interrupt flag
void SetHDDInterruptFlag(BOOL bPrimary, BOOL bFlag){
	if(bPrimary == TRUE){
		gs_stHDDManager.bPrimaryInterruptOccur = bFlag;
	}
	else{
		gs_stHDDManager.bSecondaryInterruptOccur = bFlag;
	}
}
// Wait until interrupt occurred
static BOOL WaitForHDDInterrupt(BOOL bPrimary){
	QWORD qwTickCount;
	qwTickCount = GetTickCount();

	while(GetTickCount() - qwTickCount <= HDD_WAITTIME){

		// Check interrupt occur flag
		if((bPrimary == TRUE) && (gs_stHDDManager.bPrimaryInterruptOccur == TRUE)){
			return TRUE;
		}
		else if((bPrimary == FALSE) && (gs_stHDDManager.bSecondaryInterruptOccur == TRUE)){
			return TRUE;
		}
	}
	return FALSE;
}

// Read HDD info
BOOL ReadHDDInformation(BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation){
	WORD wPortBase;
	QWORD qwLastTickCount;
	BYTE bStatus;
	BYTE bDriveFlag;
	int i;
	WORD wTemp;
	BOOL bWaitResult;

	// Set I/O port base address by PATA
	if(bPrimary == TRUE){
		// First PATA -> 0x1F0
		wPortBase = HDD_PORT_PRIMARYBASE;
	}
	else{
		// Second PATA -> 0x170
		wPortBase = HDD_PORT_SECONDARYBASE;
	}

	// Synchronization
	Lock(&(gs_stHDDManager.stMutex));

	if(WaitForHDDNoBusy(bPrimary) == FALSE){
		Unlock(&(gs_stHDDManager.stMutex));
		return FALSE;
	}

	// LBA address register, drive/head register
	if(bMaster == TRUE){
		bDriveFlag = HDD_DRIVEANDHEAD_LBA;
	}
	else{
		bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
	}

	OutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag);

	// Send command, wait for interrupt
	if(WaitForHDDReady(bPrimary) == FALSE){
		Unlock(&(gs_stHDDManager.stMutex));
		return FALSE;
	}

	SetHDDInterruptFlag(bPrimary, FALSE);

	// Send drive recognition command(0xEC) to command register(master:0x1F7, slave:0x177)
	OutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_IDENTIFY);

	bWaitResult = WaitForHDDInterrupt(bPrimary);
	bStatus = ReadHDDStatus(bPrimary);

	// Error OR interrupt not occurred
	if((bWaitResult == FALSE) || ((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR)){
		Unlock(&(gs_stHDDManager.stMutex));
		return FALSE;
	}

	// Receive data
	// Read 1 sector
	for(i=0; i<512/2; i++){
		((WORD*)pstHDDInformation)[i] = InPortWord(wPortBase + HDD_PORT_INDEX_DATA);
	}

	// Swap high/low byte
	SwapByteInWord(pstHDDInformation->vwModelNumber, sizeof(pstHDDInformation->vwModelNumber) / 2);
	SwapByteInWord(pstHDDInformation->vwSerialNumber, sizeof(pstHDDInformation->vwSerialNumber) / 2);

	Unlock(&(gs_stHDDManager.stMutex));
	return TRUE;
}

// Swap high/low order byte
static void SwapByteInWord(WORD* pwData, int iWordCount){
	int i;
	WORD wTemp;

	for(i=0; i<iWordCount; i++){
		wTemp = pwData[i];
		pwData[i] = (wTemp >> 8) | (wTemp << 8);
	}
}

// Read Sector(max=256)
// Return read sector count
int ReadHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer){
	WORD wPortBase;
	int i,j;
	BYTE bDriverFlag;
	BYTE bStatus;
	long lReadCount = 0;
	BOOL bWaitResult;

	// Check
	if((gs_stHDDManager.bHDDDetected == FALSE) || (iSectorCount <= 0) || (iSectorCount > 256) ||
			((dwLBA + iSectorCount) >=gs_stHDDManager.stHDDInformation.dwTotalSectors)){
		return 0;
	}

	// First PATA
	if(bPrimary == TRUE){
		wPortBase = HDD_PORT_PRIMARYBASE;
	}
	// Second PATA
	else{
		wPortBase = HDD_PORT_SECONDARYBASE;
	}


	// Wait for previous command
	if(WaitForHDDNoBusy(bPrimary) == FALSE){
		return FALSE;
	}

	// Synchronization
	Lock(&(gs_stHDDManager.stMutex));

	// ================ Set data register ==================
	// LBA address : sector num -> cylinder num -> head num
	// =====================================================

	// Sector count register(0x1F2 or 0x172)
	OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount);
	// Sector number register(0x1F3 or 0x173)
	OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA);
	// Cylinder LSB register(0x1F4 or 0x174) LBA 8~15 bit
	OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8);
	// Cylinder LSB register(0x1F5 or 0x175) LBA 16~23 bit
	OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 16);

	// Drive/Head data
	if(bMaster == TRUE){
		bDriverFlag = HDD_DRIVEANDHEAD_LBA;
	}
	else{
		bDriverFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
	}

	// Drive/Head register(0x1F6 or 0x176) LBA 24~27 bit and driver flag
	OutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriverFlag | ((dwLBA >> 24) & 0x0F));

	// Send command
	// Wait for ready to receive command
	if(WaitForHDDReady(bPrimary) == FALSE){
		Unlock(&(gs_stHDDManager.stMutex));
		return FALSE;
	}

	SetHDDInterruptFlag(bPrimary, FALSE);

	// Command register(0x1F7 or 0x177), send read command(0x20)
	OutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ);


	// Loop as sector count, receive data
	for(i=0; i<iSectorCount; i++){
		// Error
		bStatus = ReadHDDStatus(bPrimary);
		if((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR){
			Printf("Error Occur\n");
			Unlock(&(gs_stHDDManager.stMutex));
			return i;
		}

		if((bStatus & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST){
			bWaitResult = WaitForHDDInterrupt(bPrimary);
			SetHDDInterruptFlag(bPrimary, FALSE);
			if(bWaitResult == FALSE){
				Printf("Interrupt Not Occur\n");
				Unlock(&(gs_stHDDManager.stMutex));
				return FALSE;
			}
		}

		// Read Sector
		for(j=0; j<512/2; j++){
			((WORD*)pcBuffer)[lReadCount++] = InPortWord(wPortBase + HDD_PORT_INDEX_DATA);
		}
	}

	Unlock(&(gs_stHDDManager.stMutex));
	return i;
}



int WriteHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer){
	WORD wPortBase;
	WORD wTemp;
	int i,j;
	BYTE bDriverFlag;
	BYTE bStatus;
	long lReadCount = 0;
	BOOL bWaitResult;

	if((gs_stHDDManager.bCanWrite == FALSE) || (iSectorCount <= 0) || (256 < iSectorCount)
			|| ((dwLBA + iSectorCount) >= gs_stHDDManager.stHDDInformation.dwTotalSectors)){

		return 0;

	}

	// First PATA
	if(bPrimary == TRUE){
		wPortBase = HDD_PORT_PRIMARYBASE;
	}
	// Second PATA
	else{
		wPortBase = HDD_PORT_SECONDARYBASE;
	}

	// Wait for previous command
	if(WaitForHDDNoBusy(bPrimary) == FALSE){
		return FALSE;
	}

	// Synchronization
	Lock(&(gs_stHDDManager.stMutex));

	// ================ Set data register ==================
	// LBA address : sector num -> cylinder num -> head num
	// =====================================================

	// Sector count register(0x1F2 or 0x172)
	OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount);
	// Sector number register(0x1F3 or 0x173)
	OutPortByte(wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA);
	// Cylinder LSB register(0x1F4 or 0x174) LBA 8~15 bit
	OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8);
	// Cylinder LSB register(0x1F5 or 0x175) LBA 16~23 bit
	OutPortByte(wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 16);

	// Drive/Head data
	if(bMaster == TRUE){
		bDriverFlag = HDD_DRIVEANDHEAD_LBA;
	}
	else{
		bDriverFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
	}

	// Drive/Head register(0x1F6 or 0x176) LBA 24~27 bit and driver flag
	OutPortByte(wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriverFlag | ((dwLBA >> 24) & 0x0F));

	// Send command
	// Wait for ready to receive command
	if(WaitForHDDReady(bPrimary) == FALSE){
		Unlock(&(gs_stHDDManager.stMutex));
		return FALSE;
	}

	OutPortByte(wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE);

	while(1){
		bStatus = ReadHDDStatus(bPrimary);
		if((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR){
			Unlock(&(gs_stHDDManager.stMutex));
			return 0;
		}

		if((bStatus & HDD_STATUS_DATAREQUEST) == HDD_STATUS_DATAREQUEST){
			break;
		}
		Sleep(1);
	}


	// Send data
	// Wait for interrupt
	for(i=0; i<iSectorCount; i++){

		SetHDDInterruptFlag(bPrimary, FALSE);
		// Write sector
		for(j=0; j<512/2; j++){
			OutPortWord(wPortBase + HDD_PORT_INDEX_DATA, ((WORD*)pcBuffer)[lReadCount++]);
		}

		bStatus = ReadHDDStatus(bPrimary);
		if((bStatus & HDD_STATUS_ERROR) == HDD_STATUS_ERROR){
			Unlock(&(gs_stHDDManager.stMutex));
			return i;
		}
		// Wait for DATAREQUEST bit set
		if((bStatus & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST){
			bWaitResult = WaitForHDDInterrupt(bPrimary);
			SetHDDInterruptFlag(bPrimary, FALSE);

			if(bWaitResult == FALSE){
				Unlock(&(gs_stHDDManager.stMutex));
				return FALSE;
			}
		}
	}
	Unlock(&(gs_stHDDManager.stMutex));
	return i;

}




















