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

swap:
	push ebp
	mov ebp, esp
	sub esp, 40
	mov eax, [ebp + 8]
	mov eax, [eax]
	mov ecx, eax
	call ___output
	mov ebx, [ebp + 12]
	mov ebx, [ebx]
	mov ecx, ebx
	call ___output
	mov ecx, [ebp + 8]
	mov ecx, [ecx]
	mov [esp], ecx
	mov edx, [ebp + 12]
	mov edx, [edx]
	mov [esp + 4], eax
	mov eax, [ebp + 8]
	mov [eax], edx
	mov [ebp + 8], eax
	mov eax, [ebp + 12]
	mov [esp + 8], ebx
	mov ebx, [esp]
	mov [eax], ebx
	mov [ebp + 12], eax
	mov eax, [ebp + 8]
	mov eax, [eax]
	mov [esp + 12], ecx
	mov ecx, eax
	call ___output
	mov ecx, [ebp + 12]
	mov ecx, [ecx]
	mov [esp + 32], ecx
	mov ecx, [esp + 32]
	call ___output
	mov dword [esp + 36], 0
	mov eax, dword [esp + 36]
	mov esp, ebp
	pop ebp
	ret

	add esp, 40
_start:
	push ebp
	mov ebp, esp
	sub esp, 48
	mov dword [esp], 0
	mov eax, [esp]
	mov [esp + 4], eax
	mov dword [esp + 8], 1
	mov ebx, [esp + 8]
	mov [esp + 12], ebx
	mov ecx, [esp + 4]
	call ___output
	mov ecx, [esp + 12]
	call ___output
	mov ecx, esp
	add ecx, 4
	mov [esp + 16], ecx
	mov ecx, [esp + 16]
	call ___output
	mov ecx, esp
	add ecx, 12
	mov [esp + 20], ecx
	mov ecx, [esp + 20]
	call ___output
	mov ecx, esp
	add ecx, 4
	mov edx, esp
	add edx, 12
	mov [esp], eax
	mov [esp + 8], ebx
	mov [esp + 24], ecx
	mov [esp + 28], edx
	mov eax, [esp + 28]
	push eax
	mov eax, [esp + 28]
	push eax
	call swap
	add esp, byte 8
	mov [esp + 32], eax
	mov ecx, [esp + 4]
	call ___output
	mov ecx, [esp + 12]
	call ___output
	mov eax, esp
	add eax, 4
	mov ecx, eax
	call ___output
	mov ebx, esp
	add ebx, 12
	mov ecx, ebx
	call ___output
	mov dword [esp + 44], 0
	mov eax, 1
	int 0x80
	ret

	add esp, 48
