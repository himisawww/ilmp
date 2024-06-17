include <asm_windows>

.code
	ALIGN 16
ilmp_shl_c_ proc
win	mov rax,[rsp+40]
lin mov rax,r8
	jmp lab_ent
ilmp_shl_c_ endp

	ALIGN 16
ilmp_shl_ proc
	xor rax,rax
lab_ent::
lin mov r8,rdx
lin mov r9,rdi
lin mov rdx,rsi
win	xchg rcx,r9
	and cl,63
	push r12
	push r13
	push rax
	xor rax,rax
	mov r10,[rdx+r8*8-8]
	shld rax,r10,cl

	test r8b,1
	jnz lab_lx1

lab_lx0:
	test r8b,2
	jnz lab_l10

lab_l00:
	lea rdx,[rdx+r8*8-32]
	lea r9,[r9+r8*8]
	mov r12,r10
	mov r13,[rdx+16]
	jmp lab_ll0

lab_l10:
	lea rdx,[rdx+r8*8-16]
	lea r9,[r9+r8*8-16]
	mov r11,[rdx]
	jmp lab_ll2

lab_lx1:
	test r8b,2
	jnz lab_l11

lab_l01:
	dec r8
	jz lab_e01
	lea rdx,[rdx+r8*8-32]
	lea r9,[r9+r8*8]
	mov r12,[rdx+24]
	mov r13,[rdx+16]
	shld r10,r12,cl
	mov [r9],r10
	jmp lab_ll1

lab_e01:
	pop r12
	shl r10,cl
	or r10,r12
	mov [r9],r10
	pop r13
	pop r12
	ret

lab_l11:
	mov r13,r10
	lea rdx,[rdx+r8*8-24]
	lea r9,[r9+r8*8+8]
	mov r10,[rdx+8]
	mov r11,[rdx]
	shld r13,r10,cl
	jmp lab_ll3

	ALIGN 16
lab_top:
	mov r12,[rdx-8]
	mov r13,[rdx-16]
	lea rdx,[rdx-32]
	shld r10,r11,cl
	shld r11,r12,cl
	mov [r9+8],r10
	mov [r9],r11
lab_ll1:
lab_ll0:
	mov r10,[rdx+8]
	mov r11,[rdx]
	shld r12,r13,cl
	shld r13,r10,cl
	mov [r9-8],r12
lab_ll3:
	mov [r9-16],r13
	lea r9,[r9-32]
lab_ll2:
	sub r8,4
	ja lab_top

lab_end:
	pop r12
	shld r10,r11,cl
	shl r11,cl
	or r11,r12
	mov [r9+8],r10
	mov [r9],r11
	pop r13
	pop r12
	ret
ilmp_shl_ endp
end
