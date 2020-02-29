#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"

//Function
void CommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode);
void CommonInterruptHnadler(int iVectorNumber);
void KeyboardHandler(int iVectorNumber);
void TimerHandler(int iVectorNumber);
void DeviceNotAvailableHandler(int iVectorNumber);


#endif /*__INTERRUPTHANDLER_H__*/


