[BITS 64]

SECTION .text

global kInPortByte, kOutPortByte


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




