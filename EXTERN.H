
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
 External variables and global declarations
**********************************************************/

#ifndef _EXTERN_H
#define _EXTERN_H

/* These are the global data structures */
#ifdef __MAIN__COMPILATION
#define EXT
#else
#define EXT	extern
#endif

#define GBL
#define LCL
#define STATIC

#ifdef PC_HOSTED	/* only for Turbo-C! */

void __cli__();
void __sti__();
void __int__(int);
void __emit__();

#ifdef PC_UTILS_TARGET

#define _di()
#define _ei()
#define __ei()

#else

#define geninterrupt(i) __int__(i)

#undef _di
#undef _ei
#define _di()	__cli__()
#define _ei()	{ if (!inint) __sti__(); }
#define __ei()	 __sti__()

#endif

#define MAXDRIV	 2	/* maximum number of floppy drives */
					/* must be power of two */
#endif	/* PC_HOSTED */

#ifdef MSX_HOSTED

#ifdef Z80MU
#define cursor(p)
#else
#define cursor(p) ((*(uchar *)0xFCA9) = (p)) /* Turns cursor on (255) or off (0) */
#endif

#ifdef __KERNEL__
#define CALROM	 0801Ch			/* ROM call routine address */
#define BDOSADDR 0801Fh			/* real BDOS call routine address */
#define	BDOS	 _bdospatch		/* BDOS call routine address */
#define MAINDRV	 0F348h			/* diskrom slot to access */
#asm
	global	_bdospatch, _bdostr, _bdosaddr
	global	_bootdskrom
#endasm
#else
#define CALROM	 0F710h			/* ROM call routine address */
#define BDOS	 0F713h			/* BDOS call routine address */
#define MAINDRV  0F716h			/* diskrom slot to access */
#endif
#define DOSVER	(*(char *)0xf313)	/* MSX: 0=DOS1 */

/* MSX UZIX memory map:

   Notes:
   UZIX kernel can't be greater than F180h, or it will hang MSX Turbo-R.
   Beware of address F661h. It used by CHPUT of MSX BIOS.

   0000 ---------------------
     |	    system vectors
   0010 ---------------------
     |	     process data
   0100 ---------------------
     |	     user process
   8000 ---------------------
     |	kernel initialization
   801C ---------------------
     |	   ROM/BDOS caller
   809D ---------------------
     |	     kernel area
   F180 ---------------------
     |    DiskROM hook area
   F380 ---------------------
     |   MSX system var area
   F419 ---------------------
     |	   disk buffer area
   F619 ---------------------
     |	     system area
   F660 ---------------------
     |	ROM caller stack area
   F6EE ---------------------
     |	  kernel data area
   F710 ---------------------
     |	  kernel temp stack
   F7C5 ---------------------
     |	     system area
   FDFD ---------------------
     |	       UZIX ISR
   FF01 ---------------------
     |	     system area
   FFFF ---------------------

*/

#define BDRVADDR 63218	/* MSX boot drive (F6F2h, 1 byte) */
#define CNFGBYTE 63219	/* UZIX configuration byte */
#define TICKADDR 63220	/* MSX frequency byte indicator (F6F4h, 1 byte) */
#define RAMADDR  63221	/* MSX swap RAM size (F6F5h, 1 byte) */
#define RAMSLOT  63222	/* MSX swap RAM slot (F6F6h, 1 byte) */
#define IM2ADDR  65021	/* IM 2 interruption call address (FDFDh, 3 bytes)
			   (KILL command hook) */
#define TEMPDBUF 62489	/* temporary disk buffer (F419h, 512 bytes) */
#if DEBUG > 1
#define TSTK	_tmpstk+TMPSTK-2	/* exec() tmp stack */
#else
#define TSTK	 0f7c3h
#endif
#define MAXDRIV	 8	/* maximum number of floppy drives */
			/* must be power of two */

#endif /* MSX_HOSTED */

#define DEV_TAB

#ifdef ORION_HOSTED
/*
1B,':' - включить курсор
1B,';' - выключить курсор
*/
/* Turns cursor on (p=255) or off (p=0)  BIOS_CONOUT=4 */
#define cursor(p) /* (p ? _displaycursor() : _erasecursor()) */

