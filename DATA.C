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
 UZIX utils data (vars, structures)
**********************************************************/

#define __MAIN__COMPILATION
#define NEED__MACHDEP
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


#undef NEED__FILESYS
#undef NEED__DEVSWAP
#undef NEED__PROCESS
#include "extern.h"
#define __DEVIO__COMPILATION
#include "config.h"



