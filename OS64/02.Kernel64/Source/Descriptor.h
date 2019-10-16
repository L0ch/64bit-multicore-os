#ifndef __DESCRIPTOR_H__
#define __DESCRIPTOR_H__

#include "Types.h"
// Macro

/////////////////////////////////////
//       		GDT		           //
////////////////////////////////////

#define GDT_TYPE_CODE			0x0A
#define GDT_TYPE_DATA			0x02
#define GDT_TYPE_TSS			0x09
#define GDT_FLAGS_LOWER_S		0x10
#define GDT_FLAGS_LOWER_DPL0	0x00
#define GDT_FLAGS_LOWER_DPL1	0x20
#define GDT_FLAGS_LOWER_DPL2	0x40
#define GDT_FLAGS_LOWER_DPL3	0x60
#define GDT_FLAGS_LOWER_P		0x80
#define GDT_FLAGS_UPPER_L		0x20
#define GDT_FLAGS_UPPER_DB		0x40
#define GDT_FLAGS_UPPER_G		0x80


//Lower Flags -> Set Code/Data/TSS, DPL0, Present
#define GDT_FLAGS_LOWER_KERNELCODE	( GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_KERNELDATA	( GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_TSS			( GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_USERCODE	( GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P )
#define GDT_FLAGS_LOWER_USERDATA	( GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P )

#define GDT_FLAGS_UPPER_CODE	( GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L )
#define GDT_FLAGS_UPPER_DATA	( GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L )
#define GDT_FLAGS_UPPER_TSS		( GDT_FLAGS_UPPER_G )

//Segment Descriptor offset
#define GDT_KERNELCODESEGMENT	0x08
#define GDT_KERNELDATASEGMENT	0X10
#define GDT_TSSSEGMENT			0X18

//GDTR start address, 264KB from 1MB is Page table
#define GDTR_STARTADDRESS		0X142000	//0x100000(1MB) + 264KB
//8byte entry count
#define GDT_MAXENTRY8COUNT		3
//16byte entry count
#define GDT_MAXENTRY16COUNT		1

//GDT table size
#define GDT_TABLESIZE		( (sizeof(GDTENTRY8) * GDT_MAXENTRY8COUNT) + (sizeof(GDTENTRY16) * GDT_MAXENTRY16COUNT) )
#define TSS_SEGMENTSIZE		( sizeof(TSSSEGMENT) )

/////////////////////////////////////
//       		IDT		           //
////////////////////////////////////

#define IDT_TYPE_INTERRUPT		0x0E
#define IDT_TYPE_TRAP			0x0F
#define IDT_FLAGS_DPL0			0x00
#define IDT_FLAGS_DPL1			0x20
#define IDT_FLAGS_DPL2			0x40
#define IDT_FLAGS_DPL3			0x60
#define IDT_FLAGS_P				0x80
#define IDT_FLAGS_IST0			0
#define IDT_FLAGS_IST1			1

#define IDT_FLAGS_KERNEL		( IDT_FLAGS_DPL0 | IDT_FLAGS_P )
#define IDT_FLAGS_USER			( IDT_FLAGS_DPL3 | IDT_FLAGS_P )

//IDT Entry count
#define IDT_MAXENTRYCOUNT		100
//IDTR start address, behind TSS segment
#define IDTR_STARTADDRESS		( GDTR_STARTADDRESS + sizeof(GDTR) + GDT_TABLESIZE + TSS_SEGMENTSIZE )
//IDT table start address
#define IDT_STARTADDRESS		( IDTR_STARTADDRESS + sizeof(IDTR) )
//Total size of IDT table
#define IDT_TABLESIZE			( IDT_MAXENTRYCOUNT * sizeof(IDTENTRY) )

//IST start address
#define IST_STARTADDRESS		0x700000
//IST size
#define IST_SIZE				0x100000

//structure
//Sort by 1byte
#pragma pack(push, 1)

//GDT, IDTR
typedef struct kGDTRStruct{
	WORD wLimit;
	QWORD qwBaseAddress;
	WORD wPading;
	DWORD dwPading;
}GDTR, IDTR;

//structure of 8byte size GDT Entry
typedef struct kGDTEntry8Struct{
	WORD wLowerLimit;
	WORD wLowerBaseAddress;
	BYTE bUpperBaseAddress1;
	BYTE bTypeAndLowerFlag;	// 4bit Type, 1bit S, 2bit DPL, 1bit P
	BYTE bUpperLimitAndUpperFlag;	//4bit Segment Limit, 1bit AVL, L, D/B, G
	BYTE bUpperBaseAddress2;
}GDTENTRY8;

//structure of 16byte size GDT Entry
typedef struct kGDTEntry16Struct{
	WORD wLowerLimit;
	WORD wLowerBaseAddress;
	BYTE bMiddleBaseAddress1;
	BYTE bTypeAndLowerFlag;
	BYTE bUpperLimitAndUpperFlag;
	BYTE bMiddleBaseAddress2;
	DWORD dwUpperBaseAddress;
	DWORD dwReserved;
}GDTENTRY16;

// TSS Data
typedef struct kTSSDataStruct{
	DWORD dwReserved1;
	QWORD qwRsp[3];
	QWORD qwReserved2;
	QWORD qwIST[7];
	QWORD qwReserved3;
	WORD wReserved;
	WORD wIOMapBaseAddress;
}TSSSEGMENT;

typedef struct kIDTEntryStruct{
	WORD wLowerBaseAddress;
	WORD wSegmentSelector;
	BYTE bIST;
	BYTE bTypeAndFlags;
	WORD wMiddleBaseAddress;
	DWORD dwUpperBaseAddress;
	DWORD dwReserved;
}IDTENTRY;

#pragma pack(pop)

//Function
void kInitializeGDTTableAndTss(void);
void kSetGDTEntry8( GDTENTRY8* pstEntry, DWORD dwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType );
void kSetGDTEntry16( GDTENTRY16* pstEntry, QWORD qwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType );
void kInitializeTSSSegment( TSSSEGMENT* pstTSS );

void kInitializeIDTTables(void);
void kSetIDTEntry( IDTENTRY* pstEntry, void* pvHandler, WORD wSelector, BYTE bIST, BYTE bFlags, BYTE bType );
void kDummyHandler(void);

#endif /*__DESCRIPTOR_H__*/










