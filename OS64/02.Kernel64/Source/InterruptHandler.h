#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"

//Function
void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode);
void kCommonInterruptHnadler(int iVectorNumber);
void kKeyboardHandler(int iVectorNumber);



#endif /*__INTERRUPTHANDLER_H__*/


