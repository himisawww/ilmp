include <asm_windows>

.code
	ALIGN 16
ilmp_sqr_basecase_ proc
	cmp rx2,2
	jae lab_gt1
	mov rax,[rx1]
	mul rax
	mov [rx0],rax
	mov [rx0+8],rdx
	ret

lab_gt1:
	jne lab_gt2
	mov rax,[rx1]
	mov r11,[rx1+8]
	mov r8,rax
	mul rax
	mov [rx0],rax
	mov rax,r11
	mov r9,rdx
	mul rax
	mov r10,rax
	mov rax,r11
	mov r11,rdx
	mul r8
	xor r8,r8
	add r9,rax
	adc r10,rdx
	adc r11,r8
	add r9,rax
	mov [rx0+8],r9
	adc r10,rdx
	mov [rx0+16],r10
	adc r11,r8
	mov [rx0+24],r11
	ret

lab_gt2:
win	push rsi
win	push rdi
	cmp rx2,4
win	mov rsi,rdx
win	mov rdi,rcx
win	mov rdx,r8
	jae lab_gt3
	mov rax,[rsi]
	mov r10,rax
	mul rax
	mov r11,[rsi+8]
	mov [rdi],rax
	mov rax,r11
	mov [rdi+8],rdx
	mul rax
	mov rcx,[rsi+16]
	mov [rdi+16],rax
	mov rax,rcx
	mov [rdi+24],rdx
	mul rax
	mov [rdi+32],rax
	mov [rdi+40],rdx
	mov rax,r11
	mul r10
	mov r8,rax
	mov rax,rcx
	mov r9,rdx
	mul r10
	xor r10,r10
	add r9,rax
	mov rax,r11
	mov r11,r10
	adc r10,rdx
	mul rcx
	add r10,rax
	adc rdx,r11
	add r8,r8
	adc r9,r9
	adc r10,r10
	adc rdx,rdx
	adc r11,r11
	add [rdi+8],r8
	adc [rdi+16],r9
	adc [rdi+24],r10
	adc [rdi+32],rdx
	adc [rdi+40],r11
win	pop rdi
win	pop rsi
	ret

lab_gt3:
lab_do_mul_2:
	mov r8,[rsi]
	push rbx
	lea rdi,[rdi+rdx*8]
	mov rax,[rsi+8]
	push rbp
	lea rsi,[rsi+rdx*8]
	mov r9,rax
	push r12
	mov r12,1
	push r13
	sub r12,rdx
	push r14
	push r12
	mul r8
	mov [rdi+r12*8],rax
	mov rax,[rsi+r12*8+8]
	test r12b,1
	jnz lab_m2b1

lab_m2b0:
	lea rcx,[r12+2]
	xor r11,r11
	xor rbx,rbx
	mov r10,rdx
	jmp lab_m2l0

lab_m2b1:
	lea rcx,[r12+1]
	xor rbp,rbp
	xor r10,r10
	mov rbx,rdx
	jmp lab_m2l1

	ALIGN 16
lab_m2tp:
lab_m2l0:
	mul r8
	add r10,rax
	mov rbp,rdx
	adc rbp,0
	mov rax,[rsi+rcx*8-8]
	mul r9
	add r10,r11
	adc rbp,0
	add rbx,rax
	mov [rdi+rcx*8-8],r10
	mov r10,rdx
	adc r10,0
	mov rax,[rsi+rcx*8]
lab_m2l1:
	mul r8
	add rbx,rax
	mov r11,rdx
	adc r11,0
	add rbx,rbp
	mov rax,[rsi+rcx*8]
	adc r11,0
	mul r9
	mov [rdi+rcx*8],rbx
	add r10,rax
	mov rbx,rdx
	mov rax,[rsi+rcx*8+8]
	adc rbx,0
	add rcx,2
	jnc lab_m2tp

lab_m2ed:
	mul r8
	add r10,rax
	mov rbp,rdx
	adc rbp,0
	mov rax,[rsi-8]
	mul r9
	add r10,r11
	adc rbp,0
	add rbx,rax
	mov [rdi-8],r10
	adc rdx,0
	add rbx,rbp
	mov [rdi],rbx
	adc rdx,0
	mov [rdi+8],rdx
	add r12,2
	lea rdi,[rdi+16]
lab_do_addmul_2:
	cmp r12,-1
	jge lab_corner
lab_outer:
	mov r8,[rsi+r12*8-8]
	mov rax,[rsi+r12*8]
	mov r9,rax
	mul r8
	test r12b,1
	jnz lab_a1x1

lab_a1x0:
	xor r10,r10
	mov r14,[rdi+r12*8+8]
	add [rdi+r12*8],rax
	mov r11,rdx
	adc r11,0
	xor rbx,rbx
	mov rax,[rsi+r12*8+8]
	test r12b,2
	jnz lab_a110

