[BITS 64]

SECTION .text

global ktransmit_kbd_byte, kreceive_kbd_byte

kreceive_kbd_byte:
	push rdx

	mov rdx, rdi
	mov rax, 0
	in al, dx

	pop rdx
	ret



ktransmit_kbd_byte:
	push rdx
	push rax

	mov rdx, rdi
	mov rax, rsi
	out dx, al

	pop rax
	pop rdx
	ret
