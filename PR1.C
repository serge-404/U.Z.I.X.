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

#define NEED__DEVTTY
#define NEED__MACHDEP
#define NEED__PROCESS
#define NEED__FILESYS
#define NEED__DEVSWAP
#define NEED__DEVIO

#include "uzix.h"
#ifdef SEPH
#include "types.h"
#include "signal.h"
#include "errno.h"
#include "fcntl.h"
#include "sys\ioctl.h"
#endif
#include "unix.h"

#undef NEED__DISPATCH
#undef NEED__DEVMISC
#undef NEED__DEVFLOP
#undef NEED__SCALL
#include "extern.h"

#define BLDIR		0f201h	/* multybank  ldir    */

extern int sys_open(void);
extern int sys_sync(void);
extern int sys_execve(void);

LCL void swapin(void); /* __P((void)); */
LCL ptptr nextready __P((void));
LCL void newproc __P((ptptr p));
LCL ptptr ptab_alloc __P((void));
LCL void shootdown __P((uint));

STATIC void *_stkptr;	/* Temp storage for swapin/swapout/dofork */
STATIC ptptr _pp;	/* Temp storage for swapin/swapout/dofork */

/* Common strings. They are here to save memory */
static char *notfound = "no %s, error %d";
#ifdef ORI_UZIX
extern char bininit[];
#else
static char *bininit = "/bin/init";
#endif

/* Init() */
GBL void init(VOID) {
#ifndef ORI_UZIX
	static char *arg[2] = {"init", NULL};	/* "arg" actually placed at process page */
#endif	
	/* there is no need to fill with zeros the structures not initialized,
	   in the bss segment (such as ptab, fs_tab, modtab, etc), since UZI.AS
	   already did it.
	*/
	rdtod();
	ptotal = 1;
	bzero(UDATA_ADDR, sizeof(udata_t));
	UDATA(u_insys) = 1;	/* flag - in kernel */
	UDATA(u_mask) = 022;	/* standard user mask */

	bfill(UDATA(u_files),-1,sizeof(UDATA(u_files)));
	bcopy(UZIX,UDATA(u_name),5);
	bufinit();
	d_init();	/* initialize all devices */

	d_open(TTYDEV); /* Open the console tty device */
#ifndef	ORI_UZIX
	swap_open();	/* Open the swaper */
#endif
	/* Create the context for the first process */
	initproc = ptab_alloc();
	UDATA(u_ptab) = initproc;
#ifdef	ORI_UZIX
	swap_alloc(&(initproc->p_swap));
#else
	swap_alloc(initproc->p_swap);
#endif
	newproc(initproc);
	initproc->p_status = P_RUNNING;
	kprintf("\nMounting root fs: ");
#ifndef	ORI_UZIX
	root_dev = (*(uchar *)BDRVADDR);
#endif
#ifdef CHOOSE_BOOT
	{
	char bootchar;

	UDATA(u_base) = &bootchar;
	UDATA(u_count) = 1;
	cdread(TTYDEV);
	root_dev += bootchar - '0';
	}
#endif
#ifdef USETEST
	tester(0);
#endif
	/* Mount the root device */
	if (fmount(root_dev, NULL, 0))
		panic(notfound, "filesystem",UDATA(u_error));
	if ((root_ino = (inoptr)i_open(root_dev, ROOTINODE)) == 0)
		panic(notfound, "root", UDATA(u_error));
	kprintf("ok\n");
	UDATA(u_root) = root_ino;	/* already referenced in i_open() */
	UDATA(u_cwd) = root_ino;
	i_ref(root_ino);
	rdtime(&UDATA(u_time));
	/* Run /init and starting roll! */
#ifdef ORI_UZIX
	setvectors();
	UDATA(u_argn1) = xconsole;		/* args actually placed at process page */
#else
	UDATA(u_argn1) = (int) ("/dev/console");
#endif
	UDATA(u_argn2) = O_RDONLY;
	sys_open();						/* stdin */
	UDATA(u_argn2) = O_WRONLY;
#ifdef ORI_UZIX
	UDATA(u_argn1) = xconsole;		/* args actually placed at process page */
#endif
	sys_open();						/* stdout */
#ifdef ORI_UZIX
	UDATA(u_argn1) = xconsole;		/* args actually placed at process page */
#endif
	sys_open();						/* stderr */
#ifdef USETEST
	tester(1);
#endif
#ifdef ORI_UZIX
	UDATA(u_argn1) = xbininit;		/* args actually placed at process page */
	UDATA(u_argn2) = xinitarg0;		
	UDATA(u_argn3) = xinitarg1;
#else
	UDATA(u_argn1) = (int) (bininit);
	UDATA(u_argn2) = (int) (&arg[0]);
	UDATA(u_argn3) = (int) (&arg[1]);
#endif
	sys_execve();
	panic(notfound, bininit, UDATA(u_error));
	/* NORETURN */
}

