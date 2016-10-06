	psect	text
	global _strlen
; size_t strlen(char *str) ;
_strlen:
	pop	af
	pop	de	; DE = str
	push	de
	push	af
	ld	h,d
	ld	l,e
	ld	bc,0ffffh
	xor	a
	cpir
	sbc	hl,de
	dec	hl
	ret

