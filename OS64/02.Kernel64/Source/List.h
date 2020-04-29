#ifndef __LIST_H__
#define __LIST_H__

#include "Types.h"

// structure
#pragma pack(push,1)

// Linked List structure
typedef struct LinkedListStruct{
	// Next address
	void * pvNext;
	void * pvPrev;
	// Identification
	QWORD qwID;

} LINKEDLIST;

// List Management structure
typedef struct ListManagerStruct{
	int iNodeCount;
	// Head node address
	void* pvHead;
	// Tail node address
	void* pvTail;
} LIST;

#pragma pack(pop)

//Function
void InitializeList(LIST* pstList);
int GetListCount(const LIST* pstList);
void AddListToTail(LIST* pstList, void* pvNode);
void AddListToHead(LIST* pstList, void* pvNode);
void* RemoveList(LIST* pstList, QWORD qwID);
void* RemoveListFromHead(LIST* pstList);
void* RemoveListFromTail(LIST* pstList);
void* FindList(const LIST* pstList, QWORD qwID);
void* GetHeadFromList(const LIST* pstList);
void* GetTailFromList(const LIST* pstList);
void* GetNextFromList(const LIST* pstList, void* pstCurrent);
void* GetPreviousFromList(const LIST* pstList, void* pstCurrent);

#endif /*__LIST_H__*/