/* shootdown() */
LCL void shootdown(val)
	uint val;
{
	sys_sync();
#ifndef ORI_UZIX
	swap_close();
#endif
	kprintf("%s stopped (%p)\n", UZIX, val);
	_abort(1);
}

/* This adds two tick counts together.
 * The t_time field holds up to sixty second of ticks,
 * while the t_date field counts minutes
 */
GBL void addtick(t1, t2)
	register time_t *t1, *t2;
{
	t1->t_time += t2->t_time;
	t1->t_date += t2->t_date;
	while (t1->t_time >= TICKSPERMIN) {
		t1->t_time -= TICKSPERMIN;
		++t1->t_date;
	}
}

/* add one tick to time
 * The t_time field holds up to sixty second of ticks,
 * while the t_date field counts minutes
 */
GBL void incrtick(t)
	register time_t *t;
{
	++t->t_time;
	while (t->t_time >= TICKSPERMIN) {
		t->t_time -= TICKSPERMIN;
		++t->t_date;
	}
}

/* This interrupt device routine calls the service routine of each device
 * that could have interrupted.
 * Executed with the interrupts disabled and ++inint!
 *
 * ! can't be executed with SP=ISTK. SP must be user stack, to have
 * ! registers and return address swapped with the process
 */
GBL void service(VOID) {
	tty_poll(); 
#ifdef ORI_UZIX
	clk_int();		/* calltrap() moved into clk_int() */
#else	
	clk_int();
	/* Deal with a pending caught signal, if any */
	if (inint == 1 && !UDATA(u_insys))
		calltrap();     /* already inint == 0 ? */
#endif
}

/* Swapout() swaps out the current process, finds another that is READY,
 * possibly the same process, and swaps it in.
 * When a process is restarted after calling swapout,
 * it thinks it has just returned from swapout().
 
ORI_UZIX: ret_page variable must be defined before swapout call (its for _save_context)

 */
