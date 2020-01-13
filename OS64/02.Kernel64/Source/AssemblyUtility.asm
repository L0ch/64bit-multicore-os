[BITS 64]

SECTION .text

global InPortByte, OutPortByte, LoadGDTR, LoadTR, LoadIDTR
global EnableInterrupt, DisableInterrupt, ReadRFLAGS
global ReadTSC
global SwitchContext, Hlt

; Read 1byte from port
InPortByte:
	push rdx

	mov rdx, rdi	; RDX = PARAM 1(Port Num)
	mov rax, 0		; init RAX
	in al, dx

	pop rdx
	ret				; return RAX

; Write 1byte to port
OutPortByte:
	push rdx
	push rax

	mov rdx, rdi	;RDX = PARAM 1(Port Num)
	mov rax, rsi	;RAX = PARAM 2(Data)
	out dx, al

	pop rax
	pop rdx
	ret
; Set GDTR register
; PARAM : GDT struct Address
LoadGDTR:
	lgdt[rdi]	; Load param1(GDTR address) into processor
	ret

; Set TR register
; PARAM : TSS Descriptor offset
LoadTR:
	ltr di		; Load param1(TSS Descriptor offset) into processor
	ret

; Set IDTR register
; PARAM : IDT struct address
LoadIDTR:
	lidt[rdi]	;Load param1(IDTR address) into processor
	ret

; Enable Interrupt
EnableInterrupt:
	sti
	ret

; Disable Interrupt
DisableInterrupt:
	cli
	ret

; Return RFLAGS register
ReadRFLAGS:
	pushfq		; push RFLAGS
	pop rax

	ret			; return RFLAGS


; Return time stamp counter
ReadTSC:
	push rdx

	rdtsc		; read time stamp counter and store to RDX:RAX

	shl rdx, 32	; Upper rdx 32bit | Lower rax 32bit
	or rax, rdx

	pop rdx
	ret



;;;;;;;;;;;;;;;;;;;;;;;
;        TASK
;;;;;;;;;;;;;;;;;;;;;;;

; Save Context / Selector
%macro SAVECONTEXT 0
	; push RBP ~ GS segment selector in stack
	push rbp
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

	mov ax, ds		; DS
	push rax
	mov ax, es		; ES
	push rax
	push fs
	push gs
%endmacro


%macro LOADCONTEXT 0
	; Load GS segment selector ~ RBP register
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


; Save current context and Restore Next Context(Task)
; PARAM : CONTEXT* CurrentContext, CONTEXT* NextContext
SwitchContext:
	push rbp
	mov rbp, rsp

	; Current Context == NULL -> need not save context
	pushfq				; Before cmp, save RFLAGS register in stack
	cmp rdi, 0
	je .LoadContext
	popfq				; Restore RFLAGS

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Save context of current task ;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push rax	; Offset of Context

	; Push SS, RSP, RFLAGS, CS, RIP
	mov ax, ss						; SS
	mov qword[rdi + (23*8)], rax

	mov rax, rbp					; RSP
	add rax, 16						; +16(push rbp, ret)
	mov qword[rdi + (22*8)], rax

	pushfq							; RFLAGS
	pop rax
	mov qword[rdi + (21*8)], rax

	mov ax, cs						; CS
	mov qword[rdi + (20*8)], rax

	mov rax, qword[rbp+8]			; RIP(ret of SwitchContext)
	mov qword[rdi + (19*8)], rax


	pop rax
	pop rbp

	; change rsp to save context using push
	add rdi, (19*8)
	mov rsp, rdi
	sub rdi, (19*8)

	; Save RBP ~ GS
	SAVECONTEXT

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Load context of next task  ;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.LoadContext:
	mov rsp, rsi

	; Load register
	LOADCONTEXT
	iretq

; Make process wait state
Hlt:
	hlt
	hlt
	ret















