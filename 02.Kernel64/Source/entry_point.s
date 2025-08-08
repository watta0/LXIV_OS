[BITS 64]

SECTION .text

extern main

START:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ss, ax
	mov rsp, 0x6ffff8	;6MB ~
	mov rbp, 0x6ffff8

	push ( SWITCH_COMPLETE_MSG - $$ + 0x200000 )
	push 10
	push 0
	call PRINT_MSG
	add rsp, 24

	call main

	jmp $


;글자 깨짐
PRINT_MSG:
	push rbp
	mov rbp, rsp
	push rsi
	push rdi
	push rax
	push rcx
	push rdx

	mov rax, qword [rbp + 24]
	mov rsi, 160
	mul rsi
	mov rdi, rax

	mov rax, qword [rbp + 16]
	mov rsi, 2
	mul rsi
	add rdi, rax

	mov rsi, qword [rbp + 32]

.PRINT_MSG_LOOP:
	mov cl, byte [rsi]

	cmp cl, 0
	je .PRINT_MSG_END

	mov byte [rdi + 0xb8000], cl

	add rdi, 2
	add rsi, 1

	jmp .PRINT_MSG_LOOP

.PRINT_MSG_END:
	pop rdx
	pop rcx
	pop rax
	pop rdi
	pop rsi
	mov rsp, rbp ; 로컬 변수를 사용하지 않았으므로 필요 없음 
	pop rbp
	ret

SWITCH_COMPLETE_MSG: db '[PASS ] Switch To IA-32e Mode' 0
