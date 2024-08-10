global _start

section .data
msg:
	dq 0x1234
	db 3
	dd 3 * 2 + 1
	dq 5 - 2 * 100 + 800 / 8 + 100 + 50 + 50
	; db "Hello world!"
	skibid equ -5895
	toilet equ 239 + 1384 * 329 + 4123 - 232 - 13294 / 234 + 4 * 9 / 2
	msglen equ $-msg
	thing equ msg + 5
	stringexample equ "AZ"
	; otherthig equ msg * 5
	; otherthing equ 5 * 2334
	; asdofaf equ otadsfherthing

section .text
_start:
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, msglen
	syscall

	mov rbx, [5000 + 100 + 8 * rbx]
	mov rbx, [5858 + 8 * rbx + 100]
	mov rcx, 12 + 2348 * 12 + 49294 + 50
	mov rcx, 58 * 2
	mov rcx, 12 + 2348 * 12 + 49294 + 50 / 5
	mov rdx, 1 * 2 * 3 * 4 * 5 * 6 * 7
	mov rax, 5 + 295 + 328 + 4923 + 1283
	mov rsi, 100000 - 1 - 5 - 1 - 1 - 5 - 12394
	mov rdx, 42 + 2 * 10 + 9 - 2

	mov rax, 15 * 4
	mov rdi, 0
	syscall

