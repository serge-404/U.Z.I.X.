;[]------------------------------------------------------------[]
;|	C0.AS -- UZIX Start Up Code				|
;|	ORION/HTC/UZIX Run Time Library	version 1.0		|
;[]------------------------------------------------------------[]

	psect	text	       ;,class=CODE
;	psect	strings        ;,class=CODE
;	psect	const          ;,class=CODE
	psect	data           ;,class=DATA
	psect	bss            ;,class=DATA
	psect	bssend         ;,class=DATA

	psect 	text

	global	_main, _exit, __exit, __sys__, __sysint__, __retint__, __retlong__, __lngjmprv
	global	start

; At the start, SP points to user stack.
;	org	100h
;;; executable header layout
start:	defb	0C3h		; for ???
	defw	start0

;	org	103h
e_flags: defb	0		; bit 7: 1-not refresh system vectors on swapin()
e_text:  defw	etext
e_data:  defw	edata
e_bss:	 defw	ebss
e_heap:	 defw	0
e_stack: defw	0
e_env:	 defw	__argc
;;; total size of header == 16 bytes

;	org	110h
start0: 
	ld	(___stktop),sp

; Clear BSS
	ld	hl, (e_data)
	ex	de, hl
	ld	hl, (e_bss)
	or	a	; CLC
	sbc	hl, de
	ld	c, l
	ld	b, h
	dec	bc	; BC = counter-1
	ld	l, e
	ld	h, d	; HL = e_data
	inc	de	; DE = e_data+1
	ld	(hl), 0
	ldir		; clear bss - always >= 10 bytes

	pop	bc		; drop retaddr
; now there are the next stack structure:
;	+4 envp
;	+2 argv
; sp->	+0 argc
	ld	ix, 0
	add	ix, sp
	ld	l, (ix+4)
	ld	h, (ix+5)
	ld	(_environ),hl
	ld	l, (ix+2)
	ld	h, (ix+3)
	ld	(__argv),hl
	ld	l, (ix+0)
	ld	h, (ix+1)
	ld	(__argc),hl
start1:	
	call	_main
	pop	bc
	pop	bc
	pop	bc
	push	hl		; /* exit arg in HL */
	push	hl		; /* push fake func return address */
_exit:
	pop     hl		; /* drop retaddr, (SP)=exitcode */
	ld	hl, (___cleanup)
	ld	a, l
	or	h
	call	nz, indirect	; /* (*__cleanup)(exitcode, ???) */
	push	hl		; /* push fake func return address */
__exit:	ld	hl,11		; /* (SP+2)=exitcode */
;
__sysint__:
	ld	de,__retint__
__sys__:			; /* INP: HL=callno, DE=__retint__ || __retlong__ */ 
	ex	(sp), hl	; /* HL=func return address, (SP)=callno */
	push	de		; /* push unix() return address */
	ex	de,hl		; /* DE=func return address */
	ld	hl,(retsp)
	dec	hl
	dec	hl
	ld	(retsp), hl	; /* first decrease store pointer */
	ld	(hl),e		; /* next storing func return address */	
	inc	hl		; /* to avoid unix() reentrance within */
	ld	(hl),d		; /* calltrap (while context interrupted) */
	jp	8		; /* call unix() */
__retint__:
	ex de,hl
__retlong__:
	exx
	ld	hl,(retsp)
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	(retsp),hl
	ex	de,hl		; /* HL=func return address */
	ex	(sp), hl	; /* (SP)=func return address, HL=callno */
	exx
	ret nc
	ld (_errno),bc
	ld de,-1
	ld h,d
	ld l,e
	ret
;
retsp:	defw	038h
__lngjmprv: defw 1
;
indirect:
	jp	(hl)
;
;[]------------------------------------------------------------[]
;|	Start Up Data Area					|
;|								|
;|	WARNING 	Do not move any variables in the data	|
;|			segment unless you're absolutely sure	|
;|			that it does not matter.		|
;|								|
;[]------------------------------------------------------------[]

	psect	data
etext:

;	Memory management variables
	global	___heapbase, ___brklvl, ___heaptop, ___stktop

___heapbase:	defw	ebss
___brklvl:	defw	ebss
___heaptop:	defw	ebss
___stktop:	defw	0

	psect	bss
	global	__argc, __argv, _environ, _errno, ___cleanup
edata:	
__argc:		defs	2
__argv:		defs	2
_environ:	defs	2
_errno:		defs	2
___cleanup:	defs	2

	psect	bssend
ebss:

	end	; start


