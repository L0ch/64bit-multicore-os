[BITS 64]

SECTION .text

; Import
extern CommonExceptionHandler, CommonInterruptHandler, KeyboardHandler
extern TimerHandler, DeviceNotAvailableHandler, HDDHandler

; Export
; ISR for Processing Exception
global ISRDivideError, ISRDebug, ISRNMI, ISRBreakPoint, ISROverflow
global ISRBoundRangeExceeded, ISRInvalidOpcode, ISRDeviceNotAvailable, ISRDoubleFault
global ISRCoprocessorSegmentOverrun, ISRInvalidTSS, ISRSegmentNotPresent
global ISRStackSegmentFault, ISRGeneralProtection, ISRPageFault, ISR15
global ISRFPUError, ISRAlignmentCheck, ISRMachineCheck, ISRSIMDError, ISRETCException

; ISR for Processing Interrupt
global ISRTimer, ISRKeyboard, ISRSlavePIC, ISRSerial2, ISRSerial1, ISRParallel2
global ISRFloppy, ISRParallel1, ISRRTC, ISRReserved, ISRNotUsed1, ISRNotUsed2
global ISRMouse, ISRCoprocessor, ISRHDD1, ISRHDD2, ISRETCInterrupt

; Save Context / Replace Selector
%macro SAVECONTEXT 0
	; push RBP ~ GS segment selector in stack
	push rbp
	mov rbp, rsp
	push rax
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov ax, ds
	push rax
	mov ax, es
	push rax
	push fs
	push gs

	; Replace segment selector
	mov ax, 0x10	; Save kernel data segment descriptor to AX register
	mov ds, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
%endmacro

; Restore Context
%macro LOADCONTEXT 0
	; Load GS segment selector ~ RBP register

	; Load Kernel data segment descriptor
	pop gs
	pop fs
	pop rax
	mov es, ax
	pop rax
	mov ds, ax

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	pop rbp

%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Exception Handler
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #0, Divide Error ISR
ISRDivideError:
	SAVECONTEXT

	mov rdi, 0
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #1, Debug ISR
ISRDebug:
	SAVECONTEXT

	mov rdi, 1
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #2, NMI ISR
ISRNMI:
	SAVECONTEXT

	mov rdi, 2
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #3, BreakPoint ISR
ISRBreakPoint:
	SAVECONTEXT

	mov rdi, 3
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #4, Overflow ISR
ISROverflow:
	SAVECONTEXT

	mov rdi, 4
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #5, Bound Range Exceeded ISR
ISRBoundRangeExceeded:
	SAVECONTEXT

	mov rdi, 5
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #6, Invalid Opcode ISR
ISRInvalidOpcode:
	SAVECONTEXT

	mov rdi, 6
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #7, Device Not Available ISR
ISRDeviceNotAvailable:
	SAVECONTEXT

	mov rdi, 7
	call DeviceNotAvailableHandler

	LOADCONTEXT
	iretq

; #8, Double Fault ISR
ISRDoubleFault:
	SAVECONTEXT

	mov rdi, 8
	mov rsi, qword [ rbp + 8 ]
	call CommonExceptionHandler

	LOADCONTEXT
	add rsp, 8
	iretq

; #9, Coprocessor Segment Overrun ISR
ISRCoprocessorSegmentOverrun:
	SAVECONTEXT

	mov rdi, 9
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #10, Invalid TSS ISR
ISRInvalidTSS:
	SAVECONTEXT

	mov rdi, 10
	mov rsi, qword [ rbp + 8 ]
	call CommonExceptionHandler

	LOADCONTEXT
	add rsp, 8
	iretq

; #11, Segment Not Present ISR
ISRSegmentNotPresent:
	SAVECONTEXT

	mov rdi, 11
	mov rsi, qword [ rbp + 8 ]
	call CommonExceptionHandler

	LOADCONTEXT
	add rsp, 8
	iretq

