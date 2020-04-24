#ifndef __HDD_H__
#define __HDD_H__

#include "Types.h"
#include "Synchronization.h"

// Macro
// PATA port info

#define HDD_PORT_PRIMARYBASE		0x1F0
#define HDD_PORT_SECONDARYBASE		0x170

// Port index macro
#define HDD_PORT_INDEX_DATA				0x00
#define HDD_PORT_INDEX_SECTORCOUNT		0x02
#define HDD_PORT_INDEX_SECTORNUMBER		0x03
#define HDD_PORT_INDEX_CYLINDERLSB		0x04
#define HDD_PORT_INDEX_CYLINDERMSB		0x05
#define HDD_PORT_INDEX_DRIVEANDHEAD		0x06
#define HDD_PORT_INDEX_STATUS			0x07
#define HDD_PORT_INDEX_COMMAND			0x07
#define HDD_PORT_INDEX_DIGITALOUTPUT	0x206

// Command register
#define HDD_COMMAND_READ				0x20
#define HDD_COMMAND_WRITE				0x30
#define HDD_COMMAND_IDENTIFY			0xEC

// Status register
#define HDD_STATUS_ERROR				0x01
#define HDD_STATUS_INDEX				0x02
#define HDD_STATUS_CORRECTEDDATA		0x04
#define HDD_STATUS_DATAREQUEST			0x08
#define HDD_STATUS_SEEKCOMPLETE			0x10
#define HDD_STATUS_WRITEFAULT			0x20
#define HDD_STATUS_READY				0x40
#define HDD_STATUS_BUSY					0x80

// Drive/Head register
#define HDD_DRIVEANDHEAD_LBA			0xE0
#define HDD_DRIVEANDHEAD_SLAVE			0x10

// Digital output register
#define HDD_DIGITALOUTPUT_RESET				0x04
#define HDD_DIGITALOUTPUT_DISABLEINTERRUPT	0x01

// Waiting time for Response
#define HDD_WAITTIME					500
// Number of sectors can be read/write at one time
#define HDD_MAXBULKSECTORCOUNT			256


// Structure
#pragma pack(push, 1)

typedef struct HDDInformationStruct{
	WORD wConfiguration;

	// Cylinder
	WORD wNumberOfCylinder;
	WORD wReserved1;

	// Head
	WORD wNumberOfHead;
	WORD wUnformattedBytesPerTrack;
	WORD wUnformattedBytesPerSector;

	// Sectors Per Cylinder
	WORD wNumberOfSectorPerCylinder;
	WORD wInterSectorGap;
	WORD wBytesInPhaseLock;
	WORD wNumberOfVendorUniqueStatusWord;

	// Serial number
	WORD vwSerialNumber[10];
	WORD wControllerType;
	WORD wBufferSize;
	WORD wNumberOfECCBytes;
	WORD vwFiremwareRevision[4];

	// Model number
	WORD vwModelNumber[20];
	WORD vwReserved2[13];

	// Total number of sectors
	DWORD dwTotalSectors;
	WORD vwReserved3[196];

} HDDINFORMATION;

#pragma pack(pop)

typedef struct HDDManagerStruct{

	BOOL bHDDDetected;
	BOOL bCanWrite;

	// Interrupt, Mutex
	volatile BOOL bPrimaryInterruptOccur;
	volatile BOOL bSecondaryInterruptOccur;
	MUTEX stMutex;

	// HDD info
	HDDINFORMATION stHDDInformation;
} HDDMANAGER;


// Function
BOOL InitializeHDD(void);
BOOL ReadHDDInformation(BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation);
int ReadHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer);
int WriteHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer);
void SetHDDInterruptFlag(BOOL bPrimary, BOOL bFlag);

static void SwapByteInWord(WORD* pwData, int iWordCount);
static BYTE ReadHDDStatus(BOOL bPrimary);
static BOOL IsHDDBusy(BOOL bPrimary);
static BOOL IsHDDReady(BOOL bPrimary);
static BOOL WaitForHDDNoBusy(BOOL bPrimary);
static BOOL WaitForHDDReady(BOOL bPrimary);
static BOOL WiatForHDDInterrupt(BOOL bPrimary);

#endif /*__HDD_H__*/







