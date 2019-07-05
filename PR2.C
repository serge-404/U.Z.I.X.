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
 Processes management
**********************************************************/

#define NEED__PROCESS
#define NEED__FILESYS
#define NEED__MACHDEP
/* #define NEED__DEVSWAP */

#include "uzix.h"
#ifdef SEPH
#include "types.h"
#include "signal.h"
#include "errno.h"
#include "fcntl.h"
#include "sys\ioctl.h"
#endif
#include "unix.h"

#undef NEED__DEVSWAP 
#undef NEED__DEVTTY
#undef NEED__DISPATCH
#undef NEED__DEVIO
#undef NEED__DEVMISC
#undef NEED__DEVFLOP
#undef NEED__SCALL
#include "extern.h"

/* Ssig() send signal sig to process p and maybe
 * move process to ready-state
 */
GBL void ssig(ptptr proc, signal_t sig)
{
	register pstate_t stat;
	sigset_t mask = sigmask(sig);

	_di();
	if (proc == NULL || (uchar)(proc->p_status) <= P_ZOMBIE)
		goto Ret;	/* Presumably was killed just now */
	if (proc->p_ignored & mask)
		goto Ret;
	stat = proc->p_status;
	/* check for interruptable sleeping states */
	if ((uchar)stat == P_PAUSE ||
	    (uchar)stat == P_WAIT ||
	    (uchar)stat == P_SLEEP) {
		proc->p_status = P_READY;
		proc->p_wait = NULL;
		proc->p_intr = 1;
	}
	proc->p_pending |= mask;
Ret:	_ei();
}

/* Sendsig() send signal sig to process p
 * (or to all processes if p == NULL)
 */
GBL void sendsig(p, sig)
	register ptptr p;
	signal_t sig;
{
	if (p)
		ssig(p, sig);
	else {
		p = ptab;
		while (p < ptab+PTABSIZE) {
			if ((uchar)(p->p_status) > P_ZOMBIE)
				ssig(p, sig);
			++p;
		}
	}
}

extern void switch_context(void);

/* Clk_int() is the clock interrupt routine.
 * Its job is to increment the clock counters, increment the tick count
 * of the running process, and either swap it out if it has been in long
 * enough and is in user space or mark it to be swapped out if in system
 * space.
 * Also it decrements the alarm clock of processes.
 * This must have no automatic or register variables
 */
GBL void clk_int(VOID) {
	static ptptr p;

	/* Increment processes and global tick counters */
	incrtick(&ticks);
	if ((uchar)(UDATA(u_ptab)->p_status) == P_RUNNING)
		incrtick(UDATA(u_insys) ? &UDATA(u_stime) : &UDATA(u_utime));
	/* Do once-per-second things */
	if (++sec == TICKSPERSEC) {
		/* Update global time counters */
		sec = 0;
#ifdef NO_RTC
		updttod();	/* Really update time-of-day if no RTC */
#else
		rdtod();	/* Get updated time-of-day */
#endif
		/* Update process alarm clocks */
		p = ptab;
		while (p < ptab+PTABSIZE) {
			if (p->p_alarm) {
				if (--p->p_alarm == 0)
					sendsig(p, SIGALRM);
			}
			++p;
		}
	}
	/* Check run time of current process */
	if (((++runticks) >= UDATA(u_ptab)->p_cprio) &&
	    !UDATA(u_insys) &&
	    ((uchar)(UDATA(u_ptab)->p_status) == P_RUNNING)) { /* Time to swap out */
		_di();
		UDATA(u_insys)++;
		UDATA(u_inint) = inint;
		/* If inint == 0 then it will cause interrupts to be enabled
		 * while doing do_swap. swapin is responsible for ei().
		 */
		UDATA(u_ptab)->p_status = P_READY;  /**/
#ifdef ORI_UZIX
		switch_page=0xff;	/* do not swapin() inside swapout(), just actualizes switch_page */
		xswapout();			/* also xswapout() initializes _pp - pointer to next process       */
		if (switch_page) {
			runticks = 0;
			switch_context();	/* prepare f3-stack to exiting of ISR _service() and saves context */	
		}
#else
		swapout();			/* initializes _pp - pointer to next process */
#endif
		_di();
		UDATA(u_insys)--;	/* We have swapped back in */
		inint = UDATA(u_inint);
		/*
		  we can't return to service with inint=0, or --inint
		  will do inint=255, so service will never be called again
		*/
	}
#ifdef ORI_UZIX
	else {
	/* Deal with a pending caught signal, if any */
		if (inint == 1 && !UDATA(u_insys))
			calltrap();     /* already inint == 0 ? */
	}
#endif
}


