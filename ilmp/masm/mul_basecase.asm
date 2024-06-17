include <asm_windows>

.code
	ALIGN 16
ilmp_mul_basecase_ proc
win	push rsi
win	push rdi
	push rbx
	push rbp

win	mov rdi,rcx
win	mov rsi,rdx
win	mov rdx,r8
win	mov rcx,r9
win	mov r8,[rsp+48h]

	mov rbx,rdx
	neg rbx

	mov rax,[rsi]
	lea rsi,[rsi+rdx*8]
	lea rdi,[rdi+rdx*8]

	mov r9,[rcx]
	mul r9 

	test r8b,1
	jz lab_do_mul_2

lab_do_mul_1:
	
	test bl,1
	jnz lab_m1x1

lab_m1x0:
	mov r10,rax
	mov r11,rdx
	mov rax,[rsi+rbx*8+8]
	test bl,2
	jnz lab_m110

lab_m100:
	lea rbp,[rbx+2]
	jmp lab_m1l0

lab_m110:
	lea rbp,[rbx]
	jmp lab_m1l2

lab_m1x1:
	mov r11,rax
	mov r10,rdx
	test bl,2
	jz lab_m111

lab_m101:
	lea rbp,[rbx+3]
	test rbp,rbp
	js lab_m1l1
	mov [rdi-8],rax
	mov [rdi],rdx
	pop rbp
	pop rbx

win	pop rdi
win	pop rsi
	ret

lab_m111:
	lea rbp,[rbx+1]
	mov rax,[rsi+rbx*8+8]
	jmp lab_m1l3

	ALIGN 16 
lab_m1tp:
	mov r10,rdx
	add r11,rax
lab_m1l1:
	mov rax,[rsi+rbp*8-16]
	adc r10,0
	mul r9
	add r10,rax
	mov [rdi+rbp*8-24],r11
	mov rax,[rsi+rbp*8-8]
	mov r11,rdx
	adc r11,0
lab_m1l0:
	mul r9
	mov [rdi+rbp*8-16],r10
	add r11,rax
	mov r10,rdx
	mov rax,[rsi+rbp*8]
	adc r10,0
lab_m1l3:
	mul r9
	mov [rdi+rbp*8-8],r11
	mov r11,rdx
	add r10,rax
	mov rax,[rsi+rbp*8+8]
	adc r11,0
lab_m1l2:
	mul r9
	mov [rdi+rbp*8],r10
	add rbp,4
	jnc lab_m1tp

lab_m1ed:
	add r11,rax
	adc rdx,0
	mov [rdi-8],r11
	mov [rdi],rdx

	dec r8
	jz lab_ret2

	lea rcx,[rcx+8]
	lea rdi,[rdi+8]
	push r12
	push r13
	push r14
	jmp lab_do_addmul

lab_do_mul_2:
	
	push r12
	push r13
	push r14

	mov r14,[rcx+8]

	test bl,1
	jnz lab_m2b1

lab_m2b0:
	lea rbp,[rbx]
	xor r10,r10
	mov r12,rax
	mov r11,rdx
	jmp lab_m2l0

lab_m2b1:
	lea rbp,[rbx+1]
	xor r11,r11
	xor r12,r12
	mov r10,rax
	mov r13,rdx
	jmp lab_m2l1

	ALIGN 16
lab_m2tp:
	mul r9
	add r10,rax
	mov r13,rdx
	adc r13,0
lab_m2l1:
	mov rax,[rsi+rbp*8-8]
	mul r14
	add r10,r11
	adc r13,0
	add r12,rax
	mov [rdi+rbp*8-8],r10
	mov r10,rdx
	adc r10,0
	mov rax,[rsi+rbp*8]
	mul r9
	add r12,rax
	mov r11,rdx
	adc r11,0
	add r12,r13
lab_m2l0:
	mov rax,[rsi+rbp*8]
	adc r11,0
	mul r14
	mov [rdi+rbp*8],r12
	add r10,rax
	mov r12,rdx
	mov rax,[rsi+rbp*8+8]
	adc r12,0
	add rbp,2
	jnc lab_m2tp

