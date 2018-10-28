	psect	text
	global _bios 
;uchar  bios(int, ...);
_bios:						; /* uchar bios(int funcN, int bc) */
	pop	af
	pop	hl				; /* funcN */
	pop	bc				; /* func parameter in BC */ 
	push	bc
	push	hl
	push	af
	ld	d,h
	ld	e,l
	add	hl,de
	add	hl,de				; /*  funcN*3  */
	ex	de,hl
	ld	hl,(1)				; de=BIOS WARMSTART (FuncN=0)
	add 	hl,de				; /* HL = actual BIOS entry point */
	call 	xbioscall
	ld	l,a
	ld	h,0
	ret
xbioscall:
	jp	(hl)
;
