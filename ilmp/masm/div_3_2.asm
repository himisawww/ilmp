include <asm_windows>
.code
	ALIGN 16
ilmp_div_3_2_ proc
	mov rax,rx2
win	push rs12
win	push rc13
	mov r8,[rx1]
	mov r9,[rx1+8]
	mov r11,[rx0]
	mov rs12,[rx0+8]
	mov rc13,[rx0+16]
	
	mul rc13			;rdx:rax=a2*(inv-2^64)
	mov r10,rs12
	add r10,rax
	adc rdx,rc13		;rdx:r10=a2*inv+a1=qh:ql
	mov rc13,r10		;rc13=ql
	mov r10,rdx		;r10=qh
	imul rdx,r9		;rdx=qh*bh m64
	xchg rc13,rs12	;rs12=ql,rc13=a1
	mov rax,r8		;rax=bl
	sub rc13,rdx		;rc13=ah-qh*bh m64
	xchg rs12,r11	;r11=ql,rs12=a0
	mul r10			;rdx:rax=bl*qh
	sub rs12,r8
	sbb rc13,r9		;rc13:rs12=a-qh*bh*2^64-b m128
	sub rs12,rax
	sbb rc13,rdx		;rc13:rs12=a-(qh+1)*b m128=z m128
	xor rax,rax
	xor rdx,rdx
	cmp rc13,r11		;if(z m128<ql,d=0,-1)
	cmovnc rax,r8	;
	cmovnc rdx,r9	;
	adc r10,0		;	++qh,rc13:rs12=a-qh*b
	add rs12,rax		;else(d=0,1)
	adc rc13,rdx		;	rc13:rs12+=b,rc13:rs12=a-qh*b
	cmp rc13,r9		;if(rc13:rs12>=b)
	jae lab_fx		;	rc13:rs12-=b,++qh
lab_ok:
	mov rax,r10
	mov [rx0],rs12
	mov [rx0+8],rc13
win	pop rc13
win	pop rs12
	ret
	
lab_fx:
	seta dl
	cmp rs12,r8
	setae al
	or al,dl
	jz lab_ok
	inc r10
	sub rs12,r8
	sbb rc13,r9
	jmp lab_ok

ilmp_div_3_2_ endp
end
