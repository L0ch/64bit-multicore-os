[ORG 0x00]
[BITS 16]

SECTION .text

;====================================================
;                    CODE SECTION
;====================================================
START:
	mov ax, 0x1000	;convert PROTECTEDMODE entry point start address(0x10000) to segment register
	mov ds, ax
	mov es, ax

	cli 			; NO interrupt
	lgdt [ GDTR ] 	; set processor to GDTR data structure, load GDT table

	;====================================================
	;                 ENTER PROTECED MODE
	; 	Disable Paging, Disable Cache, Internal FPU,
	; 	Disable Align Check, Enable ProtectedMode
	;====================================================
	mov eax, 0x4000003B	; PG=0, CD=1, NW=0, AM=0, WP=0, NE=1, TS=1, EM=0, MP=1, PE=1
	mov cr0, eax		; set CR0 control register to flags

	; code segment GDT offset(0x8) + OS image base 0x10000 + PROTECTEDMODE offset
	jmp dword 0x08: ( PROTECTEDMODE - $$ + 0x10000 )

;====================================================
;                 ENTER PROTECED MODE
;====================================================
[BITS 32]
PROTECTEDMODE:
	mov ax, 0x10	; set ax to data segment discriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ss, ax
	mov esp, 0xFFFE
	mov ebp, 0xFFFE

	push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000 )
	push 2
	push 0
	call PRINTMESSAGE
	add esp, 12

	jmp dword 0x08: 0x10200 ; C language kernel



;====================================================
;              FUNCTION CODE SECTION
;====================================================
PRINTMESSAGE:
	push ebp
	mov ebp, esp
	push esi
	push edi
	push eax
	push ecx
	push edx

	mov eax, dword [ ebp + 12 ]	; set EAX to param 2 (Y)
	mov esi, 160				; one line = 160 byte
	mul esi
	mov edi, eax				; eax*esi -> edi

	mov eax, dword [ ebp + 8 ]	; set EAX to param 1 (X)
	mov esi, 2					; 1 char = char+property(2byte)
	mul esi
	add edi, eax

	mov esi, dword [ ebp + 16 ]	; param 3 (MESSAGE)

.MESSAGELOOP:
	mov cl, byte [ esi ] ; CX(CH+CL)

	cmp cl, 0			;end of string
	je .MESSAGEEND

	mov byte [ edi + 0xB8000 ], cl

	add esi, 1
	add edi, 2

	jmp .MESSAGELOOP
.MESSAGEEND:
	pop edx
	pop ecx
	pop eax
	pop edi
	pop esi
	pop ebp
	ret

;====================================================
;             		DATA SECTION
;====================================================

align 8, db 0

dw 0x0000

GDTR:
	dw GDTEND - GDT - 1			; GDT table total size
	dd ( GDT - $$ + 0x10000 )	; GDT table start address

GDT:
	; Null descriptor (reserved descriptor)
	NULLDescriptor:
		dw 0x0000
		dw 0x0000
		db 0x00
		db 0x00
		db 0x00
		db 0x00

	; code segment descriptor
	CODEDESCRIPTOR:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x9A
		db 0xCF
		db 0x00

	; data segment descriptor
	DATADESCRIPTOR:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92
		db 0xCF
		db 0x00
GDTEND:

SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success', 0

times 512 - ( $ - $$ ) db 0x00




