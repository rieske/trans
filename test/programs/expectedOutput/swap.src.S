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

swap:
	push rbp
	mov rbp, rsp
	sub rsp, 144
	mov rax, [rbp + 16]
	mov rbx, [rax]
	mov rcx, [rbp + 16]
	mov [rsp + 16], rcx
	mov rcx, rbx
	call ___output
	mov rbx, [rbp + 24]
	mov rdx, [rbx]
	mov [rbp + 16], rax
	mov rax, [rbp + 24]
	mov [rsp + 8], rcx
	mov rcx, rdx
	call ___output
	mov rdx, [rbp + 16]
	mov [rsp + 32], rax
	mov rax, [rdx]
	mov [rsp + 40], rax
	mov rax, [rbp + 16]
	mov [rsp + 48], rax
	mov rax, [rsp]
	mov rax, [rsp + 40]
	mov [rsp], rax
	mov rax, [rdx]
	mov [rsp + 56], rax
	mov rax, [rbp + 16]
	mov [rsp + 64], rax
	mov rax, [rbx]
	mov [rsp + 72], rax
	mov rax, [rbp + 24]
	mov [rsp + 80], rax
	mov rax, [rsp + 72]
	mov [rbp + 24], rbx
	mov rbx, [rsp + 64]
	mov [rbx], rax
	mov [rsp + 72], rax
	mov rax, [rbp + 24]
	mov [rsp + 64], rbx
	mov rbx, [rax]
	mov [rbp + 24], rax
	mov rax, [rbp + 24]
	mov [rsp + 96], rax
	mov rax, [rsp]
	mov [rsp + 88], rbx
	mov rbx, [rsp + 96]
	mov [rbx], rax
	mov [rsp], rax
	mov rax, [rdx]
	mov [rsp + 104], rax
	mov rax, [rbp + 16]
	mov [rsp + 24], rcx
	mov rcx, [rsp + 104]
	call ___output
	mov [rsp + 112], rax
	mov rax, [rbp + 24]
	mov [rsp + 96], rbx
	mov rbx, [rax]
	mov [rbp + 24], rax
	mov rax, [rbp + 24]
	mov [rsp + 104], rcx
	mov rcx, rbx
	call ___output
	mov qword [rsp + 136], 0
	mov rax, [rsp + 136]
	mov rsp, rbp
	pop rbp
	ret

	add rsp, 160
_start:
	push rbp
	mov rbp, rsp
	sub rsp, 96
	mov qword [rsp], 0
	mov rax, [rsp + 8]
	mov rax, [rsp]
	mov qword [rsp + 16], 1
	mov rbx, [rsp + 24]
	mov rbx, [rsp + 16]
	mov rcx, rax
	call ___output
	mov [rsp + 8], rcx
	mov rcx, rbx
	call ___output
	mov rax, rsp
	add rax, 8
	mov [rsp + 24], rcx
	mov rcx, rax
	call ___output
	mov rax, rsp
	add rax, 24
	mov [rsp + 32], rcx
	mov rcx, rax
	call ___output
	mov rax, rsp
	add rax, 8
	mov rbx, rsp
	add rbx, 24
	mov [rsp + 48], rax
	mov [rsp + 56], rbx
	mov [rsp + 40], rcx
	mov rax, [rsp + 56]
	push rax
	mov rax, [rsp + 56]
	push rax
	call swap
	add rsp, 16
	mov [rsp + 64], rax
	mov rcx, [rsp + 8]
	call ___output
	mov [rsp + 8], rcx
	mov rcx, [rsp + 24]
	call ___output
	mov rax, rsp
	add rax, 8
	mov [rsp + 24], rcx
	mov rcx, rax
	call ___output
	mov rax, rsp
	add rax, 24
	mov [rsp + 72], rcx
	mov rcx, rax
	call ___output
	mov qword [rsp + 88], 0
	mov rax, 60
	mov rdi, 0
	syscall
	ret

	add rsp, 96
