include <asm_windows>
EXTERN ilmp_inv_1_:near

.code

func_1 TEXTEQU <ilmp_div_nq_1_>
ifq TEXTEQU <;>
include <div_1>

func_1 TEXTEQU <ilmp_div_1_>
ifq TEXTEQU <>
include <div_1>

	ALIGN 16
ilmp_div_1_s_ proc
	push rbp
	push rbx
win	push rdi
win	push rsi

	mov rbx,rx2
win	mov rdi,rcx
win	mov rsi,rdx
lin	mov r10,rdi
	mov rbp,rx3
	mov rx0,rx3
	call ilmp_inv_1_
lin	mov rdi,r10
	mov r8,rbp
	mov r9,rax

	xor rcx,rcx
	mov r10,[rsi+rbx*8-8]
	mov rax,r10
	sub rax,r8
	setnc cl
	cmovc rax,r10
	dec rbx
	
	ALIGN 16
lab_top:				;r10=xl,rax=xh<r8
	lea rbp,[rax+1]
	mov r10,[rsi+rbx*8-8]
	mul r9				;rdx:rax=xh*(inv-2^64)
	add rax,r10			;=xh*(inv-2^64)+xl
	adc rdx,rbp			;=xh*inv+xl+2^64 (mod 2^128)=q m128
	mov rbp,rax			;rbp=rax=ql
	mov r11,rdx			;r11=rdx=qh m64
	imul rdx,r8			;rdx=qh*r8 m64
	sub r10,rdx			;r10=x-qh*r8 m64=z m64
	lea rax,[r8+r10]	;rax=z+r8 m64
	cmp r10,rbp			;c=z m64<ql
	cmovc rax,r10		;if(c,d={0,-1})rax=z m64=z
	adc r11,-1			;if(nc,d={1,0})qh-=1,(rax=z+r8 m64=z')
	cmp rax,r8			;if(z>=r8)
	jae lab_fx			;qh+=1,rax=z'
lab_ok:
	mov [rdi+rbx*8-8],r11
	dec rbx
	jnz lab_top

	mov [rsi],rax
	mov rax,rcx

win	pop rsi
win	pop rdi
	pop rbx
	pop rbp
	ret

lab_fx:
	sub rax,r8
	inc r11
	jmp lab_ok

ilmp_div_1_s_ endp

end
