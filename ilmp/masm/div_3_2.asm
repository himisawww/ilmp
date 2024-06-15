.code
	ALIGN 16
ilmp_div_3_2_ proc
	push r12
	push r13
	mov rax,r8
	mov r11,[rcx]
	mov r12,[rcx+8]
	mov r13,[rcx+16]
	mov r8,[rdx]
	mov r9,[rdx+8]
	
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
	mov rax,r10
	mov [rcx],r12
	mov [rcx+8],r13
	pop r13
	pop r12
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

ilmp_div_3_2_ endp
end
