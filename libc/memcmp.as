	psect	text
	global	_memcmp
	; int memcmp(void *s, void *d, size_t l)
        ; HL=s, BC=d, DE=l
_memcmp:
	pop	af
	pop	hl
	pop	bc
	pop	de
	push	de
	push	bc
	push	hl
	push	af
        push    iy
        push    bc
        pop     iy      ; IY=d
        ld      bc,0    ; char1, char2
1:      ld      a,(hl)
        ld      b,a
        ld      a,(iy)  ; char1 != char 2 ?
        ld      c,a
        cp      b
        jr      nz,2f
        inc     hl	; s++
        inc     iy	; d++
        dec     de	; l--
        ld      a,d
        or      e
        jp      nz,1b	; l != 0, continue
2:      ld      a,c	; char1 - char2
        ld      e,a
	rla
	sbc	a,a
	ld	d,a
        ld      a,b
        ld      l,b
	rla
	sbc	a,a
	ld	h,a
	or	a
	sbc	hl,de
	pop	iy
	ret

