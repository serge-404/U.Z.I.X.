/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
/*
 * This deals with the atexit function call / on_exit removed - 2015
 *
 * Note: calls installed with atexit are called with the same args as
 * on_exit fuctions; the void* is given the NULL value.
 */

#include "errno.h"

#define MAXONEXIT 10		/* AIUI Posix requires 10 */

typedef void (*atexit_t) __P((int));
extern atexit_t __cleanup;
atexit_t __exit_table[MAXONEXIT];

int __on_exit_count = 0;

static void __do_exit(int rv);

static void __do_exit(int rv)
{
	register int count = __on_exit_count - 1;

	__on_exit_count = -1;	/* ensure no more will be added */
	__cleanup = 0;		/* Calling exit won't re-do this */
	/* In reverse order */
	while (count >= 0) {
		(*__exit_table[count])(rv);
		--count;
	}
}

int atexit(atexit_t ptr)
{
	if (__on_exit_count < 0 || __on_exit_count >= MAXONEXIT) {
		errno = ENOMEM;
		return -1;
	}

	__cleanup = (atexit_t)__do_exit;
	if (ptr) {
		__exit_table[__on_exit_count] = ptr;
		__on_exit_count++;
	}
	return 0;
}


