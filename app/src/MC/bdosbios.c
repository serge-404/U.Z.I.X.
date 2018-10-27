#asm
	psect text
	global _bios 

_bios:						; /* uchar xbios(int funcN, int bc) */
	pop	af
	pop	hl					; /* funcN */
	pop	bc					; /* func parameter in BC */ 
	push bc
	push hl
	push af
	ld	d,h
	ld	e,l
	add	hl,de
	add	hl,de				; /*  funcN*3  */
	ld	de,(1)				; de=BIOS WARMSTART (FuncN=0)
	add hl,de				; /* HL = actual BIOS entry point */
	call xbioscall
	ld	l,a
	ld	h,0
	ret
xbioscall:
	jp	(hl)
;
#endasm

#asm
	psect text
	global _bios 

_bios:						; /* uchar xbios(int funcN, int bc) */
	pop	af
	pop	hl					; /* funcN */
	pop	bc					; /* func parameter in BC */ 
	push bc
	push hl
	push af
	ld	d,h
	ld	e,l
	add	hl,de
	add	hl,de				; /*  funcN*3  */
	ld	de,(1)				; de=BIOS WARMSTART (FuncN=0)
	dec	de
	dec	de
	dec	de				; de=BIOS COLDSTART (FuncN=0)
	add hl,de				; /* HL = actual BIOS entry point */
	call xbioscall
	ld	l,a
	ld	h,0
	ret
xbioscall:
	jp	(hl)
;
#endasm
