section .data
	buf db 255

section .text
	global _start

___output:
	push eax
	push ebx
	push ecx
	push edx
	push ebp
	mov ebp, esp
	push dword 10
	mov eax, ecx
	mov ecx, 4
	mov ebx, eax
	xor edi, edi
	and ebx, 0x80000000
	jz ___loop
	mov dword edi, 1
	not eax
	add dword eax, 1

___loop:
	mov ebx, 10
	xor edx, edx
	div ebx
	add edx, 0x30
	push edx
	add ecx, 4
	cmp eax, 0
	jg ___loop
	cmp edi, 0
	jz ___output_exit
	add ecx, 4
	push dword 45
___output_exit:
	mov edx, ecx
	mov ecx, esp
	mov ebx, 1
	mov eax, 4
	int 0x80
	mov esp, ebp
	pop ebp
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

___input:
	push eax
	push ebx
	push edx
	push ebp
	mov ebp, esp
	mov ecx, buf
	mov ebx, 0
	mov edx, 255
	mov eax, 3
	int 0x80
	xor eax, eax
	xor ebx, ebx
	mov ebx, 10
	xor edx, edx
___to_dec:
	cmp byte [ecx], 10
	je ___exit_input
	mul ebx
	mov dl, byte [ecx]
	sub dl, 48
	add eax, edx
	inc ecx
	jmp ___to_dec
___exit_input:
	mov ecx, eax
	mov esp, ebp
	pop ebp
	pop edx
	pop ebx
	pop eax
	ret

fib:
	push ebp
	mov ebp, esp
	sub esp, 16
	mov eax, [ebp + 8]
	add eax, [ebp + 12]
	mov [esp + 4], eax
	mov ecx, [ebp + 12]
	call ___output
	mov ebx, [ebp + 16]
	cmp dword [esp + 4], ebx
	jg __L1
	mov dword [esp + 8], 0
	jmp __L2
__L1:
	mov dword [esp + 8], 1
__L2:
	cmp dword [esp + 8], 0
	je __L3
	mov eax, [esp + 4]
	mov esp, ebp
	pop ebp
	ret

	jmp __L4
__L3:
	mov eax, [ebp + 16]
	push eax
	mov eax, [esp + 8]
	push eax
	mov eax, [ebp + 12]
	push eax
	call fib
	add esp, 12
	mov [esp + 12], eax
	mov eax, [esp + 12]
	mov esp, ebp
	pop ebp
	ret

__L4:
	add esp, 28
_start:
	push ebp
	mov ebp, esp
	sub esp, 28
	call ___input
	mov [esp + 8], ecx
	mov dword [esp + 12], 1
	mov eax, [esp]
	mov eax, [esp + 12]
	mov dword [esp + 16], 1
	mov ebx, [esp + 4]
	mov ebx, [esp + 16]
	mov [esp], eax
	mov [esp + 4], ebx
	mov [esp + 8], ecx
	mov eax, [esp + 8]
	push eax
	mov eax, [esp + 8]
	push eax
	mov eax, [esp + 8]
	push eax
	call fib
	add esp, 12
	mov [esp + 20], eax
	mov ecx, [esp + 20]
	call ___output
	mov dword [esp + 24], 0
	mov eax, 1
	int 0x80
	ret

	add esp, 28
