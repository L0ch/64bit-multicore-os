[BITS 64]

SECTION .text

global kInPortByte, kOutPortByte, kLoadGDTR, kLoadTR, kLoadIDTR


; Read 1byte from port
kInPortByte:
	push rdx

	mov rdx, rdi	; RDX = PARAM 1(Port Num)
	mov rax, 0		; init RAX
	in al, dx

	pop rdx
	ret				; return RAX

; Write 1byte to port
kOutPortByte:
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
kLoadGDTR:
	lgdt[rdi]	; Load param1(GDTR address) into processor
	ret

; Set TR register
; PARAM : TSS Descriptor offset
kLoadTR:
	ltr di		; Load param1(TSS Descriptor offset) into processor
	ret

; Set IDTR register
; PARAM : IDT struct address
kLoadIDTR:
	lidt[rdi]	;Load param1(IDTR address) into processor
	ret








