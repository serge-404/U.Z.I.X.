/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-1997 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by David L Parsons (orc@pell.chi.il.us).  My name may not be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.  THIS SOFTWARE IS PROVIDED
 * AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * Unix interface for levee
 */
#include "levee.h"
#include "extern.h"
#if !VT52
#include <termios.h>
#endif
#ifdef GCC
#include <ioctls.h>
#endif

#if VT52
#include <ioctl.h>
#endif

#include <string.h>
/*
#include <unistd.h>
*/
extern int read(int d, void *buf, uint nbytes);
extern int write(int d, void *buf, uint nbytes); 
#ifdef ORI_UZIX
char* CLKset="\033Z\003\001";  /* esc,'Z',3,byte - set clock mode */
char* CLKoff="\033Z\003\002";  /* D0=1 for show, D0=0 for hide */
#endif


int
min(int a, int b)
{
    return (a>b) ? b : a;
}

int
max(int a,int b)
{
    return (a<b) ? b : a;
}

strput(char *s)
{
    if (s)
	write(1, s, strlen(s));
}


#ifndef GCC
char *basename(char *s)
{
    register char *p;

    if (p=strrchr(s,'/'))
	return 1+p;
    return s;
}
#endif


static int ioset = 0;
#if !VT52
static struct termios old;
#endif

void initcon(void)
{
#if !VT52
    struct termios new;
#else
    register unsigned char i;
#endif

    if (!ioset) {			/* preserve current terminal setup */
#if VT52
#ifdef ORI_UZIX
	ioctl(STDIN_FILENO, TTY_RAWCHAR);
        strput("\033Z\002");            /* get clock mode byte into CLKset[3] */
        for (i=200; !read(0,&CLKset[3],1) && i ; i--);
        if (! CLKset[3]) *CLKset=0;
        strput(CLKoff);                 /* because clock moving with scrolled text */
#endif
        strput(CURon);
#else
	tcgetattr(0, &old);	

        erasechar = old.c_cc[VERASE];
        eraseline = old.c_cc[VKILL];

        memcpy(&new, &old, sizeof(new));

	new.c_iflag &= ~(IXOFF|IXANY|ICRNL|INLCR);
	new.c_lflag &= ~(ICANON|ISIG|ECHO);
	new.c_oflag = 0;

	tcsetattr(0, TCSANOW, &new); 	/* terminal setup for editor */
#endif
        ioset=1;
    }
}

void fixcon(void)
{
    if (ioset) {
#if VT52
#ifdef ORI_UZIX
        strput(CLKset);
#endif
        strput(CURon);
	ioctl(STDIN_FILENO, TTY_COOKED);
#else
        tcsetattr(0, TCSANOW, &old); 	/* restore original terminal setup */
#endif
        ioset = 0;
    }
}

int getKey(void)
{
    unsigned char c;
    while (!read(0,&c,1)) ;   /* for non-blocking read */
#ifdef ORI_UZIX
    switch (c) {
	case 8:  return LTARROW;
	case 4:  return RTARROW;
	case 5:  return UPARROW;
	case 24: return DNARROW;
        default: return c;
    }
#endif
}