GBL void xswapout(void) {
#ifndef LOC_UDATA
	/* Auto variables must conform to swapin/dofork autos ! */
	auto udata_t ud;
#endif
	chksigs();		/* See if any signals are pending */
	_pp = nextready();	/* Get a new process */
	/* If there is nothing else to run, just return */
	if (_pp == UDATA(u_ptab)) {
		UDATA(u_ptab)->p_status = P_RUNNING;
#ifdef ORI_UZIX
		switch_page = 0;				
#endif
		runticks = 0;
		/* UDATA(u_ptab)->p_cprio-1;	/* add only one tick! */
		return;
	}
#ifndef ORI_UZIX
#ifndef LOC_UDATA
	/* Save the user data structure on stack */
	bcopy(UDATA_ADDR,&ud,sizeof(udata_t));
#endif
	/* Save the stack pointer and critical registers */
	/* push 1	; ret status
	 * push si, di, bp
	 * _stkptr = (void *)_SP;
	 */
#define __SAVEALL	swapout_SAVEALL
#ifdef PC_HOSTED
#include "process.mtc"
#else
#ifdef MSX_HOSTED
#include "process.msx"
#endif
#endif
	UDATA(u_ptab)->p_sp = _stkptr;	/* save SP in core memory */
#ifdef	LOC_UDATA
	UDATA(u_ptab)->p_udata = UDATA_ADDR; /* back link to static udata */
#else
	UDATA(u_ptab)->p_udata = &ud;	/* back link to saved udata */
#endif
#ifndef MEMBANKING
	if (*(uint *)UDATA(u_ptab)->p_swap != 0) /* not in-core process */
		swap_write(UDATA(u_ptab)->p_swap);
#endif
#else  /* ORI_UZIX */
	if (switch_page) {				/* switch_page!=0 if switching inside ISR */	
		switch_page = _pp->p_swap;
		return; /* in case of switching inside ISR we just return - later correcting f3-stack and return to _save_context of interrupted process: it does switching itself */
	}
#endif /* ORI_UZIX */
	/* Swap the next process in, and return into its context. */
	swapin();
	/* NORETURN */
	panic("swapin failed");
}

/* psleep() puts a process to sleep on the given event.
 * If another process is runnable, it swaps out the current one
 * and starts the new one.
 * Normally when psleep is called, the interrupts have already been
 * disabled. An event of 0 means a pause(), while an event equal
 * to the process's own ptab address is a wait().
 */
GBL void psleep(event)
	void *event;
{
	register pstate_t sts = P_PAUSE;
	ptptr pt = UDATA(u_ptab);

	_di();
	if (pt->p_status != P_RUNNING)
		panic("psleep: voodoo");
	if (UDATA(u_ptab)->p_pending == 0) {	/* pending signals */
		if (event) {
			sts = (event == pt) ? P_WAIT : P_SLEEP;
		}
		pt->p_status = sts;
		pt->p_wait = event;
		pt->p_intr = 0;
	}
	else	pt->p_intr = 1;
	_ei();
#ifdef ORI_UZIX
	ret_page=KERNEL_PAGE;
	switch_page=0;
#endif
	swapout();	/* Swap us out, and start another process */			/* STACK: B7067F 05C1F0F7 733E40B5 79EF0000*/
	/* Swapout doesn't return until we have been swapped back in */
}

/* Wakeup() looks for any process waiting on the event,
 * and make them runnable.
 * If event is null this procedure will wakeup all paused processes.
 */
GBL void wakeup(event)
	void *event;
{
	register ptptr p = ptab;

	_di();
	while (p < ptab+PTABSIZE) {
		if ((uchar)(p->p_status) >= P_READY && p->p_wait == event) {
			p->p_status = P_READY;
			p->p_wait = NULL;
		}
		++p;
	}
	_ei();
}

/* Nextready() returns the process table pointer of a runnable process.
 * It is actually the scheduler.
 * If there are none, it loops.  This is the only time-wasting loop in the
 * system.
 */
LCL ptptr nextready(VOID) {
	static ptptr pp = ptab; /* Pointer for round-robin scheduling */
	register ptptr p = pp;

	for (;;) {
		if (++p >= ptab + PTABSIZE)
			p = ptab;
		if (p == pp) {		/* Loop closed */
#if DEBUG > 0
			/* sys_sync() causes system to access disk very
			   often and slowing down UZIX if running on
			   floppy. since it's just for wasting time, it
			   was supressed on non-debug versions.
			 */
			sys_sync();	/* waste time by sync */
#endif
			/* ok, no ready process was found. so, let's
			   enable interrupts and maybe clk_int() can
			   cleanup process table to us */
			__ei();
			/* and let's call tty_poll(), so user don't think
			   we are frozen and we also collect keyboard data */
			tty_poll();
		}
		if ((uchar)p->p_status == P_RUNNING)
			panic("nextready: extra running");
		if ((uchar)p->p_status == P_READY) 
			return (pp = p);
	}
}


