section .data
	buf db 255

section .text
	global _start

___output:
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	mov rbp, rsp
	push qword 10
	mov rax, rcx
	mov rcx, 8
	mov rbx, rax
	xor rdi, rdi
	mov rdx, 0x8000000000000000
	and rbx, rdx
	jz ___loop
	mov qword rdi, 1
	not rax
	add qword rax, 1

___loop:
	mov rbx, 10
	xor rdx, rdx
	div rbx
	add rdx, 0x30
	push rdx
	add rcx, 8
	cmp rax, 0
	jg ___loop
	cmp rdi, 0
	jz ___output_exit
	add rcx, 8
	push qword 45
___output_exit:
	mov rdx, rcx
	mov rsi, rsp
	mov rdi, 1
	mov rax, 1
	syscall
	mov rsp, rbp
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret

___input:
	push rax
	push rbx
	push rdx
	push rbp
	mov rbp, rsp
	mov rsi, buf
	mov rdx, 255
	mov rdi, 0
	mov rax, 0
	syscall
	xor rax, rax
	xor rbx, rbx
	mov rbx, 10
	xor rdx, rdx
___to_dec:
	cmp byte [rsi], 10
	je ___exit_input
	mul rbx
	mov dl, byte [rsi]
	sub dl, 48
	add rax, rdx
	inc rsi
	jmp ___to_dec
___exit_input:
	mov rcx, rax
	mov rsp, rbp
	pop rbp
	pop rdx
	pop rbx
	pop rax
	ret

fib:
	push rbp
	mov rbp, rsp
	sub rsp, 32
	mov rax, [rbp + 16]
	add rax, [rbp + 24]
	mov [rsp + 8], rax
	mov rcx, [rbp + 24]
	call ___output
	mov rbx, [rbp + 32]
	cmp qword [rsp + 8], rbx
	jg __L1
	mov qword [rsp + 16], 0
	jmp __L2
__L1:
	mov qword [rsp + 16], 1
__L2:
	cmp qword [rsp + 16], 0
	je __L3
	mov rax, [rsp + 8]
	mov rsp, rbp
	pop rbp
	ret

	jmp __L4
__L3:
	mov rax, [rbp + 32]
	push rax
	mov rax, [rsp + 16]
	push rax
	mov rax, [rbp + 24]
	push rax
	call fib
	add rsp, 24
	mov [rsp + 24], rax
	mov rax, [rsp + 24]
	mov rsp, rbp
	pop rbp
	ret

__L4:
	add rsp, 56
_start:
	push rbp
	mov rbp, rsp
	sub rsp, 56
	call ___input
	mov [rsp + 16], rcx
	mov qword [rsp + 24], 1
	mov rax, [rsp]
	mov rax, [rsp + 24]
	mov qword [rsp + 32], 1
	mov rbx, [rsp + 8]
	mov rbx, [rsp + 32]
	mov [rsp], rax
	mov [rsp + 8], rbx
	mov [rsp + 16], rcx
	mov rax, [rsp + 16]
	push rax
	mov rax, [rsp + 16]
	push rax
	mov rax, [rsp + 16]
	push rax
	call fib
	add rsp, 24
	mov [rsp + 40], rax
	mov rcx, [rsp + 40]
	call ___output
	mov qword [rsp + 48], 0
	mov rax, 60
	mov rdi, 0
	syscall
	ret

	add rsp, 56
