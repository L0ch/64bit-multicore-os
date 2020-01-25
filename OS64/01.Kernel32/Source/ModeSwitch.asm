[BITS 32]

global kReadCPUID		;Export for C Kernel
global kSwitchAndExecute64bitKernel
SECTION .text

;return CPUID
;	PARAM: DWORD dwEAX, DWORD* pdwEAX, * pdwEBX, * pdwECX, * pdwEDX
kReadCPUID:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx
	push esi

	; execute CPUID opcode
	mov eax, dword[ebp + 8] ; parameter 1(dwEAX)
	cpuid

	; return value
	; *pdwEAX
	mov esi, dword[ebp+12]	; parameter 2(pdwEAX)
	mov dword[esi], eax
	; *pdwEBX
	mov esi, dword[ebp+16]	; parameter 3(pdwEBX)
	mov dword[esi], ebx
	; *pdwECX
	mov esi, dword[ebp+20]	; parameter 4(pdwECX)
	mov dword[esi], ecx
	; *pdwEDX
	mov esi, dword[ebp+24]	; parameter 5(pdwEDX)
	mov dword[esi], edx

	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	pop ebp
	ret

; Switch to IA-32e Mode
kSwitchAndExecute64bitKernel:
	; PAE bit of CR4 register = 1
	mov eax, cr4
	or eax, 0x620	; PAE bit(bit 5), OSXMMEXCPT bit(bit 10), OSFXSR bit(bit 9) 1
	mov cr4, eax

	; CR3 = PML4 Table Address
	mov eax, 0x100000	; 0x100000(1MB) = PML4 Table
	mov cr3, eax

	; Activate IA-32e mode
	; Set IA32_EFER.LME bit to 1
	mov ecx, 0xC0000080	; Address of IA32_EFER MSR register
	rdmsr				; Read MSR register

	or eax, 0x0100		; Set LME bit of Lower 32bit to 1

	wrmsr				; Write MSR register


	; Set CR0 control register to activate cache, paging, FPU
	mov eax, cr0
	or eax, 0xE000000E		; Set NW(29), CD(30), PG(31), TS(3), EM(2), MP(1) 1
	xor eax, 0x60000004		; Set NW, CD, EM 0
	mov cr0, eax

	jmp 0x08:0x200000		; Change CS segment selector to IA-32e Mode code segment descriptor
							; jmp 0x200000(2MB) address

	jmp $













