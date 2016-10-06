/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include <sys/types.h>
#include <sys/param.h>	/* for HZ */
#include "defs.h"	/* includes sys/types.h if not already included */

#ifndef HZ
#define	HZ	60
#endif

char numbuf[12];		/* big enough for 32 bits */

/* printing and io conversion */

void
newline(void)
{
	prc(NL);
}

void
blank(void)
{
	prc(SP);
}

void
prp(void)
{
	if ((flags & prompt) == 0 && cmdadr) {
		prs(cmdadr);
		prs(colon);
	}
}

#ifdef notdef
static void
verify(void)			/* DEBUG */
{
	if (output < 0)
		write(2, "-1\n", 3);
	if (dup2(output, 30) < 0)
		write(2, "closed\n", 7);
	else
		close(30);
}
#endif

void
prs(STRING s)
{
	if (s)
		write(output, s, length(s) - 1);
}

void
prc(char c)
{
	if (c)
		write(output, &c, 1);
}

void
prt(long t)		/* t is time in clock ticks, not seconds */
{
	int hr, min, sec;

	t += HZ / 2;	/* round to nearest second */
	t /= HZ;
	sec = t % 60;
	t /= 60;
	min = t % 60;
	if (hr = t / 60) {
		prn(hr);
		prc('h');
	}
	prn(min);
	prc('m');
	prn(sec);
	prc('s');
}

void
prn(int n)
{
	itos(n);
	prs(numbuf);
}

void
itos(unsigned long n)
{
	int pr, d;
	unsigned long a, i;
	char *abuf;

	abuf = numbuf;
	pr = FALSE;
	a = n;
	/*
	 * this routine is used to print pids (among other things), which be
	 * quite large on Crays or Plan 9.
	 */
	for (i = 1000000000; i != 1; i /= 10) {
		if ((pr |= (d = a / i)))
			*abuf++ = d + '0';
		a %= i;
	}
	*abuf++ = a + '0';
	*abuf = 0;
}

unsigned long
stoi(STRING icp)
{
	unsigned long r = 0;
	char *cp;
	char c;

	for (cp = icp; (c = *cp, digit(c)) && c && r >= 0; cp++)
		r = r*10 + c-'0';
	if (r < 0 || cp == icp)
		failed(icp, badnum);
	else
		return r;
}
