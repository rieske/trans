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

_start:
	push rbp
	mov rbp, rsp
	sub rsp, 72
	mov qword [rsp], 1
	mov rax, [rsp + 8]
	mov rax, [rsp]
	mov rcx, rax
	call ___output
	mov rax, rcx
	not rax
	add rax, 1
	mov [rsp + 8], rcx
	mov rcx, rax
	call ___output
	mov rax, [rsp + 8]
	not rax
	add rax, 1
	mov rbx, [rsp + 8]
	not rbx
	add rbx, 1
	mov rdx, rbx
	not rdx
	add rdx, 1
	mov [rsp + 16], rcx
	mov rcx, rdx
	call ___output
	mov qword [rsp + 40], 2
	mov qword [rsp + 48], 5
	mov rdx, [rsp + 40]
	sub rdx, [rsp + 48]
	mov [rsp + 32], rcx
	mov rcx, rdx
	call ___output
	mov qword [rsp + 64], 0
	mov rax, 60
	mov rdi, 0
	syscall
	ret

	add rsp, 72