/* Newproc() fixes up the tables for the child of a fork
 */
LCL void newproc(p)
	register ptptr p;	/* New process table entry */
{
	register uchar *j;

	/* Note that ptab_alloc clears most of the entry */
	_di();
	runticks = 0;
	p->p_pptr = UDATA(u_ptab);		/* parent */
	p->p_ignored = UDATA(u_ptab)->p_ignored; /* inherited signal masks */
	p->p_uid = UDATA(u_ptab)->p_uid; 	/* inherited user id */
	p->p_cprio = MAXTICKS;			/* timeslice */
	p->p_nice = 0;				/* nice value */
#ifdef UZIX_MODULE
	RESET_MODULE();
#endif
	UDATA(u_ptab) = p;
	bzero(&UDATA(u_utime), 4 * sizeof(time_t)); /* Clear tick counters */
	rdtime(&UDATA(u_time));
	UDATA(u_cursig) = UDATA(u_error) = 0;
	if (UDATA(u_cwd))  i_ref(UDATA(u_cwd));	 /* add ref to current dir */
	if (UDATA(u_root)) i_ref(UDATA(u_root)); /* add ref to current root */
	j = UDATA(u_files);
	while (j != UDATA(u_files) + UFTSIZE) {
		if (!freefileentry(*j))
			++of_tab[*j].o_refs;	/* add ref to inherited files */
		j++;
	}
	_ei();
}

/* Ptab_alloc() allocates a new process table slot, and fills
 * in its p_pid field with a unique number.
 */
LCL ptptr ptab_alloc(VOID) {
	static uint nextpid = 0;
	register ptptr p, pp = ptab;

	_di();
	while (pp < ptab+PTABSIZE) {
		if ((uchar)(pp->p_status) == P_EMPTY)
			goto Found;
		++pp;
	}
	_ei();
	return NULL;

	/* See if next pid number is unique */
Found:	p = pp;
Loop:	if (++nextpid & 0x8000) 	/* pid always positive value! */
		nextpid = 1;
	pp = ptab;
	while (pp < ptab+PTABSIZE) {
		if ((uchar)pp->p_status != P_EMPTY && pp->p_pid == nextpid)
			goto Loop;
		++pp;
	}
	bzero(pp = p, sizeof(ptab_t));
	p->p_pid = nextpid;
	p->p_status = P_FORKING;
	_ei();
	return pp;
}

/* doexit() destroys current process and doing all cleanups
 */
GBL void doexit(val, val2)
	uchar val;
	uchar val2;
{
	register uchar j;
	register ptptr p;

#ifdef UZIX_MODULE
	clear_module_reqs(UDATA(u_ptab)->p_pid);
#endif
	_di();
	UDATA(u_ptab)->p_status = P_ZOMBIE;	/* don't swapout me! */
	/* close all opened files */
	j = UFTSIZE /* - 1*/ ;
	while (j-- != 0) {
		if (!freefileentry(UDATA(u_files)[j]))
			doclose(j);
	}
	i_deref(UDATA(u_root));
	i_deref(UDATA(u_cwd));
	sys_sync();	/* Not necessary, but a good idea. */
	UDATA(u_ptab)->p_exitval = (val << 8) | val2;	/* order is important */
	/* Set child's parent to init (UZI180/280 sets to our parent) */
	p = ptab;
	while (p < ptab+PTABSIZE) {
		if ((uchar)(p->p_status) != P_EMPTY &&
		    p->p_pptr == UDATA(u_ptab) && p != UDATA(u_ptab))
			p->p_pptr = initproc; /* UDATA(u_ptab)->p_pptr; */
		++p;
	}
#ifdef ORI_UZIX
	swap_dealloc(&(UDATA(u_ptab)->p_swap));	/* free process swap area */
#else
	swap_dealloc(UDATA(u_ptab)->p_swap);	/* free process swap area */
#endif
	/* Stash away child's execution tick counts in process table,
	 * overlaying some no longer necessary stuff.
	 */
	addtick(&UDATA(u_utime), &UDATA(u_cutime));
	addtick(&UDATA(u_stime), &UDATA(u_cstime));
	bcopy(&UDATA(u_utime), &(UDATA(u_ptab)->p_break), 2 * sizeof(time_t));   
	/* Wake up a waiting parent, if any. */
	if (UDATA(u_ptab) != initproc)
		wakeup(UDATA(u_ptab)->p_pptr);
	_ei();
	if (--ptotal == 0)
		shootdown(UDATA(u_ptab)->p_exitval);
	_pp = nextready();
	swapin();
	/* NORETURN */
}

