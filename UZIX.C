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
 Main procedure and UZIX data
**********************************************************/

#define __MAIN__COMPILATION
#define NEED__DISPATCH
#define NEED__SCALL
#define NEED__DEVFLOP
#define NEED__DEVIO

#include "uzix.h"
#ifdef SEPH
#include "signal.h"
#endif
#include "types.h"
#include "unix.h"

#undef NEED__MACHDEP 
#undef NEED__FILESYS
#undef NEED__DEVSWAP
#undef NEED__PROCESS
#include "extern.h"
#define __DEVIO__COMPILATION
#include "config.h"

#include "dispatch.c"

char *UZIX="UZIX";

#ifdef ORI_UZIX
extern uint _getc(void);
extern uchar inint;			/* inside interrupt */

int isdig(char cc) {
  return ((cc>='0') && (cc<='9'));
}
#endif

/*
extern void clkint(void);
extern int getfsys(dev_t devno,	void *buf);
*/
uint xbioscold, xbdoscold;
extern void setcalltrap(void);
extern short bdoshl(int, ...);

void main(argc, argv)
	int argc;
	char *argv[];
{
#ifdef ORI_UZIX
	xbioscold=(*((uint *)0x0001))-3;
	xbdoscold=(*((uint *)0x0006));
	setcalltrap();
#endif
	bufinit(); 	/* do all buffers free without flushing */
	d_init();
#if ORI_DEBUG
	ptab_t ptt;
	
	inint=1;
	__ei();
	
	kprintf("\n p_swap-p_status=%d, sizeof(p_status)=%d, sizeof(ptab_t)=%d", (int)&ptt.p_swap-(int)&ptt.p_status, sizeof(ptt.p_status), sizeof(ptt));

	kprintf("\nclkint=%x, rawrw=%x, getfsys=%x, rawrw2=%x\nTesting console I/O.\nHit any key. ESCAPE ends. CTRL+C for exit.\n\n",
			(int)clkint, (int)rawrw, (int)getfsys, (int)rawrw2);
/*	uchar a;
	for (;;) {
		while ((a = _getc()) == 0)
			;
		if (a == 27)
			break;
		if (a == 3)
			exit(0);
		_putc(a);
	}
*/
#endif
	inint=0;
#ifdef ORI_UZIX
#ifdef NO_RTC
	tod.t_time = bdoshl(176+9);		/* HHHHHmmm.mmmSSSSS */
	tod.t_date = bdoshl(176+8);		/* YYYYYYYm.mmmDDDDD */
#endif
	if (argc < 2 || !isdig(argv[1][0]) || (argv[1][1] != ':')) {
		kprintf("usage: uzix d:\n\n\td = root device (0..7)\n");
		exit(1);
	}
	root_dev = argv[1][0] - '0';
#endif
	kprintf("Welcome to %s %s.%s\n", UZIX, VERSION, RELEASE);
	arch_init();		/* NORETURN */
}


