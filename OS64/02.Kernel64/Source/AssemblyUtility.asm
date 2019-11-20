[BITS 64]

SECTION .text

global InPortByte, OutPortByte, LoadGDTR, LoadTR, LoadIDTR
global EnableInterrupt, DisableInterrupt, ReadRFLAGS

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







