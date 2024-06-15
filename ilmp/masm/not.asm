
.code
	ALIGN 16
ilmp_not_ proc
	mov r10,[rdx]
	mov rax,r8
	lea rcx,[rcx+r8*8]
	lea rdx,[rdx+r8*8]
	neg r8
	and al,3
	je lab_b00
	cmp al,2
	jc lab_b01
	je lab_b10

lab_b11:
	not r10
	mov [rcx+r8*8],r10
	dec r8
	jmp lab_e11
lab_b10:
	add r8,-2
	jmp lab_e10

lab_b01:
	not r10
	mov [rcx+r8*8],r10
	inc r8
	jz lab_ret

	ALIGN 16
lab_oop:
	mov r10,[rdx+r8*8]
lab_b00:
	mov r11,[rdx+r8*8+8]
	not r10
	not r11
	mov [rcx+r8*8],r10
	mov [rcx+r8*8+8],r11
lab_e11:
	mov r10,[rdx+r8*8+16]
lab_e10:
	mov r11,[rdx+r8*8+24]
	not r10
	not r11
	mov [rcx+r8*8+16],r10
	mov [rcx+r8*8+24],r11
	add r8,4
	jnc lab_oop
lab_ret:
	ret
ilmp_not_ endp
end
