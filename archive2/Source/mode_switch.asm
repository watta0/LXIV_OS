[BITS 32]

global kread_cpuid, kswitch_exe_k64

SECTION .text

;cpuid, &4bytes, &4bytes, &4bytes
kread_cpuid:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx
	push esi

	mov eax, dword [ebp + 8]
	cpuid

	mov esi, dword [ebp + 12]
	mov dword [esi], eax

	mov esi, dword [ebp + 16]
	mov dword [esi], ebx

	mov esi, dword [ebp + 20]
	mov dword [esi], ecx

	mov esi, dword [ebp + 24]
	mov dword [esi], edx

	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	;mov esp, ebp
	pop ebp
	ret

kswitch_exe_k64:
	mov eax, cr4
	or eax, 0x20	;PAE bit 1
	mov cr4, eax

	mov eax, 0x100000	;1MB PML4
	mov cr3, eax

	mov ecx, 0xc0000080	
	rdmsr
	or eax, 0x0100	;IA-32e MSR register lower 32bits LME bits 1
	wrmsr

	mov eax, cr0
	or eax, 0xe0000000	;NW(not write through) bits 1, CD(cache disable) bits 1, PG(page enable) bits 1
	xor eax, 0x60000000	;NW, CD xor (always make 0)
	mov cr0, eax

	jmp 0x08:0x200000	;2MB k64 start address

	;won't executed
	jmp $