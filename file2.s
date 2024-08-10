global _start

section .data
msg:
	db "A"
	value equ "AZ"

section .text
_start:
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, 1

	mov r8, value
loop:
	syscall
	dec r8
	cmp r8, 0
	jne loop

	mov rax, 15 * 4
	mov rdi, 0
	syscall

