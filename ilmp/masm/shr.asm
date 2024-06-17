include <asm_windows>

.code
	ALIGN 16
ilmp_shr_c_ proc
win	mov rax,[rsp+40]
lin mov rax,r8
	jmp lab_ent
ilmp_shr_c_ endp

	ALIGN 16
ilmp_shr_ proc
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
	mov r10,[rdx]
	xor rax,rax
	lea r9,[r9+r8*8]
	lea rdx,[rdx+r8*8]
	shrd rax,r10,cl
	neg r8

	test r8b,1
	jnz lab_rx1
	
lab_rx0:
	test r8b,2
	jnz lab_r10

lab_r00:
	mov r12,r10
	mov r13,[rdx+r8*8+8]
	jmp lab_rl0

lab_r10:
	mov r11,[rdx+r8*8+8]
	add r8,2
	jmp lab_rl2

lab_rx1:
	test r8b,2
	jz lab_r11

lab_r01:
	inc r8
	jz lab_e01
	mov r12,[rdx+r8*8]
	mov r13,[rdx+r8*8+8]
	shrd r10,r12,cl
	mov [r9+r8*8-8],r10
	jmp lab_rl1

lab_e01:
	pop r12
	shr r10,cl
	or r10,r12
	mov [r9-8],r10
	pop r13
	pop r12	
	ret

lab_r11:
	mov r13,r10
	mov r10,[rdx+r8*8+8]
	mov r11,[rdx+r8*8+16]
	shrd r13,r10,cl
	dec r8
	jmp lab_rl3

	ALIGN 16
lab_top:
	mov r12,[rdx+r8*8]
	mov r13,[rdx+r8*8+8]
	shrd r10,r11,cl
	shrd r11,r12,cl
	mov [r9+r8*8-16],r10
	mov [r9+r8*8-8],r11
lab_rl1:
lab_rl0:
	mov r10,[rdx+r8*8+16]
	mov r11,[rdx+r8*8+24]
	shrd r12,r13,cl
	shrd r13,r10,cl
	mov [r9+r8*8],r12
lab_rl3:
	mov [r9+r8*8+8],r13
	add r8,4
lab_rl2:
	jnz lab_top

lab_end:
	pop r12
	shrd r10,r11,cl
	shr r11,cl
	or r11,r12
	mov [r9-16],r10
	mov [r9-8],r11

	pop r13
	pop r12
	ret
ilmp_shr_ endp
end