#include "orion.h"

#endif /* ORION_HOSTED */


#define __EXTERN_H_IN_PROGRESS
#include "config.h"
#undef	__EXTERN_H_IN_PROGRESS

EXT uchar MAXTICKS;	/* Max ticks before swapping out (time slice) */
EXT uchar TICKSPERSEC;	/* Ticks per second */
EXT int TICKSPERMIN;	/* Ticks per minute */

#define COMMON_MEM		0xf162

#ifdef ORI_UZIX

#define _COMMONMEM		0f162h
#define udata			(*((udata_t *)(COMMON_MEM+7)))		/* see also _UDATAADDR*/
#define	UDATA(a)		(udata.a)
#define	UDATA_ADDR		(&udata)
#else
#ifdef LOC_UDATA
#if DEBUG > 2
#ifdef __MAIN__COMPILATION
udata_t *_udata_ = ((udata_t *)0x10);
#else
EXT udata_t *_udata_;
#endif
#define	UDATA(a)	(_udata_->a)
#define	UDATA_ADDR	(_udata_)
#else
#define udata	(*((udata_t *)0x10))
#define	UDATA(a)	(udata.a)
#define	UDATA_ADDR	(&udata)								/* see also _UDATAADDR*/
#endif
#else
EXT udata_t udata;		/* MUST BE FIRST */
#define	UDATA(a)	(udata.a)
#define	UDATA_ADDR	(&udata)
#endif
#endif

#ifdef __KERNEL__
EXT ptab_t ptab[PTABSIZE];	/* processes descriptors */
EXT uchar ptotal;		/* total active processes */
#endif
#if DEBUG > 1
EXT uchar traceon;		/* syscall tracing on */
#endif

#ifdef UZIX_MODULE
EXT modtab_t modtab[MTABSIZE];		/* modules table */
EXT modreply_t modreply[RTABSIZE];	/* modules reply table */
#endif


#ifdef __KERNEL__
#if DEBUG > 1 || defined(PC_HOSTED)
EXT char tmpstk[TMPSTK];	/* temporary stack */
#endif
#endif

extern char *UZIX;

EXT inoptr root_ino;		/* Address of root dir in inode table */
EXT dev_t root_dev;		/* Device number of root filesystem. */

EXT cinode_t i_tab[ITABSIZE];	/* In-core inode table */
EXT oft_t of_tab[OFTSIZE];	/* Open File Table */

EXT filesys_t fs_tab[NFS];	/* Table entry for each device with a fs. */
EXT blkbuf_t bufpool[NBUFS];	/* Buffer pool */
EXT uchar dirty_mask;		/* 1 if buffer writing delayed, else 0 */

EXT ptptr initproc;	/* The process table address of the first process. */
EXT volatile uchar inint; /* flag is set whenever interrupts are being serviced */

EXT uchar sec;		/* Ticks counter for counting off one second */
EXT uchar runticks;	/* Number of ticks current process has been swapped in */

#ifndef ORI_UZIX
EXT time_t tod; 	/* Time of day */
#else
#define tod	*((time_t *)0x0f3d8)  /* autoupdated by driver.sys ORION CP/M console driver */
#endif

EXT time_t ticks;	/* Cumulative tick counter, in minutes and ticks  */

EXT uchar total;	/* Total available memory 16k pages */

#ifdef USETEST
extern void tester __P((int where));
#endif

#ifdef NEED__DEVFLOP
/* DEVFLOP.C */

extern int fd_init __P((uchar minor));
extern int fd_open __P((uchar minor));
extern int fd_close __P((uchar minor));
extern int fd_ioctl __P((uchar minor, int, void *));
extern int fd_read __P((uchar minor, uchar rawflag));
extern int fd_write __P((uchar minor, uchar rawflag));

#endif

#ifdef NEED__DEVSWAP
/* DEVSWAP.C */