lab_a100:
	lea rcx,[r12+2]
	jmp lab_lo0

lab_a110:
	lea rcx,[r12]
	jmp lab_lo2

lab_a1x1:
	mov r14,[rdi+r12*8]
	xor rbx,rbx
	mov r13,[rdi+r12*8+8]
	add r14,rax
	mov rbp,rdx
	adc rbp,0
	xor r10,r10
	mov rax,[rsi+r12*8+8]
	test r12b,2
	jz lab_a111

lab_a101:
	lea rcx,[r12+3]
	jmp lab_lo1

lab_a111:
	lea rcx,[r12+1]
	jmp lab_lo3

	ALIGN 16
lab_top:
	mov r13,[rdi+rcx*8-16]
	mul r9
	mov r10,rdx
	add r13,rax
	adc r10,0
	add r14,r11
	adc rbp,0
	add r13,rbx
	adc r10,0
	mov rax,[rsi+rcx*8-16]
lab_lo1:
	mul r8
	add r13,rax
	mov r11,rdx
	adc r11,0
	mov rax,[rsi+rcx*8-16]
	mul r9
	mov [rdi+rcx*8-24],r14
	mov r14,[rdi+rcx*8-8]
	add r13,rbp
	adc r11,0
	mov rbx,rdx
	mov [rdi+rcx*8-16],r13
	add r14,rax
	adc rbx,0
	mov rax,[rsi+rcx*8-8]
	add r14,r10
	adc rbx,0
lab_lo0:
	mul r8
	add r14,rax
	mov rbp,rdx
	adc rbp,0
	mov rax,[rsi+rcx*8-8]
	mul r9
	add r14,r11
	mov r13,[rdi+rcx*8]
	adc rbp,0
	mov r10,rdx
	add r13,rax
	adc r10,0
	mov rax,[rsi+rcx*8]
lab_lo3:
	mul r8
	add r13,rbx
	mov [rdi+rcx*8-8],r14
	mov r11,rdx
	adc r10,0
	add r13,rax
	adc r11,0
	mov rax,[rsi+rcx*8]
	add r13,rbp
	adc r11,0
	mul r9
	mov r14,[rdi+rcx*8+8]
	add r14,rax
	mov rbx,rdx
	adc rbx,0
	mov rax,[rsi+rcx*8+8]
	mov [rdi+rcx*8],r13
lab_lo2:
	mul r8
	add r14,r10
	mov rbp,rdx
	adc rbx,0
	add r14,rax
	mov rax,[rsi+rcx*8+8]
	adc rbp,0
	add rcx,4
	jnc lab_top

lab_end:
	mul r9
	add r14,r11
	adc rbp,0
	add rax,rbx
	adc rdx,0
	mov [rdi-8],r14
	add rax,rbp
	adc rdx,0
	mov [rdi],rax
	mov [rdi+8],rdx
	add r12,2
	lea rdi,[rdi+16]
	cmp r12,-1
	jl lab_outer

lab_corner:
	jg lab_sqr_diag_addlsh1

lab_small_corner:
	mov r8,[rsi-16]
	mov rax,[rsi-8]
	lea rdi,[rdi+8]
	mul r8
	add [rdi-16],rax
	adc rdx,0
	mov [rdi-8],rdx

lab_sqr_diag_addlsh1:
	pop rcx
	mov rax,[rsi+rcx*8-8]
	shl rcx,1
	mul rax
	mov [rdi+rcx*8-8],rax
	xor rbx,rbx
	test cl,2
	jnz lab_dm1

	lea rcx,[rcx+2]
	jmp lab_dm0

	ALIGN 16
lab_dtop:
	add r8,r10
	adc r9,rax
	mov [rdi+rcx*8-32],r8
	mov [rdi+rcx*8-24],r9
lab_dm0:
	mov r8,[rdi+rcx*8-16]
	mov r9,[rdi+rcx*8-8]
	adc r8,r8
	adc r9,r9
	mov rax,[rsi+rcx*4-8]
	lea r10,[rdx+rbx]
	setc bl
	mul rax
	add r8,r10
	adc r9,rax
	mov [rdi+rcx*8-16],r8
	mov [rdi+rcx*8-8],r9
lab_dm1:
	mov r8,[rdi+rcx*8]
	mov r9,[rdi+rcx*8+8]
	adc r8,r8
	adc r9,r9
	mov rax,[rsi+rcx*4]
	lea r10,[rdx+rbx]
	setc bl
	mul rax
	add rcx,4
	jnc lab_dtop

lab_dend:
	add r8,r10
	adc r9,rax
	mov [rdi-16],r8
	mov [rdi-8],r9
	adc rdx,rbx
	mov [rdi],rdx
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
win	pop rdi
win	pop rsi
	ret
ilmp_sqr_basecase_ endp
end
