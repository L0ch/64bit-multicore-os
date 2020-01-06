#include "List.h"


// Initailize list
void InitializeList(LIST* pstList){
	pstList->iNodeCount = 0;
	pstList->pvHead = NULL;
	pstList->pvTail = NULL;
}

// Return list node count
int GetListCount(const LIST* pstList){
	return pstList->iNodeCount;
}

// Add node to tail of list
void AddListToTail(LIST* pstList, void* pvNode){
	LINKEDLIST* pstLink;

	// Next node address : NULL
	pstLink = (LINKEDLIST*) pvNode;
	pstLink->pvNext = NULL;

	// If list is empty
	// Set Head/Tail
	if(pstList->pvHead == NULL){
		pstList->pvHead = pvNode;
		pstList->pvTail = pvNode;
		pstList->iNodeCount = 1;
		return ;
	}

	// List is not empty
	// Find tail node
	pstLink = (LINKEDLIST*) pstList->pvTail;
	// Set next
	pstLink->pvNext = pvNode;

	// Set Tail to added node
	pstList->pvTail = pvNode;
	pstList->iNodeCount++;


}


// Add node to Head of list
void AddListToHead(LIST* pstList, void* pvNode){
	LINKEDLIST* pstLink;

	// Set Next to Head
	pstLink = (LINKEDLIST*) pvNode;
	pstLink->pvNext = pstList->pvHead;

	// If list is empty
	// Set Head/Tail
	if(pstList->pvHead == NULL){
		pstList->pvHead = pvNode;
		pstList->pvTail = pvNode;
		pstList->iNodeCount = 1;
		return ;
	}

	// Set tail to added node
	pstList->pvHead = pvNode;
	pstList->iNodeCount++;
}


// Remove node in list, return removed node pointer
void* RemoveList(LIST* pstList, QWORD qwID){
	LINKEDLIST* pstLink;
	LINKEDLIST* pstPreviousLink;

	pstPreviousLink = (LINKEDLIST*) pstList->pvHead;
	for(pstLink = pstPreviousLink; pstLink != NULL; pstLink = pstLink->pvNext){
		// Remove If same ID exist
		if(pstLink->qwID == qwID){
			// If there is only one node, initialize list
			if((pstLink==pstList->pvHead) && (pstLink == pstList->pvTail)){
				pstList->pvHead = NULL;
				pstList->pvTail = NULL;
			}
			// If first node, set Head to next node
			else if(pstLink == pstList->pvHead){
				pstList->pvHead == pstLink->pvNext;
			}
			// If last node, set tail to previous node
			else if(pstLink == pstList->pvTail){
				pstList->pvTail == pstPreviousLink;
			}
			else{
				pstPreviousLink->pvNext = pstLink->pvNext;
			}
			pstList->iNodeCount--;
			return pstLink;
		}
		// Save previous node
		pstPreviousLink = pstLink;
	}
	// ID does not exist
	return NULL;
}

// Remove Head node
void* RemoveListFromHead(LIST* pstList){
	LINKEDLIST* pstLink;

	if(pstList->iNodeCount == 0){
		return NULL;
	}

	pstLink = (LINKEDLIST*) pstList->pvHead;
	return RemoveList(pstList, pstLink->qwID);
}

// Remove tail node
// It can be improve performance by not to call RemoveList function
void* RemoveListFromTail(LIST* pstList){
	LINKEDLIST* pstLink;

	if(pstList->iNodeCount == 0){
		return NULL;
	}

	pstLink = (LINKEDLIST *) pstList->pvTail;
	return RemoveList(pstList, pstLink->qwID);
}

// Find node by ID
void* FindList(const LIST* pstList, QWORD qwID){
	LINKEDLIST* pstLink;

	for(pstLink = (LINKEDLIST*)pstList->pvHead; pstLink != NULL; pstLink = pstLink->pvNext){
		if(pstLink->qwID == qwID){
			return pstLink;
		}
	}
	return pstLink;
}

// Return head node
void* GetHeadFromList(const LIST* pstList){
	return pstList->pvHead;
}

// Return tail node
void* GetTailFromList(const LIST* pstList){
	return pstList->pvTail;
}

// Return next node of current node
void* GetNextFromList(const LIST* pstList, void* pstCurrent){
	LINKEDLIST* pstLink;
	pstLink = (LINKEDLIST*) pstCurrent;
	return pstLink->pvNext;
}