#ifdef __KERNEL__
#ifndef	ORI_UZIX
extern int swap_init __P((void));
extern int swap_open __P((void));
extern int swap_close __P((void));
extern int swap_read __P((page_t *mm));
#endif
extern int swap_alloc __P((page_t *mm));
extern int swap_dealloc __P((page_t *mm));
extern int swap_mmread __P((struct swap_mmread *rp));
#ifdef MEMBANKING
#ifdef ORI_UZIX
extern int swap_dup __P((page_t mm, page_t mmnew));
#else
extern int swap_dup __P((page_t *mm, page_t *mmnew));
#endif
#else
extern int swap_write __P((page_t *mm));
#endif
#endif /*__KERNEL__*/

#endif

#ifdef NEED__DEVTTY
/* DEVTTY.C */

#ifndef MSX_HOSTED			/* PC_HOSTED, ORION_HOSTED */
#define ERASECURSOR()
#define DISPLAYCURSOR()
#else					/* MSX_HOSTED */
#define ERASECURSOR()	_erasecursor()
#define DISPLAYCURSOR()	_displaycursor()
extern void _erasecursor __P((void));
extern void _displaycursor __P((void));
#endif

extern void _putc __P((uchar c));
extern int tty_poll __P((void));

extern int tty_init __P((uchar minor));
extern int tty_read __P((uchar minor, uchar rawflag));
extern int tty_write __P((uchar minor, uchar rawflag));
extern int tty_open __P((uchar minor));
extern int tty_close __P((uchar minor));
extern int tty_ioctl __P((uchar minor, int, void *));

#endif

#ifdef NEED__DEVMISC
/* DEVMISC.C */

#ifdef MEM_DEV
extern int mem_read __P((uchar minor, uchar rawflag));
extern int mem_write __P((uchar minor, uchar rawflag));
#endif

extern int null_read __P((uchar minor, uchar rawflag));
extern int null_write __P((uchar minor, uchar rawflag));

extern int lpr_open __P((uchar minor));
extern int lpr_close __P((uchar minor));
extern int lpr_read __P((uchar minor, uchar rawflag));
extern int lpr_write __P((uchar minor, uchar rawflag));

#endif

#ifdef NEED__DEVIO
/* DEVIO.C */

/* release buffer */
#define brelse(bp)	bfree(bp, 0)

/* free buffer and mark them dirty for eventually writing */
#define bawrite(bp)	bfree(bp, 1)

/* shortcuts for block device and character device read/write and device open/close */
#define bdread(bp)	bdreadwrite(bp, 0)
#define bdwrite(bp)	bdreadwrite(bp, 1)
#define cdread(dev)	cdreadwrite(dev, 0)
#define cdwrite(dev)	cdreadwrite(dev, 1)
#define d_open(dev)	d_openclose(dev, 1)
#define d_close(dev)	d_openclose(dev, 0)

extern char *baddevmsg;

/* do all buffers free without flushing */
extern void bufinit(void);

extern void *bread __P((dev_t dev, blkno_t blk, uchar rewrite));
extern int bfree __P((bufptr bp, uchar dirty));
extern void *zerobuf __P((uchar waitfor));
extern void bufsync __P((void));
#if DEBUG > 0
extern void bufdump __P((void));
#endif

extern devsw_t *validdev __P((dev_t dev, char *msg));

extern int bdreadwrite __P((bufptr bp, uchar write));
extern int cdreadwrite __P((dev_t dev, uchar write));
extern int d_openclose __P((dev_t dev, uchar open));

extern int d_init __P((void));
extern int d_ioctl __P((dev_t dev, int request, void *data));

extern int ok __P((uchar minor));
extern int nogood __P((uchar minor));
extern int nogood_ioctl __P((uchar minor, int, void *));

extern int insq __P((queue_t *q, uchar c));
extern int remq __P((queue_t *q, uchar *cp));
extern int uninsq __P((queue_t *q, uchar *cp));
extern void clrq __P((queue_t *q));
extern int fullq __P((queue_t *q));
#endif

#ifdef NEED__FILESYS
/* FILESYS.C */

extern char *baddevmsg;

#define freefileentry(e)	((e) & 0x80)

