
.code
	ALIGN 16
ilmp_limb_bits_ proc
	mov rax,-1
	bsr rcx,rcx
	cmovnz rax,rcx
	inc rax
	ret
ilmp_limb_bits_ endp

	ALIGN 16
ilmp_leading_zeros_ proc
	mov rax,-1
	bsr rcx,rcx
	cmovnz rax,rcx
	not rax
	add rax,64
	ret
ilmp_leading_zeros_ endp

	ALIGN 16
ilmp_tailing_zeros_ proc
	mov rax,64
	bsf rcx,rcx
	cmovnz rax,rcx
	ret
ilmp_tailing_zeros_ endp

	ALIGN 16
ilmp_mulh_ proc
	mov rax,rdx
	mul rcx
	mov rax,rdx
	ret
ilmp_mulh_ endp

end
