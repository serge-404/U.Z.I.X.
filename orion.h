#ifndef _ORION_H
#define _ORION_H

/*#ifdef __KERNEL__*/
#define DEV_TAB 5		/* records count in dev_tab table (majorN limit) */
/*#endif	/* __KERNEL__ */

#define MAXDRIV	 10		/* maximum number of "drives" - partitions */

/* #define BDOSADDR        5			/* real BDOS call routine address */
#define VTY_STATUS      ((unsigned char *)0x0f3f6)
#define VTY_ININT       (*VTY_STATUS & 0x20)
/*  D0 = 0/1 = CPM WIDTH 6/8
    D1 = 0/1 = CPM PROMPT OFF/ON
    D2 = 0/1 = CPM ECHO OFF/ON
    D3 = 0/1 = CPM FN10 BDOS (GetSTR) history recall off/on
    D4 = 0/1 = LPT F500/F600
    D5 = 0/1 = CPM VTY kernel driver busy within base INT 50 Hz handler  <- !!
    D6 = 0/1 = INT 50Hz notsupported/supported
    D7 = 0/1 = FN10 BDOS service bit
*/

extern char TEMPDBUF[];
extern char TEMPDBUFX[];

#ifdef ORI_UZIX

#define switch_page (*((uchar *)COMMON_MEM+6))	/* page where to switch context */
#define _SWITCH_PAGE _COMMONMEM+6

#define OLD_YINTVEC	((unsigned char *)0x0effa)		/* page:address, used only at kernel page   */
#define _OLDYINTVEC 0effah							/* for store CP/M int50hz vector, in kernel page */
/* #define TEMPDBUF 	(OLD_YINTVEC-513)   /* temporary disk buffer (512 bytes) inside kernel page */
/* #define TEMPDBUFX	(OLD_YINTVEC-257)   /* a half of temporary disk buffer (256 bytes) inside kernel page */
/* #define _TEMPDBUF _OLDYINTVEC-513 */
#define UDATA_STASH 0xef80					/* local process udata copy, in processes pages */
#define _UDATASTASH 0ef80h
#define BNK_MARKER	((unsigned char *)0x0efff)		/* unique memory page number - 0..15 for 1Mb*/
#define _BNKMARKER	0efffh							/* ordinal page number, all pages */

#else

#ifndef kprintf
#define kprintf printf
#endif	/* kprintf */

#endif  /* ORI_UZIX */

#endif /* _ORION_H */