extern fsptr findfs __P((dev_t devno));
extern int wr_inode __P((inoptr ino));
extern void i_ref __P((inoptr ino));
extern void i_free __P((dev_t devno, ino_t ino));
extern void i_deref __P((inoptr ino));
extern inoptr namei __P((char *name, inoptr *parent, uchar followsym));
extern inoptr srch_dir __P((inoptr wd, char *compname));
extern inoptr srch_mt __P((inoptr ino));
extern inoptr i_open __P((dev_t devno, ino_t ino));
extern int doclose __P((uchar uindex));
extern int ch_link __P((inoptr wd, char *oldname, char *newname, inoptr nindex));
extern char *filename __P((char *path));
extern int namecomp __P((uchar *n1, uchar *n2));
extern inoptr newfile __P((inoptr pino, char *name));
extern ino_t i_alloc __P((dev_t devno));
extern blkno_t blk_alloc __P((dev_t devno));
extern void blk_free __P((dev_t devno, blkno_t blk));
extern uchar oft_alloc __P((void));
extern void oft_deref __P((uchar of));
extern uchar uf_alloc __P((void));
extern bool_t isdevice __P((inoptr ino));
extern void f_trunc __P((inoptr ino));
extern void freeblk __P((dev_t dev, blkno_t blk, uchar level));
extern blkno_t bmap __P((inoptr ip, blkno_t bn, bool_t rwflg));
extern void validblk __P((dev_t dev, blkno_t num));
extern inoptr getinode __P((uchar uindex));
extern int getperm __P((inoptr ino));
extern void setftim __P((inoptr ino, uchar which));
extern int fmount __P((dev_t dev, inoptr ino, bool_t roflag));
extern void magic __P((inoptr ino));
extern void i_sync __P((void));
extern void fs_sync __P((void));
#endif

#ifdef NEED__MACHDEP
/* MACHDEP.C */

/* tempstack() MUST be inline in any case! */
#ifndef ORION_HOSTED
#ifdef __TURBOC__
#define tempstack()	(_AX = (uint)tmpstk+sizeof(tmpstk)-2, _SP = _AX)
#else	/* z80 */
#define __str3(x)	__STRING(x)
#define __str2(x)	__str3(x)
#define __str1(x)	__str2(x)
#if DEBUG > 1
#define tempstack()	asm("	global _tmpstk"); asm(" ld sp," __str1(TSTK))
#else
#define tempstack()	asm(" ld sp," __str1(TSTK))
#endif
#endif /* __TURBOC__ */
#endif /* ORION_HOSTED */

extern int Min __P((int a, int b));
extern void bcopy __P((void *src, void *dst, uint count));
extern void bzero __P((void *ptr, uint count));
extern void bfill __P((void *ptr, uchar val, uint count));

#ifndef di
#ifdef HTC

#ifdef ORION_HOSTED

#define PSWAP_OFFSET	6				/* p_swap offset within ptab_t structure - see unix.h */

#define xforkcontext _UZIXBASE+1c6h
#define xsavecontext _UZIXBASE+1c8h
#define xdoswapin    _UZIXBASE+21ch
#define xinitarg0	UZIXBASE+0x265
#define xinitarg1	UZIXBASE+0x267
#define xbininit	UZIXBASE+0x269
#define xconsole 	UZIXBASE+0x273

#define KERNEL_PAGE 2
#define _RET_PAGE  	 _COMMONMEM+1
#define _RET_ADDR  	 _COMMONMEM+2
#define _RET_SP  	 _COMMONMEM+4

#define dtsize	(*((uchar *)COMMON_MEM))		/* allways starting at COMMON_MEM */
#define ret_page (*((uchar *)(COMMON_MEM+1)))	/* page where to return when context continuing. Placement: ret_page,ret_addr ! */
#define ret_addr (*((uchar *)(COMMON_MEM+2)))	/* addr where to return when context continuing. Placement: ret_page,ret_addr ! */
#define ret_sp (*((uchar *)(COMMON_MEM+4)))		/* stack pointer with to return when context continuing  */
#define _UDATAADDR	 _COMMONMEM+7				/* see also UDATA_ADDR, udata */
#define FSTACK_BCKP	UDATA_STASH-0x0100
#define _FSTACKBCKP	_UDATASTASH-0100h			/* for store F-area content for swapped out process, in process pages*/

