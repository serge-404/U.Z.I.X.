	psect	text
	global	_memcpy
;void *memcpy(void *d, void *s, size_t l)
_memcpy:
	pop	af
	pop	de
	pop	hl
	pop	bc
	push	bc
	push	hl
	push	de
	push	af
	push	de
	ld	a,b
	or	c
	jr	z,1f
	ldir
1:	pop	hl
	ret
