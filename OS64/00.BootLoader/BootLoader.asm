[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START	;CS=0x07C0, goto START

TOTALSECTORCOUNT: dw 1024	;OS IMAGE SIZE (exept BootLoader), MAX 1152 SECTOR(0x90000byte)

;====================================================
;                    CODE SECTION
;====================================================
START:
	mov ax, 0x07C0
	mov ds, ax		; DS=0x07C00 BootLoader start address
	mov ax, 0xB800
	mov es, ax		; ES=0xB8000 VIDEO MEMORY start address

	; create stack 0x0000:0000~0x0000:FFFF of 64kb size
	mov ax, 0x0000
	mov ss, ax      ;SS(Stack Segment)=0x0000
	mov sp, 0xFFFE
	mov bp, 0xFFFE	; sp, bp=0xFFFE

	;====================================================
	;                  CLEAR SCREEN
	;====================================================
	mov si, 0 ; init SI

.SCREENCLEARLOOP:
	mov byte [ es: si ] , 0
	mov byte [ es: si+1 ], 0x0F
	add si, 2

	cmp si, 80 * 25 * 2 ; 4000
	jl .SCREENCLEARLOOP


	;====================================================
	;              PRINT STARTING MESSAGE
	;====================================================
	push MESSAGE1
	push 0
	push 0				;parameter MESSAGE, Y, X
	call PRINTMESSAGE	;call Function
	add sp,6			;stack clearing (cdecl)

	;====================================================
	;           PRINT OS IMAGE LOADING MESSAGE
	;====================================================
	push IMAGELOADINGMESSAGE
	push 1
	push 0
	call PRINTMESSAGE
	add sp,6

	;====================================================
	;            OS IMAGE LOADING FROM DISK
	;====================================================
RESETDISK: ;Disk reset
	;====================================================
	;            CALL BIOS RESET FUNCTION
	;====================================================
	mov ax, 0
	mov dl, 0			; service number 0, drive number(0=Floppy)
	int 0x13 			; Software Interrupt : Disk I/O service
	jc HANDLEDISKERROR	; Error Exception

	;====================================================
	;             READ SECTOR FROM DISK
	;====================================================
	; Set address(ES:BX) to copy OS image to memory(0x10000)
	mov si, 0x1000	; *16 -> 0x10000 physical address
	mov es, si
	mov bx, 0x0000	;address : 0x10000:0000

	mov di, word [ TOTALSECTORCOUNT ]	;set DI register to OS image count to copy

READDATA:
	cmp di, 0		;loop until OS image count is 0
	je READEND		;if count is 0, end READDATA
	sub di, 0x1		;decrease count

	;====================================================
	;             CALL BIOS READ FUNCTION
	;====================================================
	mov ah, 0x02					;BIOS service number(Read Sector)
	mov al, 0x1						;read sector number 1
	mov ch, byte[ TRACKNUMBER ]		;set track number
	mov cl, byte[ SECTORNUMBER ]	;set sector number
	mov dh, byte[ HEADNUMBER ]		;set header number
	mov dl, 0x00					;set drive number(0=Floppy)
	int 0x13						;call interrupt service
	jc HANDLEDISKERROR				;error exception

	;====================================================
	;  CALCULATE ADDRESS TO COPY & TRACK,HEAD,SECCTOR
	;====================================================
	add si, 0x0020	; read 512(0x200), so convert to segment register value
	mov es, si		; increase 512

	; SECTORNUMBER++
	mov al, byte [ SECTORNUMBER ]
	add al, 0x01
	mov byte [ SECTORNUMBER ], al
	cmp al, 19						; if NOT last sector(18), continue to read sector
	jl READDATA

	; HEADNUMBER TOGGLE (0->1, 1->0), set SECTORNUMER 1
	xor byte [ HEADNUMBER ], 0x01
	mov byte [ SECTORNUMBER ], 0x01

	; if HEADNUMBER TOGGLED 1->0, both header read completed -> TRACKNUMBER++
	cmp byte [ HEADNUMBER ], 0x00
	jne READDATA

	add byte [ TRACKNUMBER ], 0x01
	jmp READDATA
READEND:

	;====================================================
	;      PRINT OS IMAGE LOADING COMPETE MESSAGE
	;====================================================
	push LOADINGCOMPLETEMESSAGE
	push 1
	push 20
	call PRINTMESSAGE
	add sp, 6

	;====================================================
	;              EXECUTE LOADED OS IMAGE
	;====================================================
	jmp 0x1000:0x0000


	;====================================================
	;              FUNCTION CODE SECTION
	;====================================================
	; EXCEPTION HANDLING FUNCTION
	HANDLEDISKERROR:
		push DISKERRORMESSAGE	; push error message
		push 1					; Y
		push 20					; X
		call PRINTMESSAGE

		jmp $					; loooooooooop

	; PRINT MESSAGE FUNCTION
	; parameter : x coordinate, y coordinate, string
	PRINTMESSAGE:
		push bp
		mov sp, bp	; function prologue


		push es		; push ES ~ DX register
		push si		; temp register in function, restore in last part of function
		push di
		push ax
		push cx
		push dx

		mov ax, 0xB800	;video memory address (0xB8000)
		mov es, ax

		; calculate video memory addres with x, y coordinate
		mov ax, word [ bp + 6 ]	; set AX to param 2 (Y)
		mov si, 160				; one line = 160 byte
		mul si
		mov di, ax				; ax*si -> di

		mov ax, word [ bp + 4 ]	; set AX to param 1 (X)
		mov si, 2				; 1 char = char+property(2byte)
		mul si
		add di, ax

		mov si, word [ bp + 8 ]	; param 3 (MESSAGE)

.MESSAGELOOP:
	mov cl, byte [ si ] ; CX(CH+CL)

	cmp cl, 0			;end of string
	je .MESSAGEEND

	mov byte [ es:di ], cl

	add si, 1
	add di, 2

	jmp .MESSAGELOOP
.MESSAGEEND:
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es
	pop bp
	ret


;====================================================
;             		DATA SECTION
;====================================================

MESSAGE1:  				db  'OS Boot Loader Start....',0

DISKERRORMESSAGE:		db  'DISK Error!',0
IMAGELOADINGMESSAGE:	db	'OS Image Loading...',0
LOADINGCOMPLETEMESSAGE:	db	'Complete',0

SECTORNUMBER:			db	0x02
HEADNUMBER:				db	0x00
TRACKNUMBER:				db	0x00


times 510 - ( $-$$ )	db	0x00

db 0x55
db 0xAA

