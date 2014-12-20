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

_start:
	push ebp
	mov ebp, esp
	sub esp, 36
	mov dword [esp + 4], 1
	mov eax, [esp + 4]
	mov [esp], eax
	mov ecx, [esp]
	call ___output

	mov ebx, [esp]
	not ebx
	add dword ebx, 1
	mov ecx, ebx
	call ___output

	mov ecx, [esp]
	not ecx
	add dword ecx, 1

	mov edx, [esp]
	not edx
	add dword edx, 1
	mov [esp + 4], eax
	mov [esp + 12], ecx
	mov eax, [esp + 12]
	not eax
	add dword eax, 1
	mov [esp + 12], ecx
	mov ecx, eax
	call ___output
	mov dword [esp + 20], 2
	mov dword [esp + 24], 5
	mov ecx, [esp + 20]
	sub ecx, [esp + 24]
	mov [esp + 28], ecx
	mov ecx, [esp + 28]
	call ___output
	mov dword [esp + 32], 0
	mov eax, 1
	int 0x80
	ret

