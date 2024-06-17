include <asm_windows>

.code
	ALIGN 16
ilmp_mul_1_ proc
win	push rsi
win	push rdi

win	mov rdi,rcx
win	mov rsi,rdx
win	mov rdx,r8
lin mov r8,rx2
	neg r8

	mov rax,[rsi]
	lea rsi,[rsi+rdx*8]
	lea rdi,[rdi+rdx*8]

	mul rx3
	
	test r8b,1
	jnz lab_m1x1

lab_m1x0:
	mov r10,rax
	mov r11,rdx
	mov rax,[rsi+r8*8+8]
	test r8b,2
	jnz lab_m1l2

	lea r8,[r8+2]
	jmp lab_m1l0
	
lab_m1x1:
	mov r11,rax
	mov r10,rdx
	test r8b,2
	jz lab_m111

lab_m101:
	lea r8,[r8+3]
	test r8,r8
	js lab_m1l1
	mov [rdi-8],rax
	mov rax,rdx
win	pop rdi
win	pop rsi
	ret

lab_m111:
	lea r8,[r8+1]
	mov rax,[rsi+r8*8]
	jmp lab_m1l3

	ALIGN 16 
lab_m1tp:
	mov r10,rdx
	add r11,rax
lab_m1l1:
	mov rax,[rsi+r8*8-16]
	adc r10,0
	mul rx3
	add r10,rax
	mov [rdi+r8*8-24],r11
	mov rax,[rsi+r8*8-8]
	mov r11,rdx
	adc r11,0
lab_m1l0:
	mul rx3
	mov [rdi+r8*8-16],r10
	add r11,rax
	mov r10,rdx
	mov rax,[rsi+r8*8]
	adc r10,0
lab_m1l3:
	mul rx3
	mov [rdi+r8*8-8],r11
	mov r11,rdx
	add r10,rax
	mov rax,[rsi+r8*8+8]
	adc r11,0
lab_m1l2:
	mul rx3
	mov [rdi+r8*8],r10
	add r8,4
	jnc lab_m1tp

	add r11,rax
	adc rdx,0
	mov [rdi-8],r11
	mov rax,rdx

win	pop rdi
win	pop rsi
	ret
ilmp_mul_1_ endp
end
