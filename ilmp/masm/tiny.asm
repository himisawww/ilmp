include <asm_windows>

.code
	ALIGN 16
ilmp_limb_bits_ proc
	mov rax,-1
	bsr rx0,rx0
	cmovnz rax,rx0
	inc rax
	ret
ilmp_limb_bits_ endp

	ALIGN 16
ilmp_leading_zeros_ proc
	mov rax,-1
	bsr rx0,rx0
	cmovnz rax,rx0
	not rax
	add rax,64
	ret
ilmp_leading_zeros_ endp

	ALIGN 16
ilmp_tailing_zeros_ proc
	mov rax,64
	bsf rx0,rx0
	cmovnz rax,rx0
	ret
ilmp_tailing_zeros_ endp

	ALIGN 16
ilmp_mulh_ proc
	mov rax,rx1
	mul rx0
	mov rax,rdx
	ret
ilmp_mulh_ endp

end