/* CP/M interbank routines */
#define BLDIR		0f201h	/* multybank  ldir    */
#define BJMP		0f204h	/* multybank  jump    */
#define BCALL		0f207h	/* multybank  call    */
#define BRET		0f20ah	/* multybank  ret     */
#define BLDB		0f20dh	/* multybank  getbyte */
#define BSTB		0f210h	/* multybank  setbyte */
/* #define MEI		0f2c1h  /* pseudo     ei      */
/* #define MDI		0f2deh  /* pseudo     di      */

/* memory constatns. Mapper: port 0F9h, 0f000h..0ffffh - shared mem, 00000h...0efffh - local mem*/
#define YINTVEC		((unsigned char *)0x0f331)		/* int 50Hz interbank vector - page:address */
#define PRC_IMSTACK	((unsigned char *)0x0effd)		/* stackpointer copy at a moment of int 50Hz*/
/* #define OLD_YINTVEC	((unsigned char *)0x0effa)		/* page:address, used only at kernel page   */
#define ISR_RET		0f2e3h
#define F3STACK		0f2b5h
#define _YINTVEC	0f331h							
#define _PRCIMSTACK	0effdh							/* for store SP at 50HZ interrupt moment, all pages */
/* #define _OLDYINTVEC 0effah							/* for store CP/M int50hz vector, in kernel page */
#define _GOUNIXSTACK 0effbh							/* for store process SP during kernel call, in process pages */
#define _GOSWAPSTACK 0eff9h							/* for store F-area SP for swapped out process and FSTACK while executing, in process pages */
#define _RETPAGE	 0eff8h
#define FAREA_STACK	0xf100
#define _FAREASTACK	0f100h
#define _PRC_STACK1	0f0feh				/* F3_1 area to store SP value at moment of first unix() entrance - usually SP at 0F3xx area */
#define _PRC_STACK2	0f0fch				/* F3_2 area to store SP value at moment of second (recursive) unix() entrance - usually SP at 0F3xx area */
#define	_TRP_STACK	0f0fah				/* F0   area to store SP value at moment of calltrap() - can be resursive unix() in it */

#define _SRV_STACK	0edf6h				/* TEMPDBUF-6   , in kernel page only */
/* #define TSTK	 	0ed76h				/* TEMPDBUF-128 , in kernel page only */

#define	U_INSYS  DIRNAMELEN+2			/* True if in kernel now */
#define	U_CALLNO U_INSYS+2				/* syscall being executed */
#define	U_ERROR  U_CALLNO+1				/* Last error number */
#define	U_ARGN1	 U_CALLNO+2				/* First arg */
#define	U_ARGN2	 U_ARGN1+2				/* Second arg */
#define	U_ARGN3	 U_ARGN2+2				/* Third arg */
#define	U_ARGN4	 U_ARGN3+2				/* Fourth arg (only for lseek) */
#define	U_RETVAL U_ARGN4+2				/* Return value from syscall */

extern void B_COPY(void *src, void *dst, unsigned int count);
extern void B_LDIR(unsigned int count, unsigned char bnksrc, void *src, unsigned char bnkdst, void *dst);
extern void B_LDIRTON(unsigned int count, void *src, void *dst);	/* send block from KERNEL_PAGE to process page */
extern int B_LDIRTO2(unsigned int count, void *src, void *dst);		/* send block from process page to KERNEL_PAGE, returns (int)dst */

extern void calltrap __P((void));

#ifdef ORI_UTIL
#define _di()
#define _ei()
#define __ei()
#endif

#else

#define _di()	asm("	di")
#define _ei()	{ if (!inint) asm("	ei"); }
#define __ei()	 asm("	   ei")

#endif   /* ORION_HOSTED */

#else	/* HTC */
extern void _di __P((void));
extern void _ei __P((void));

#endif	/* HTC */
#endif  /* di */

#ifndef sttime
#define sttime(x)	wrtod(x)
#endif

#ifdef __KERNEL__
#define kputchar 	_putc
#else
#define kprintf		printf
#define kputchar(x) printf("%c", x)
#endif