/* dowait()
 */
#define pid (int)UDATA(u_argn1)
#define statloc (int *)UDATA(u_argn2)
#define options (int)UDATA(u_argn3)
GBL int dowait()
{
	register ptptr p;
	int retval;
	
	for (;;) {
		/* Search for an exited child */
		_di();
		p = ptab;
		while (p < ptab+PTABSIZE) {
			if (p->p_pptr == UDATA(u_ptab) && /* my own child! */
			   (uchar)(p->p_status) == P_ZOMBIE) {
				retval = p->p_pid;
			    	/* if (pid < -1)...
			    	   wait for any child whose group ID is equal
			    	   to the absolute value of PID
			    	   - NOT IMPLEMENTED -
			    	   
			    	   if (pid == 0)...
			    	   wait for any child whose group ID is equal
			    	   to the group of the calling process
			    	   - NOT IMPLEMENTED -
			    	   
			    	   if option == WUNTRACED...
			    	   - NOT IMPLEMENTED -
			    	*/
			    	if (pid > 0) if (retval != pid) continue;
				if (statloc != NULL) 
#ifdef ORI_UZIX
					UDATA(u_argn4)=p->p_exitval;
					B_LDIRTON(2,&UDATA(u_argn4),statloc);
#else				
					*statloc = p->p_exitval;
#endif
				p->p_status = P_EMPTY;	/* free ptab now! */
				/* Add in child's time info.
				 * It was stored on top of p_alarm in the childs
				 * process table entry
				 */
				addtick(&UDATA(u_cutime), (time_t *)&(p->p_break)+0);
				addtick(&UDATA(u_cstime), (time_t *)&(p->p_break)+1);
				_ei();
				return retval;
			}
			++p;
		}
		if ((options & WNOHANG) != 0) {
			UDATA(u_error) = ECHILD;
			return 0;
		}
		/* Nothing yet, so wait */
		psleep(UDATA(u_ptab));	/* P_WAIT */
		if (UDATA(u_ptab)->p_intr) {
			UDATA(u_error) = EINTR;
			return -1;
		}
	}
}
#undef options
#undef statloc
#undef pid

/* chksigs() sees if the current process has any signals set,
 * and deals with them
 */
GBL void chksigs(VOID) {
	register uchar j;
	sigset_t mask;
	sig_t vect;
			
	_di();
	if (UDATA(u_ptab)->p_pending != 0) {
		j = 1;
		while (j != NSIGS+1) {
			mask = sigmask(j);
			vect = (void*)(UDATA(u_sigvec)[j-1]);
			if ((mask & UDATA(u_ptab)->p_pending) != 0) {
				if (vect == (void*)SIG_DFL) {
					_ei();
					doexit(0, j);
				}
				if (vect != (void*)SIG_IGN) {
					/* Arrange to call the user routine at return */
					UDATA(u_ptab)->p_pending &= ~mask;
					UDATA(u_cursig) = j;
					break;	/* don't check next signals! */
				}
			}
			++j;
		}
	}
	_ei();
}

