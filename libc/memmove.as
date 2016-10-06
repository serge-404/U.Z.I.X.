	psect	text
	global	_memmove
; void *memmove(void *d, void *s, size_t l)
_memmove:
	pop	af
	pop	hl	; d
	pop	de	; s
	pop	bc	; l
	push	bc
	push	de
	push	hl
	push	af
	ld	a,b
	or	c
	ret	z
	push	hl	; saves d for return
	push	de	; saves s
	push	hl	; saves d
	or	a
	sbc	hl,de	; d = d - s
	or	a
	sbc	hl,bc	; d = d -l
	jr	c, 2f
	; d-s >=l -> memcpy
	pop	de	; d
	pop	hl	; s
	ldir
	jr	1f
2:	; s=s+l, d=d+l, lddr
	xor	a
	pop	hl	; HL=d
	dec	hl
	add	hl,bc	; d=d+l
	pop	de	; DE=s
	ex	de,hl	; HL=s, DE=d+l
	dec	hl
	add	hl,bc	; HL=s+l  BC=l
	lddr
1:	pop	hl	; d
	ret
