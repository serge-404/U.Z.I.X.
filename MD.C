/*
 * UZIX - UNIX Implementation for MSX
 * (c) 1997-2001 Arcady Schekochikhin
 *		 Adriano C. R. da Cunha
 *
 * UZIX is based on UZI (UNIX Zilog Implementation)
 * UZI is a UNIX kernel clone written for Z-80 systems.
 * All code is public domain, not being based on any AT&T code.
 *
 * The author, Douglas Braun, can be reached at:
 *	7696 West Zayante Rd.
 *	Felton, CA 95018
 *	oliveb!intelca!mipos3!cadev4!dbraun
 *
 * This program is under GNU GPL, read COPYING for details
 *
 */

/**********************************************************
 Additional machine dependent subroutines for kernel
**********************************************************/

#define NEED__DEVIO
#define NEED__MACHDEP
#define NEED__DISPATCH

#include "uzix.h"
#ifdef SEPH
#include "types.h"
#include "signal.h"
#include "errno.h"
#endif
#include "unix.h"

#ifdef ORION_HOSTED
/*#include "idebdos.h"*/
#define BBASE	176   
#define BGETDT  BBASE+8	/* get date       (inp:nothing; out:HL=MSDOS FAT date) */
#define BGETTM  BBASE+9	/* get time       (inp:nothing; out:HL=MSDOS FAT time) */
#define BSETDT  BBASE+10 /* set date      (inp:DE=MSDOS date;  out:nothing)	*/
#define BSETTM  BBASE+11 /* set time      (inp:DE=MSDOS time;  out:nothing)	*/
extern short bdoshl(int, ...);	/* bdos call returning value in hl */
#endif /* ORION_HOSTED */

#undef NEED__DEVTTY
#undef NEED__DEVMISC
#undef NEED__FILESYS
#undef NEED__SCALL
#undef NEED__DEVFLOP
#undef NEED__DEVSWAP
#undef NEED__PROCESS
#include "extern.h"

#ifdef __KERNEL__
#ifdef ORI_UZIX
void unix __P((void));
#else
long unix __P((uint callno, ... /*uint arg1, uint arg2, uint arg3, uint arg4*/));
#endif
LCL long unixret __P((void));
#endif /* __KERNEL__ */

extern unsigned char TotalDrives;
#define _FCALLTRAP	0f100h

#ifdef PC_HOSTED
#include "machdep.mtc"
#include <stdarg.h>
#else
#ifdef MSX_HOSTED
#include "machdep.msx"
#else
/* #include <stdarg.h> */
#ifdef __KERNEL__
#include "machdep2.orn"	
#endif /* __KERNEL__ */
#include "machdep.orn"		/* ORION_HOSTED */
#endif
#endif

/*  Ф. 110 (6Eh)
  Получить информацию о памяти
  Выход: H=размер всего исправного ОЗУ в 4k сегментах
         L=размер свободного ОЗУ в 4k сегментах
*/

/* Architecture specific init.
 * This is called at the very beginning to initialize everything.
 * It is the equivalent of main()
 */
GBL void arch_init(VOID) {
#ifdef __KERNEL__
#asm
	di
#endasm
/*	bufinit(); */ 					/* do all buffers free without flushing */
/*	d_init(); */ 					/* see uzix.c */
	initsys();						/* Initialize system dependent parts */
	total=bdoshl(110)/0x400 - 11;	/* total memory size in 16k pages, except CP/M pages 0..2 */
	TICKSPERMIN = TICKSPERSEC * 60;
	MAXTICKS = TICKSPERSEC / 10;	/* 100 ms timeslice */
/*	_ei(); */
	ARCH_INIT;	/* MACRO */
#asm
	ld	sp,_TRP_STACK
#endasm
	init(); 	/* NORETURN */  
#else /* __KERNEL__ */
	inint = 0;
	UDATA(u_euid) = 0;
	UDATA(u_insys) = 1;
	UDATA(u_mask) = 022;
#endif /* __KERNEL__ */
}

#ifdef __KERNEL__
/*  __itoa(), itoa(), kprintf() migrated to devtty.orn */

#ifdef ORI_DEBUG
extern char *disp_names[];
#endif
extern ptptr nextready(VOID);
extern ptptr ptab_alloc(VOID);
extern int swap_alloc(page_t *mm);
extern ptptr _pp;					/* Temp storage for swapin/swapout/dofork */
STATIC int _newid;					/* Temp storage for dofork */


/* dofork() implements forking.
 * This function can have no arguments
 */
