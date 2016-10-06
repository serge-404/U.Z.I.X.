/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"

/* ========	general purpose string handling ======== */

/* strcpy with arguments reversed and a more useful return value */
STRING
movstr(STRING a, STRING b)
{
	while (*b++ = *a++)
		continue;
	return b-1;
}

/* simpler form of strchr with arguments reversed */
int
any(char c, STRING s)
{
	char d;

	while (d = *s++)
		if (d == c)
			return(TRUE);
	return(FALSE);
}

int
cf(STRING s1, STRING s2)
{
	return strcmp(s1, s2);
}

/* return size of as, including terminating NUL */
int
length(STRING as)
{
	return (as == NIL? 0: strlen(as)+1);
}
