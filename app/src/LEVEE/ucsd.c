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
#include "levee.h"
#include "extern.h"

#ifndef moveleft

PROC
moveleft(char *src, char *dest, int length)
{
    while (--length >= 0)
	*(dest++) = *(src++);
}

#endif /*moveleft*/

#ifndef moveright

PROC
moveright(src, dest, length)
  char *src;
  char *dest;
  register int length;
{
    src = &src[length];
    dest = &dest[length];
    while (--length >= 0)
	*(--dest) = *(--src);
}

#endif /*moveright*/

#ifndef fillchar

PROC
fillchar(char *src,int length, char ch)
{
    while (--length >= 0)
	*(src++) = ch;
}

#endif

int PROC
lvscan(int length, char tst, char ch, char *src)
{
    register int inc;
    int l;

    if (length < 0)
	inc = -1;
    else
	inc = 1;
    if (tst == '!') {
	for(l = ((int)inc)*length; l > 0; l--,src += inc)
	    if (*src != ch)
		break;
    }
    else {
	for(l = ((int)inc)*length; l > 0; l--,src += inc)
	    if (*src == ch)
		break;
    }
    return length-(inc*l);
}
