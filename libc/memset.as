	psect	text
	global	_memset
; void *memset(void *str, int c, size_t l)
_memset:
	pop	af
	pop	hl	; HL=str
	pop	de	; E=c
	pop	bc	; BC=l
	push	bc
	push	de
	push	hl
	push	af	
	ld	a,b
	or	c	; l=0? so return
	ret	z
	ld	(hl),e
	dec	bc
	ld	a,b
	or	c	; l=1? so return
	ret	z
	ld	d,h
	ld	e,l
	inc	de	; DE=str+1
	push	hl
	ldir
	pop	hl
	ret
