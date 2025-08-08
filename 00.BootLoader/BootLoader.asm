[ORG 0X00]
[BITS 16]

SECTION .text

jmp 0x7c0:START

TOTAL_SECTOR_CNT: dw 2

K32_SECTOR_CNT: dw 2

START:
	mov ax, 0x7c0
	mov ds, ax

	mov ax, 0xb800
	mov es, ax

	mov ax, 0x0000
	mov ss, ax

	mov sp, 0xfffe
	mov bp, 0xfffe

	mov si, 0x00

	call CLEAR_SCREEN

	push START_MSG
	push 0
	push 0
	call PRINT_MSG
	add sp, 6

	push LOADING_MSG
	push 1
	push 0
	call PRINT_MSG
	add sp, 6


RESET_DISK:
	mov ax, 0
	mov dl, 0
	int 0x13

	jc HANDLE_DISK_ERROR

	mov si, 0x1000
	mov es, si
	mov bx, 0x00	;es:bx, es(0x1000*0x10) + bx(0x0000)

	mov di, word [TOTAL_SECTOR_CNT]

READ_DATA:
	
	cmp di, 0
	je READ_END
	sub di, 1

	mov ah, 2
	mov al, 1
	mov ch, byte [TRACK_NUM]
	mov cl, byte [SECTOR_NUM]
	mov dh, byte [HEAD_NUM]
	mov dl, 0x00

	int 0x13
	jc HANDLE_DISK_ERROR

	add si, 0x0020
	mov es, si


	mov al, byte [SECTOR_NUM]	;왜 HEAD_NUM와 다르게, 굳이 al에 옮겨서 비교하는지 모르겠음
	add al, 1
	mov byte [SECTOR_NUM], al
	cmp al, 19
		
	jl READ_DATA

	mov byte [SECTOR_NUM], 1
	xor byte [HEAD_NUM], 0x01
	cmp byte [HEAD_NUM], 0
	jne READ_DATA

	add byte [TRACK_NUM], 1
	cmp byte [TRACK_NUM], 80
	jl READ_DATA

	mov byte [TRACK_NUM], 0
	jmp READ_DATA
	
HANDLE_DISK_ERROR:
	push DISK_ERROR_MSG
	push 2
	push 0
	call PRINT_MSG
	add sp, 6

	jmp $

READ_END:

	push LOADING_COMPLETE_MSG
	push 1
	push 29
	call PRINT_MSG
	add sp, 6

	jmp 0x1000:0x0000	;로딩한 가상 이미지로 점프(실행)

PRINT_MSG:
	push bp
	mov bp, sp

	push es
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xb800
	mov es, ax

	mov ax, word [bp+4]
	mov si, 2
	mul si
	mov di, ax

	mov ax, word [bp+6]
	mov si, 2*80
	mul si
	add di, ax

	mov si, word [bp+8]

.PRINT_MSG_LOOP:

	mov cl, byte [si]
	cmp cl, 0
	je .PRINT_MSG_END

	mov byte [es: di], cl

	add si, 1
	add di, 2

	jmp .PRINT_MSG_LOOP

.PRINT_MSG_END:
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es

	mov sp, bp
	pop bp
	ret


CLEAR_SCREEN:
	push bp
	mov bp, sp

	push es
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xb800
	mov es, ax
	mov di, 0

.SCREEN_CLEAR_LOOP:

	mov byte [es: di], 0
	mov byte [es: di+1], 0x0a 
	
	add di, 2

	cmp di, 80 * 25 * 2
	jl .SCREEN_CLEAR_LOOP

	jmp .SCREENCLEAREND
.SCREENCLEAREND:
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es

	mov sp, bp
	pop bp
	ret

START_MSG: db 'LXIV OS Boot Loader Start', 0
LOADING_MSG: db 'LXIV OS Image Loading...', 0
DISK_ERROR_MSG: db 'Disk Loading Error', 0
LOADING_COMPLETE_MSG: db 'Complete', 0

SECTOR_NUM:		db	0x02 	;first 512bytes is bootloader, start with 1
HEAD_NUM:		db	0x00
TRACK_NUM:		db	0x00

times 510 - ( $ - $$ )	db 0x00

db 0x55
db 0xaa