lab_m2ed:
	mul r9
	add r10,rax
	mov r13,rdx
	adc r13,0
	mov rax,[rsi-8]
	mul r14
	add r10,r11
	adc r13,0
	add r12,rax
	mov [rdi-8],r10
	adc rdx,0
	add r12,r13
	mov [rdi],r12
	adc rdx,0
	mov [rdi+8],rdx

	add r8,-2
	jz lab_ret5
	lea rcx,[rcx+16]
	lea rdi,[rdi+16]


lab_do_addmul:
	
	push r15
	push r8 
	
lab_outer:
	
	mov r9,[rcx]
	mov r8,[rcx+8]
	mov rax,[rsi+rbx*8]
	mul r9
	test bl,1
	jnz lab_a1x1

lab_a1x0:
	mov r14,[rdi+rbx*8]
	xor r10,r10
	mov r11,rdx
	test bl,2
	jnz lab_a110

lab_a100:
	lea rbp,[rbx+2]
	add r14,rax
	adc r11,0
	mov rax,[rsi+rbx*8]
	mul r8
	mov r15,[rdi+rbx*8+8]
	jmp lab_lo0

lab_a110:
	lea rbp,[rbx]
	xor r13,r13
	jmp lab_lo2

lab_a1x1:
	mov r15,[rdi+rbx*8]
	xor r12,r12
	xor r11,r11
	test bl,2
	jz lab_a111

lab_a101:
	lea rbp,[rbx+3]
	mov r13,rdx
	add r15,rax
	mov rax,[rsi+rbx*8]
	adc r13,0
	jmp lab_top

lab_a111:
	lea rbp,[rbx+1]
	jmp lab_lo3

	ALIGN 16
lab_top:
	mov r14,[rdi+rbp*8-16]
	mul r8
	mov r10,rdx
	add r14,rax
	adc r10,0
	add r15,r11
	adc r13,0
	add r14,r12
	adc r10,0
	mov rax,[rsi+rbp*8-16]
	mul r9
	add r14,rax
	mov r11,rdx
	adc r11,0
	mov rax,[rsi+rbp*8-16]
	mul r8
	mov [rdi+rbp*8-24],r15
	mov r15,[rdi+rbp*8-8]
	add r14,r13
	adc r11,0
lab_lo0:
	mov r12,rdx
	mov [rdi+rbp*8-16],r14
	add r15,rax
	adc r12,0
	mov rax,[rsi+rbp*8-8]
	add r15,r10
	adc r12,0
	mul r9
lab_lo3:
	add r15,rax
	mov r13,rdx
	adc r13,0
	mov rax,[rsi+rbp*8-8]
	mul r8
	add r15,r11
	mov r14,[rdi+rbp*8]
	adc r13,0
	mov r10,rdx
	add r14,rax
	adc r10,0
	mov rax,[rsi+rbp*8]
	mul r9
	add r14,r12
	mov [rdi+rbp*8-8],r15
	mov r11,rdx
	adc r10,0
lab_lo2:
	add r14,rax
	adc r11,0
	mov rax,[rsi+rbp*8]
	add r14,r13
	adc r11,0
	mul r8
	mov r15,[rdi+rbp*8+8]
	add r15,rax
	mov r12,rdx
	adc r12,0
	mov rax,[rsi+rbp*8+8]
	mov [rdi+rbp*8],r14
	mul r9
	add r15,r10
	mov r13,rdx
	adc r12,0
	add r15,rax
	mov rax,[rsi+rbp*8+8]
	adc r13,0
	add rbp,4
	jnc lab_top

lab_end:
	mul r8
	add r15,r11
	adc r13,0
	add rax,r12
	adc rdx,0
	mov [rdi-8],r15
	add rax,r13
	adc rdx,0
	mov [rdi],rax
	mov [rdi+8],rdx

	add qword ptr [rsp],-2
	lea rcx,[rcx+16]
	lea rdi,[rdi+16]
	jnz lab_outer

	pop rax
	pop r15
lab_ret5:
	pop r14
	pop r13
	pop r12
lab_ret2:
	pop rbp
	pop rbx

win	pop rdi
win	pop rsi
	ret
ilmp_mul_basecase_ endp
end
