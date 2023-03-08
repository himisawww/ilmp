EXTERN ilmp_inv_1_:near

.code

func_2 TEXTEQU <ilmp_div_nq_2_>
ifq TEXTEQU <;>
include <div_2>

func_2 TEXTEQU <ilmp_div_2_>
ifq TEXTEQU <>
include <div_2>

	ALIGN 16
ilmp_div_2_s_ proc
	push rdi
	push rsi
	push rbp
	push rbx
	push r12
	push r13

	lea rbx,[r8-1]
	mov rdi,rcx
	mov rsi,rdx
	mov r12,[r9]
	mov r13,[r9+8]
	mov rcx,r13
	call ilmp_inv_1_
	mov r9,r13
	mov r8,r12
	mov rbp,rax
	imul r13,rax	;r13=bh*(inv-2^64) (mod 2^64) =bh*inv m64
	xor rcx,rcx
	mul r8			;rdx:rax=bl*(inv-2^64)
	add r13,r8		;r13=bh*inv+bl m64
	adc rcx,-1		;rcx:r13=bh*inv+bl m128
	add r13,rdx
	adc rcx,0		;rcx:r13:rax=b*inv m192
	js lab_2f
lab_1b:
	dec rbp
	sub rax,r8
	sbb r13,r9
	sbb rcx,0
	jns lab_1b
lab_2f:
					;rbp=(2^192-1) div b - 2^64
	xor rcx,rcx
	mov rax,[rsi+rbx*8-8]
	mov rdx,[rsi+rbx*8]
	mov r12,rax
	mov r13,rdx
	sub r12,r8
	sbb r13,r9
	setnc cl
	cmovc r12,rax
	cmovc r13,rdx
	dec rbx

	ALIGN 16
lab_top:			;r13:r12=a2:a1=ah<b=bh:bl=r9:r8
	mov r11,[rsi+rbx*8-8]
	mov rax,rbp		;rax=inv-2^64
	mul r13			;rdx:rax=a2*(inv-2^64)
	mov r10,r12
	add r10,rax
	adc rdx,r13		;rdx:r10=a2*inv+a1=qh:ql
	mov r13,r10		;r13=ql
	mov r10,rdx		;r10=qh
	imul rdx,r9		;rdx=qh*bh m64
	xchg r13,r12	;r12=ql,r13=a1
	mov rax,r8		;rax=bl
	sub r13,rdx		;r13=ah-qh*bh m64
	xchg r12,r11	;r11=ql,r12=a0
	mul r10			;rdx:rax=bl*qh
	sub r12,r8
	sbb r13,r9		;r13:r12=a-qh*bh*2^64-b m128
	sub r12,rax
	sbb r13,rdx		;r13:r12=a-(qh+1)*b m128=z m128
	xor rax,rax
	xor rdx,rdx
	cmp r13,r11		;if(z m128<ql,d=0,-1)
	cmovnc rax,r8	;
	cmovnc rdx,r9	;
	adc r10,0		;	++qh,r13:r12=a-qh*b
	add r12,rax		;else(d=0,1)
	adc r13,rdx		;	r13:r12+=b,r13:r12=a-qh*b
	cmp r13,r9		;if(r13:r12>=b)
	jae lab_fx		;	r13:r12-=b,++qh
lab_ok:
	mov [rdi+rbx*8-8],r10
	dec rbx
	jnz lab_top

	mov [rsi],r12
	mov [rsi+8],r13
	mov rax,rcx

	pop r13
	pop r12
	pop rbx
	pop rbp
	pop rsi
	pop rdi
	ret

lab_fx:
	seta dl
	cmp r12,r8
	setae al
	or al,dl
	jz lab_ok
	inc r10
	sub r12,r8
	sbb r13,r9
	jmp lab_ok
	
ilmp_div_2_s_ endp

end
