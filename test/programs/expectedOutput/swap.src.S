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
	sub esp, 72
	mov eax, [ebp + 8]
	mov ebx, [eax]
	mov ecx, [ebp + 8]
	mov [esp + 8], ecx
	mov ecx, ebx
	call ___output
	mov ebx, [ebp + 12]
	mov edx, [ebx]
	mov [ebp + 8], eax
	mov eax, [ebp + 12]
	mov [esp + 4], ecx
	mov ecx, edx
	call ___output
	mov edx, [ebp + 8]
	mov [esp + 16], eax
	mov eax, [edx]
	mov [esp + 20], eax
	mov eax, [ebp + 8]
	mov [esp + 24], eax
	mov eax, [esp]
	mov eax, [esp + 20]
	mov [esp], eax
	mov eax, [edx]
	mov [esp + 28], eax
	mov eax, [ebp + 8]
	mov [esp + 32], eax
	mov eax, [ebx]
	mov [esp + 36], eax
	mov eax, [ebp + 12]
	mov [esp + 40], eax
	mov eax, [esp + 36]
	mov [ebp + 12], ebx
	mov ebx, [esp + 32]
	mov [ebx], eax
	mov [esp + 36], eax
	mov eax, [ebp + 12]
	mov [esp + 32], ebx
	mov ebx, [eax]
	mov [ebp + 12], eax
	mov eax, [ebp + 12]
	mov [esp + 48], eax
	mov eax, [esp]
	mov [esp + 44], ebx
	mov ebx, [esp + 48]
	mov [ebx], eax
	mov [esp], eax
	mov eax, [edx]
	mov [esp + 52], eax
	mov eax, [ebp + 8]
	mov [esp + 12], ecx
	mov ecx, [esp + 52]
	call ___output
	mov [esp + 56], eax
	mov eax, [ebp + 12]
	mov [esp + 48], ebx
	mov ebx, [eax]
	mov [ebp + 12], eax
	mov eax, [ebp + 12]
	mov [esp + 52], ecx
	mov ecx, ebx
	call ___output
	mov dword [esp + 68], 0
	mov eax, [esp + 68]
	mov esp, ebp
	pop ebp
	ret

	add esp, 80
_start:
	push ebp
	mov ebp, esp
	sub esp, 48
	mov dword [esp], 0
	mov eax, [esp + 4]
	mov eax, [esp]
	mov dword [esp + 8], 1
	mov ebx, [esp + 12]
	mov ebx, [esp + 8]
	mov ecx, eax
	call ___output
	mov [esp + 4], ecx
	mov ecx, ebx
	call ___output
	mov eax, esp
	add eax, 4
	mov [esp + 12], ecx
	mov ecx, eax
	call ___output
	mov eax, esp
	add eax, 12
	mov [esp + 16], ecx
	mov ecx, eax
	call ___output
	mov eax, esp
	add eax, 4
	mov ebx, esp
	add ebx, 12
	mov [esp + 24], eax
	mov [esp + 28], ebx
	mov [esp + 20], ecx
	mov eax, [esp + 28]
	push eax
	mov eax, [esp + 28]
	push eax
	call swap
	add esp, 8
	mov [esp + 32], eax
	mov ecx, [esp + 4]
	call ___output
	mov [esp + 4], ecx
	mov ecx, [esp + 12]
	call ___output
	mov eax, esp
	add eax, 4
	mov [esp + 12], ecx
	mov ecx, eax
	call ___output
	mov eax, esp
	add eax, 12
	mov [esp + 36], ecx
	mov ecx, eax
	call ___output
	mov dword [esp + 44], 0
	mov eax, 1
	int 0x80
	ret

	add esp, 48