; #12, Stack Segment Fault ISR
ISRStackSegmentFault:
	SAVECONTEXT

	mov rdi, 12
	mov rsi, qword [ rbp + 8 ]
	call CommonExceptionHandler

	LOADCONTEXT
	add rsp, 8
	iretq

; #13, General Protection ISR
ISRGeneralProtection:
	SAVECONTEXT

	mov rdi, 13
	mov rsi, qword [ rbp + 8 ]
	call CommonExceptionHandler

	LOADCONTEXT
	add rsp, 8
	iretq

; #14, Page Fault ISR
ISRPageFault:
	SAVECONTEXT

	mov rdi, 14
	mov rsi, qword [ rbp + 8 ]
	call CommonExceptionHandler

	LOADCONTEXT
	add rsp, 8
	iretq

; #15, Reserved ISR
ISR15:
	SAVECONTEXT

	mov rdi, 15
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #16, FPU Error ISR
ISRFPUError:
	SAVECONTEXT

	mov rdi, 16
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #17, Alignment Check ISR
ISRAlignmentCheck:
	SAVECONTEXT

	mov rdi, 17
	mov rsi, qword [rbp+8]
	call CommonExceptionHandler

	LOADCONTEXT
	add rsp, 8
	iretq

; #18, Machine Check ISR
ISRMachineCheck:
	SAVECONTEXT

	mov rdi, 18
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #19, SIMD Floating Point Exception ISR
ISRSIMDError:
	SAVECONTEXT

	mov rdi, 19
	call CommonExceptionHandler

	LOADCONTEXT
	iretq

; #20~#31, Reserved ISR
ISRETCException:
	SAVECONTEXT

	mov rdi, 20
	call CommonExceptionHandler

	LOADCONTEXT
	iretq


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   Interrupt Handler
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; #32, Timer ISR
ISRTimer:
	SAVECONTEXT

	mov rdi, 32
	call TimerHandler

	LOADCONTEXT
	iretq

; #33, Keyboard ISR
ISRKeyboard:
	SAVECONTEXT

	mov rdi, 33
	call KeyboardHandler

	LOADCONTEXT
	iretq

; #34, Slave PIC ISR
ISRSlavePIC:
	SAVECONTEXT

	mov rdi, 34
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #35, Serial Port2 ISR
ISRSerial2:
	SAVECONTEXT

	mov rdi, 35
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #36, Serial Port1 ISR
ISRSerial1:
	SAVECONTEXT

	mov rdi, 36
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #37, Parallel Port2 ISR
ISRParallel2:
	SAVECONTEXT

	mov rdi, 37
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #38, Floppy Disk Controller ISR
ISRFloppy:
	SAVECONTEXT

	mov rdi, 38
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #39, Parallel Port1
ISRParallel1:
	SAVECONTEXT

	mov rdi, 39
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #40, RTC ISR
ISRRTC:
	SAVECONTEXT

	mov rdi, 40
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #41, Reserved Interrupt ISR
ISRReserved:
	SAVECONTEXT

	mov rdi, 41
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #42, NOT Used
ISRNotUsed1:
	SAVECONTEXT

	mov rdi, 42
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #43, NOT Used
ISRNotUsed2:
	SAVECONTEXT

	mov rdi, 43
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #44, Mouse ISR
ISRMouse:
	SAVECONTEXT

	mov rdi, 44
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #45, Coprocessor ISR
ISRCoprocessor:
	SAVECONTEXT

	mov rdi, 45
	call CommonInterruptHandler

	LOADCONTEXT
	iretq

; #46, HDD1 ISR
ISRHDD1:
	SAVECONTEXT

	mov rdi, 46
	call HDDHandler

	LOADCONTEXT
	iretq

; #47, HDD2 ISR
ISRHDD2:
	SAVECONTEXT

	mov rdi, 47
	call HDDHandler

	LOADCONTEXT
	iretq

; #48 ETC
ISRETCInterrupt:
	SAVECONTEXT

	mov rdi, 48
	call CommonInterruptHandler

	LOADCONTEXT
	iretq






















