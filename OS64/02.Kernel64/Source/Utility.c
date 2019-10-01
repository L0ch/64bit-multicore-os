#include "Utility.h"

// Fill Memory with specific value
void kMemSet( void* pvDestination, BYTE bData, int iSize ){
	int i;

	for(i=0; i<iSize; i++){
		((char*)pvDestination)[i] = bData;
	}
}

// Copy
int kMemCpy( void* pvDestination, cont void* pvSource, int iSize ){
	int i;

	for(i=0; i<iSize; i++){
		((char*)pvDestination)[i] = ((char*)pvSource)[i];
	}
	return iSize;
}

// Compare
int kMemCmp( const void* pvDestination, const void* pvSource, int iSize ){
	int i;
	char cTemp;

	for(i=0; i<iSize; i++){
		cTemp = ((char*)pvDestination)[i] - ((char*)pvSource)[i];
		if(cTemp != 0){
			return (int)cTemp;
		}
	}
	return 0;
}