GBL int dofork(VOID) {
#ifndef LOC_UDATA
	/* Auto variables must conform to swapin/swapout autos ! */
	auto udata_t ud;
#endif

	if (NULL == (_pp = ptab_alloc())) {	/* Can't allocate ptab entry */
		goto Err;
	}
#ifdef ORI_UZIX
	if (swap_alloc(&(_pp->p_swap))) {		
#else
	if (swap_alloc(_pp->p_swap)) {		/* Can't allocate swap area */
#endif
		_pp->p_status = P_EMPTY;	/* Free ptab entry */
Err:		UDATA(u_error) = EAGAIN;
		return (-1);
	}
	++ptotal;				/* add process */
/*	_di(); */
#asm
	di
#endasm
	UDATA(u_ptab)->p_status = P_READY;	/* Parent is READY */
	_newid = _pp->p_pid;			/* Child ident */
#ifndef ORI_UZIX
#ifndef LOC_UDATA
	bcopy(UDATA_ADDR,&ud,sizeof(udata_t));	/* Put user data onto stack */
#endif
	/* Save the stack pointer and critical registers.
	 * When the process is swapped back in, it will be as
	 * if it returns with the value of the childs pid.
	 */
	/* push _newid
	 * push si, di, bp
	 * _stkptr = (void *)_SP
	 */
#define __SAVENEWID	dofork_SAVENEWID
#ifdef PC_HOSTED
#include "process.mtc"
#else
#ifdef MSX_HOSTED
#include "process.msx"
#endif
#endif
	UDATA(u_ptab)->p_sp = _stkptr;	/* save parent stack pointer */
#endif /* ORI_UZIX*/
#ifdef MEMBANKING
	swap_dup(UDATA(u_ptab)->p_swap, _pp->p_swap);
#else
	if (*(uint *)(UDATA(u_ptab)->p_swap) != 0) /* not in-core process */
		swap_write(UDATA(u_ptab)->p_swap);  /* swap out parent image */
#endif
#ifdef ORI_UZIX
#asm
global	_fstack
global	_ydofork
_ydofork:
	di
	call _fstack			; /* A = page_t	p_swap  - current process 64k page index: 3..15, hl=PRC_STACK, bc=_UDATAADDR+U_INSYS */
	push	hl
	inc hl
	inc hl
	ld	(_RET_SP),hl
	ld	(_RET_PAGE),a		; /* A = page_t	p_swap  - current process 64k page index: 3..15 */
	ld	h,b
	ld	l,c					; /* hl=_UDATAADDR+U_INSYS */
	dec	(hl)				; /* --insys */
	ld	hl,_RET_ADDR
	exx						; /* dst: HL' */
	pop	hl					; /* src: HL=(SP) */
	ld	bc,2
	call _R_LDIRTO2			; /* copying within process space, INP: hl=src de=dst bc=count */
	xor	a
	ld	(_SWITCH_PAGE),a	; /* do not context switch in _fork_content */
	ld	a,(_RET_PAGE)
	ld	hl,(__newid)		; /* return value for parent`s dofork() = child id */
	ld	(_UDATAADDR+U_RETVAL),hl 
	ex	de,hl
	ld	hl,xforkcontext	
	call BCALL				; /* parent content saved - later will continue at exit of dofork() with HL=DE=(_newid) */
	ld	hl,_UDATAADDR+U_INSYS
	inc	(hl)				; /* ++insys */
#endasm
#else
	/* repair child stack pointer */
	/* add sp, 2*4 */
#define __DROPSTACK	dofork_DROPSTACK
#ifdef PC_HOSTED
#include "process.mtc"
#else
#ifdef MSX_HOSTED
#include "process.msx"
#endif
#endif
#endif /* ORI_UZIX */
	/* Make a new (child) process table entry, etc. */
	newproc(_pp);			/* fills UDATA with new current process pointer - UDATA(u_ptab), i.e. p->p_swap - return page */
	UDATA(u_ptab)->p_status = P_RUNNING;
/*	_ei(); */
#asm
	ld a,(_inint)
	or a
	jr nz,1f
	ei			; /* if (!inint) ei; */
1:	nop
#endasm
	return 0;	/* Return to child */
}


/* Swapin()
 */
LCL void swapin(void) {
#ifndef ORI_UZIX
#ifndef LOC_UDATA
	/* Auto variables must conform to swapout/dofork autos ! */
	auto udata_t ud;
#endif
	tempstack();	/* CAN'T ACCESS STACK NOW !!! */
	swap_read(_pp->p_swap);
	/* STACK OF NEW PROCESS NOW AVAILABLE */
	/* Now we must restore stack frame */
	_stkptr = _pp->p_sp;
	/* _SP = (uint)_stkptr
	 * pop bp, di, si
	 */
#define __RESTFRAME	swapin_RESTFRAME
#ifdef PC_HOSTED
#include "process.mtc"
#else
#ifdef MSX_HOSTED
#include "process.msx"
#endif
#endif
#ifndef LOC_UDATA
	/* At this point stack reloaded to swapout state and
	   ud available for usage
	 */
	bcopy(&ud,UDATA_ADDR,sizeof(udata_t));
#endif
	if (_pp != UDATA(u_ptab))
		panic("mangled swapin");
#endif 								/* ORI_UZIX */ 
#asm
	di 
#endasm
	_pp->p_status = P_RUNNING;		/* _pp initialized before swapin() call */
	runticks = 0;
#ifndef ORI_UZIX
	_ei();
	if (REFRESH_VECTORS())
		setvectors();
#endif
	inint = 0;	/* is this right, inint=0 for swapped in process? */
	/* Restore to pre-swapout state */
	/* pop ax
	 * mov sp,bp
	 * pop bp
	 * ret
	 */
#ifdef ORI_UZIX
#asm
global __pp
	ld	hl,(__pp)				; /* HL = ptab_t *u_ptab  */
	ld	de,PSWAP_OFFSET
	add	hl,de					; /* 20150116 */
	ld	a,(hl)					; /* A = page_t	p_swap  - next process 64k page index: 3..15 */
	ld	(_SWITCH_PAGE),a		; /* switch_page = _pp->p_swap; */
	ld	hl,xdoswapin
	jp	BJMP					; /* _do_swapin switching to (switch_page) context and does EI at ret */
#endasm
#endif	/* ORI_UZIX */
#define __RETFROM	swapin_RETFROM
#ifdef PC_HOSTED
#include "process.mtc"
#else
#ifdef MSX_HOSTED
#include "process.msx"
#endif
#endif
}

/* unix() is the system calls dispatcher */
#ifdef ORI_UZIX
GBL void unix(VOID)
#else
GBL long unix(uint callno, ... /* arg1, arg2, arg3, arg4*/)
/*	register uchar callno; */
/*	uint arg1, arg2, arg3, arg4; */
#endif
{
#ifndef ORI_UZIX
	uint *ap;
	
	_di();
	/* we must DI because interrupt can occur before we get
	   user arguments. if a signal is handled, user maybe call a
	   system call and corrupts the arguments */
	ap = (uint *)(((char *)&callno) + sizeof(callno));
#if DEBUG > 1
	if ((uint)UDATA(u_break)+256 > (uint)ap)
		panic("PID #%d: stack to deep (%x/%x)\n",
			UDATA(u_ptab)->p_pid, ap, UDATA(u_break));
#endif
	if ((uchar)callno > dtsize) {
		UDATA(u_error) = EINVFNC;
		/* we know we're not called during interrupt, so we can force EI */
		__ei();
		return -1;
	}
	UDATA(u_argn1) = ap[0]; /*arg1*/
	UDATA(u_argn2) = ap[1]; /*arg2*/
	UDATA(u_argn3) = ap[2]; /*arg3*/
	UDATA(u_argn4) = ap[3]; /*arg4*/
	UDATA(u_callno) = (uchar)callno;
	UDATA(u_insys)++;
	UDATA(u_error) = 0;

#endif						/* ORI_UZIX */

	/* now we're safe. calltrap() will not occur with insys > 0 */
	/* we know we're not called during interrupt, so we can force EI */
#asm
	ei
#endasm
	tty_poll();
#if DEBUG > 0
	clk_int();
#endif
#ifdef ORI_DEBUG
/*	if (traceon || UDATA(u_traceme)) */
		kprintf("\t%4d : %s(%p, %p, %p, %p) = ",
			UDATA(u_ptab)->p_pid, disp_names[UDATA(u_callno)],
			UDATA(u_argn1), UDATA(u_argn2), UDATA(u_argn3), UDATA(u_argn4));
#endif */
	/* Branch to correct routine */
	UDATA(u_retval) = (*disp_tab[UDATA(u_callno)])();
#asm
	ei
#endasm

#ifdef ORI_DEBUG
/*	if (traceon || UDATA(u_traceme)) */
		kprintf("%p (%d)\n", UDATA(u_retval), UDATA(u_error));
#endif
	chksigs();
#asm
global _unix2
_unix2:
	di
#endasm

#ifdef ORI_DEBUG									/* because HUNG inside this branch - TODO !!! */
	if (runticks >= UDATA(u_ptab)->p_cprio) {
		UDATA(u_ptab)->p_status = P_READY;
#ifdef ORI_UZIX
		ret_page=KERNEL_PAGE;
		switch_page=0;
#endif
		swapout();		/* backup stack, SP and udata into process space */
	}
#endif
	
/*	_ei(); */
#asm
	ld a,(_inint)
	or a
	jr nz,1f
	ei			; /* if (!inint) ei; */
1:	nop
#endasm
	if (!(inint) && (UDATA(u_insys) == 1)) 
		calltrap();	/* Call trap routine if necessary, does DI if called */
#ifdef ORI_UZIX
#asm
global _unixret
global _fstack
_unixret:
	di
	call _fstack					; /* A=retpage, BC=_UDATAADDR+U_INSYS, HL=(_PRC_STACK1)|(_PRC_STACK2)*/ 
	ld	sp,hl
	ld	h,b
	ld	l,c							; /*--UDATA(u_insys)*/
	dec	(hl)
	jp 	_FCALLTRAP+20				; /* _zcalltrap: out (PAGE_PORT),a ; RET */
#endasm
#else
	--UDATA(u_insys);
	return unixret();
#endif
}

/* Calltrap() deal with a pending caught signal, if any.
 * udata.u_insys should be false.
 * Remember: the user may never return from the trap routine!
 */
GBL void calltrap(void) {
	void (*curvec) __P((signal_t)); /* for HTC call throught regs */
	register signal_t cursig = UDATA(u_cursig);
	long oldretval;
	uchar olderrno;

	if (cursig != __NOTASIGNAL && cursig <= NSIGS) {
		curvec = UDATA(u_sigvec)[cursig-1];
		UDATA(u_cursig) = __NOTASIGNAL;
		UDATA(u_sigvec)[cursig-1] = SIG_DFL; /* Reset to default */
		/* calltrap was called by unix() or service(), so it
		   can't erase previous contents of retval/errno! */
		oldretval = *(long *)&(UDATA(u_retval));
		olderrno = UDATA(u_error);
#ifdef ORI_UZIX
		if (inint == 1 && !UDATA(u_insys)) {
#asm
			di
			ld	hl,_PRC_STACK1		; /* we are in int, but u_insys==0, so _PRC_STACK1 is a free temporary storage */
			exx						; /* HL'=dst */  
			ld	hl,_BNKMARKER-2		; /* HL=src    _BNKMARKER-2 = where INTSP+6 stored by DOS ISR handler */
			ld	bc,2				; /* BC=count */
			call _R_LDIRTO2			; /* RET: HL=dst */
			ld	hl,(_PRC_STACK1)
; /*			dec	hl */
			dec	h					; /* dec hl    because "call BCALL0" stores retaddr on stack */
			ld	(_PRC_STACK1),hl
#endasm		
		}
#asm
		di
		ld	hl,-6
		add	hl,sp
		ld	(_TRP_STACK),hl		; /* -6 = reserve for "push; push; call _FCALLTRAP+1 */
		ld	hl,(_UDATAADDR)		; /* HL = ptab_t *u_ptab  */
		ld	bc,PSWAP_OFFSET
		add	hl,bc				; /* 20150116 */
		ld a,(_inint)
		or a					; /* Z=do_ei */	
		ld	a,(hl)				; /* A = page_t	p_swap  - process 64k page index: 3..N */
		ld	b,0
		ld	c,(ix-3)			; /* BC=cursig */
		ld	h,(ix-2)
		ld	l,(ix-1)			; /* HL=curvec */
		push ix
		push iy
		call _FCALLTRAP+1		; /* ei, (*curvec)(cursig) */	
		pop	iy
		pop	ix 
		di
#endasm
#else
		_ei();
		(*curvec)(cursig);
		_di();
#endif
		*(long *)&(UDATA(u_retval)) = oldretval;
		UDATA(u_error) = olderrno;
	}
}

#endif /* __KERNEL__ */

#ifdef NO_RTC
/* Update software emulated clock */
void updttod() {
	uchar t1, t2, t3, d1, d2;
	uint d3, i;
	
	t1 = ++tod_sec;
	t2 = ((tod.t_time >> 5) & 63);
	t3 = ((tod.t_time >> 11) & 31);
	d1 = (tod.t_date & 31);
	d2 = ((tod.t_date >> 5) & 15);
	d3 = ((tod.t_date >> 9) & 127)+1980;
	if (t1 > 59) {t2++; t1=0;};
	if (t2 > 59) {t3++; t2=0;};
	if (t3 > 23) {d1++; t3=0;};
	if (d2==2) {
		i=d3/4;
		if (i*4==d3) { /* leap year */
			if (d1>29) goto march;
		} else
			if (d1>28) {
march:				d1=1;
				d2++;
			};
	} else {
		if ((d2<8 && d2%2==1) || (d2>7 && d2%2==0)) {
			if (d1>31) goto newmonth;
		} else {        /* 30 days per month */
			if (d1>30) {
newmonth:			d1=1;
				d2++;
			}
		}
	}
	if (d2>12) {d2=1; d3++;};
	d3-=1980;
	tod.t_time = (((int)t3 << 8) << 3) |    /* hours in 24h format */
		     ((int)t2 << 5) |           /* min */
		     (t1 >> 1);                 /* sec/2 */
	tod.t_date = (((int)d3 << 8) << 1) |    /* year relative to 1980 */
		     ((int)d2 << 5) |           /* mon */
		     d1;                        /* day */
	tod_sec = t1;
}       
#endif	/* NO_RTC */