extern void arch_init __P((void));
#ifdef __KERNEL__
extern void _abort __P((int status));
#else
#define _abort(v) exit(v)
#endif
extern void initsys __P((void));
extern void setvectors __P((void));
#ifdef ORI_UZIX
extern void doexec __P((uint argc, uint argv, uint envp, uint start));
#define	rdtod()
#else
extern void doexec __P((uint sp));
extern void rdtod __P((void));
#endif
extern int valadr __P((void *base, uint size));
extern void addtick __P((time_t *t1, time_t *t2));
extern void incrtick __P((time_t *t));
extern void rdtime __P((time_t *tloc));
/*#ifdef NO_RTC
extern void updttod __P((void));
#endif*/
extern void wrtod __P((time_t *tloc));
#ifdef __KERNEL__
extern char *itoa __P((int, char *s, int));
extern void kputs __P((char *s));
#endif
#ifdef __TURBOC__
#ifdef __KERNEL__
extern void kprintf __P((char *fmt, ...));
#endif
extern void panic __P((char *s, ...));
/* extern void panic __P((char* s, void* a1, void* a2, void* a3)); */

extern void warning __P((char *s, ...));
#else
#ifdef __KERNEL__
extern void kprintf () /*__P((char *fmt, ...))*/ ;
#endif
extern void panic __P((char *s, ...));
extern void warning __P((char *s, ...));
#endif
#if DEBUG > 0
extern void idump __P((void));
#endif

#endif

#ifdef NEED__PROCESS
/* PROCESS.C */

extern void init __P((void));
#ifdef __KERNEL__
extern void psleep __P((void *event));
extern void wakeup __P((void *event));
extern void swapout __P((void));	/* used in MACHDEP.unix() */
extern int dowait __P((void));
extern int dofork __P((void));
extern void doexit __P((uchar val, uchar val2));
extern void clk_int __P((void));
extern void chksigs __P((void));
extern void sendsig __P((ptptr proc, signal_t sig));
extern void ssig __P((ptptr proc, signal_t sig));
#else /* __KERNEL__ */
#define psleep(p)
#define wakeup(p)
#endif /* __KERNEL__ */

#endif

#ifdef NEED__DISPATCH
/* dispatch.c */

extern int (*disp_tab[]) __P((void));
/*
extern uchar dtsize;
*/
#endif

extern void readwritei __P((char write, inoptr ino));
#define readi(x) 	readwritei(0, x)
#define writei(x)	readwritei(1, x)

#ifdef NEED__SCALL
/* SCALL.C */

extern void exit0 __P((void));
extern int getfsys __P((dev_t devno, void *buf));

extern int sys_NONE __P((void));
extern int sys_open __P((void));
extern int sys_close __P((void));
extern int sys_pipe __P((void));
extern int sys_link __P((void));
extern int sys_symlink __P((void));
extern int sys_unlink __P((void));
extern int sys_readwrite __P((void));
extern int sys_lseek __P((void));
extern int sys_chdir __P((void));
extern int sys_chroot __P((void));
extern int sys_mknod __P((void));
extern int sys_sync __P((void));
extern int sys_access __P((void));
extern int sys_chmod __P((void));
extern int sys_chown __P((void));
extern int sys_statfstat __P((void));
extern int sys_dup __P((void));
extern int sys_dup2 __P((void));
extern int sys_getset __P((void));
extern int sys_getfsys __P((void));
extern int sys_ioctl __P((void));
extern int sys_mountumount __P((void));
extern int sys_time __P((void));
extern int sys_stime __P((void));
extern int sys_times __P((void));
extern int sys_execve __P((void));
extern int sys_brk __P((void));
extern int sys_sbrk __P((void));
extern int sys_waitpid __P((void));
extern int sys_exit __P((void));
extern int sys_fork __P((void));
extern int sys_pause __P((void));
extern int sys_signal __P((void));
extern int sys_kill __P((void));
extern int sys_alarm __P((void));
extern int sys_utime __P((void));
extern int sys_reboot __P((void));
#ifdef UZIX_MODULE
extern void clear_module_reqs __P((int pid));
extern int sys_reg __P((void));
extern int sys_dereg __P((void));
extern int sys_callmodu __P((void));
extern int sys_repfmodu __P((void));
extern int sys_modreply __P((void));
#endif

#endif

#endif
