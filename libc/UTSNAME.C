/*
 * utsname.c for UZIX
 * by A&L Software 1999
 *
 */

#include "utsname.h"
#include "unistd.h"
#include "string.h"
#include "ctype.h"
#ifdef __TURBOC__
#include <..\kernel\unix.h>
#endif

#ifdef HI_TECH_C

#include "ioctl.h"
typedef struct s_kdata {
	char	k_name[14];	/* OS name */
	char	k_version[8];	/* OS version */
	char	k_release[8];	/* OS release */
	char	k_machine[8];	/* Host machine */
	int	k_tmem; 	/* System memory, in kbytes */
	int	k_kmem; 	/* Kernel memory, in kbytes */
};

#endif

int uname (__utsbuf)
	struct utsname *__utsbuf;
{
	struct s_kdata kdata;
	int i;

	getfsys(GI_KDAT, &kdata);
	strcpy(__utsbuf->sysname,kdata.k_name);
	strcpy(__utsbuf->nodename,kdata.k_name);
	for (i=0;i<strlen(__utsbuf->nodename);i++)
		__utsbuf->nodename[i]=tolower(__utsbuf->nodename[i]);
	strcpy(__utsbuf->release,kdata.k_release);
	strcpy(__utsbuf->version,kdata.k_version);
	strcpy(__utsbuf->machine,kdata.k_machine);
	strcpy(__utsbuf->domainname,"(localhost)");
	return 0;
}
