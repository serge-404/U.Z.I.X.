/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include "string-l.h"

/********************** Function strrchr ************************************/
#ifdef L_strrchr
char *strrchr(s, c)
	char *s;
	int c;
{
	register char *p = s + strlen(s);

	/* For null it's just like strlen */
	if (c == '\0')
		return p;
	while (p != s) {
		if (*--p == c)
			return p;
	}
	return NULL;
}
#endif
