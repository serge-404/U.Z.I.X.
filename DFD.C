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
 Floppy disk drivers manager (real FD and file mapped)
**********************************************************/

#define NEED__DEVFLOP
/* #define NEED__MACHDEP */

#include "uzix.h"
#ifdef SEPH
#include "types.h"
#include "signal.h"
#include "ioctl.h"
#endif
#include "unix.h"

#undef NEED__MACHDEP
#undef NEED__DISPATCH
#undef NEED__DEVTTY
#undef NEED__DEVMISC
#undef NEED__FILESYS
#undef NEED__SCALL
#undef NEED__DEVIO
#undef NEED__DEVSWAP
#undef NEED__PROCESS

#include "extern.h"

#ifdef BCB_COMPILER
#include <stdio.h>
#include <dir.h>
#endif /* BCB_COMPILER */

#ifdef ORION_HOSTED
#include "dsk.h"
#endif

/* If __ONFILE defined then floppies will be simulated on files
 * __ONFILE"minor"
 * This option mainly for MS-DOS (or for Z80 emulator under MS-DOS)!
 */
#ifdef PC_HOSTED
#define __ONFILE	"..\\floppy"
#else
#ifdef Z80MU
/* name must be in FCB form, without point and in capital letters only */
#define __ONFILE	"FLOP"
#endif
#endif

/* Floppy disk sector size */
#define SEC_SIZE	512

#ifdef __ONFILE
/* this structure contains all info needed for floppy disk access */
typedef struct {
	uchar 	mediaid;	/* opened file descriptor */
#ifdef BCB_COMPILER
	void* 	fHandle;	/* opened file descriptor */
        unsigned long fOffset;  /* partition offset into image file */
        unsigned long fParSize; /* partition size in 512b blocks */
#endif  /*  BCB_COMPILER */
	uint	size;		/* pseudo floppy size in blocks */
} fdinfo_t;

#ifdef BCB_COMPILER
unsigned char fBuff[512];
#endif /* BCB_COMPILER */

#else /* __ONFILE -- real floppies */

#ifdef MSX_HOSTED
/* this structure contains all info needed for floppy disk access */
typedef struct {
	uchar	mediaid;	/* floppy media ident */
	uint	size;		/* floppy size in blocks */
} fdinfo_t;

STATIC uchar fmediaid;		/* media id for current operation */

STATIC uchar medias[2] = {0xF9, 0xF0};	/* possible disk types */

#endif /* MSX_HOSTED */

#ifdef PC_HOSTED
/* this structure contains all info needed for floppy disk access */
typedef struct {
	uchar	mediaid;	/* media id - for compatibility */
	uint	tracks;		/* # of tracks */
	uchar	heads;		/* # of heads */
	uchar	sectors;	/* # of sectors per track */
	uint	size;		/* floppy size in blocks */
} fdinfo_t;

STATIC uint ftrack;		/* track # for current operation */
STATIC uchar
	fhead,			/* head # for current operation */
	fsector;		/* sector # for current operation */
#endif /* PC_HOSTED */

#endif /* __ONFILE */

/* in this table we keep the current floppy disks configuration
 * -1 as mediaid means 'floppy is not opened'
 */
#ifdef ORION_HOSTED
extern PARTITION Drives[MAXDRIV];
#define fdinfo Drives
#else
STATIC fdinfo_t fdinfo[MAXDRIV] = {
	{ (uchar)-1},
	{ (uchar)-1}
};
#endif

STATIC uchar fdrive;		/* drive for current operation */
STATIC uint firstblk; 		/* first block for current operation */
STATIC uint nbytes; 		/* byte counter for -//- */
STATIC uint nblocks;		/* number of block accessed in -//- */
STATIC int xferror;		/* returned error */
STATIC uchar *fbuf;		/* buffer pointer */
STATIC uchar fpage;

STATIC int fd __P((uchar rdflag, uchar minor, uchar rawflag));
STATIC int dskread __P((void));
STATIC int dskwrite __P((void));
#ifdef __ONFILE
STATIC int closeonfile __P((void));
STATIC int openonfile __P((char *, int));
#else
#ifndef ORION_HOSTED
STATIC void reset __P((void));
#endif
#ifdef MSX_HOSTED
STATIC void chkfdrive __P((void));
#endif
#endif

/* Includes for low-level access -- machine dependand */
#ifdef PC_HOSTED
#include "devflop.mtc"
#else	/* PC_HOSTED */
#ifdef MSX_HOSTED
/* MSX machine */
#include "devflop.msx"
#else /* MSX_HOSTED */
#include "devflop.orn"    /* ORION_HOSTED */
#endif	/* MSX_HOSTED */
#endif	/* PC_HOSTED */

/* common routine for read/write floppies */
STATIC int fd(rdflag, minor, rawflag)
	uchar rdflag;		/* !0 if read, 0 if write */
	uchar minor;		/* floppy number */
	uchar rawflag;		/* 0 - buffered, !0 - raw */
{
#ifndef __ONFILE
	/*** real floppy ***/
#ifndef MSX_HOSTED
#ifndef ORION_HOSTED
	fdinfo_t *fi = fdinfo+minor;
	uchar retry = 3;
#else
	uchar retry = 2;	/* because no needed for IDE */
#endif /* ORION_HOSTED */
#else
	uchar retry = 10;	/* MSX disk-drives are too sensible */
#endif /* MSX_HOSTED */
#endif /* __ONFILE */

#ifndef ORION_HOSTED
	minor &= (MAXDRIV-1);
#else
	minor &= 15;
	if (minor>=MAXDRIV) minor=0;
#endif
	if (fdinfo[(uint)minor].mediaid == (uchar)-1)
		goto Err;	/* floppy is not opened yet */
	/* For raw operation we use the buffer/counter/block
	 * parameters from the user area.
	 * For block operation we use information from u_buf
	 * and must read only one block.
	 */
	if (rawflag) {
#ifdef ORI_UZIX
		fbuf = UDATA(u_base);
		fpage = UDATA(u_ptab)->p_swap;
		nbytes = UDATA(u_count);
		firstblk = (uint)(UDATA(u_offset) >> BUFSIZELOG);
#else
		panic("fd: rawflag");
#endif
	}
	else {
		fbuf = UDATA(u_buf)->bf_data;
		fpage = 0;
		nbytes = BUFSIZE;
		firstblk = UDATA(u_buf)->bf_blk;
	}
#if BUFSIZELOG > 8
	nblocks = (uchar)((nbytes + (BUFSIZE - 1)) >> 8) >> (BUFSIZELOG-8);
#else
	nblocks = (nbytes + (BUFSIZE - 1)) >> BUFSIZELOG;
#endif
	/* check against the floppy limits */
	if (firstblk >= fdinfo[(uint)minor].size) {			/* HERE !!! */
		/* trying to read non-existent sector: "EOF"
		 * trying to write to a non-existent sector: I/O error */
		if (rdflag) UDATA(u_error) = EFAULT;
		goto Err;
	}
	if (firstblk + nblocks > fdinfo[(uint)minor].size) {
		/* request is out of bounds -- correct request parameters */
		nblocks = fdinfo[minor].size - firstblk;
		/* nblocks is never negative, since firstblk<size */
		nbytes = nblocks << BUFSIZELOG;
		UDATA(u_count) = nbytes;
	}
#ifdef __ONFILE
	if (nblocks > 0) {
		fdrive = fdinfo[minor].mediaid;
		if ((xferror = rdflag ? dskread() : dskwrite()) != 0) {
			/* temporary patch -- only as debugging aid */
			kprintf("fd_%s: error %x at blk %d (total %d)\n",
			    rdflag ? "read":"write", xferror & 0xFF,
			    firstblk, nblocks);
			panic("stop");
		}
	}
#else /* __ONFILE -- real floppies */
#ifdef MSX_HOSTED
	fdrive = minor;
	chkfdrive();
	if (fdrive == (uchar)-1)
		goto Err;
	fmediaid = fdinfo[(uint)minor].mediaid;
	/*some MSX with hardware stopdrive  must
	  have drive reseted before access or it
	  it  will  return  a 'not ready'  error*/
	if (*(uchar *)CNFGBYTE & 1) reset();
	while (nblocks > 0) {
		if ((xferror = rdflag ? dskread() : dskwrite()) != 0) {
			kprintf("fd_%s: error ", rdflag ? "read":"write");
			if ((rdflag == 0) && (xferror == 1)) { /* write-protect error */
				kprintf("write protection\n");
				goto Err;
			}
			kprintf("%x, sector %d (id %x)\n",
				(xferror & 0xFF) - 1,
				firstblk, fmediaid);
			if (retry-- == 0)
				break;
			reset();
			continue;
		}
		fbuf += SEC_SIZE;
		firstblk++;	/* is this right? no prblem detected yet... */
		--nblocks;
	}
#else /* MSX_HOSTED -- */
#ifdef ORION_HOSTED			/* ORION HDD PARTITIONS */
	fdrive = minor;
	while (nblocks > 0) {
		if ((xferror = (rdflag ? dskread() : dskwrite())) != 0) {
			kprintf("fd_%s: error ", rdflag ? "read":"write");
			if ((rdflag == 0) && (STA_PROTECT & xferror == STA_PROTECT)) { /* write-protect error */
				kprintf("write protection\n");
				goto Err;
			}
			kprintf("%x, sector %d (id %x)\n",
				(xferror & 0xFF) - 1,
				firstblk, fdrive);
			if (retry-- == 0)
				break;
			continue;
		}
		fbuf += SEC_SIZE;
		firstblk++;	/* is this right? no prblem detected yet... */
		--nblocks;
	}
#ifdef PC_HOSTED /* PC floppies */
	fdrive = minor;
	ftrack = firstblk / (fi->sectors*fi->heads);
	firstblk %= (fi->sectors*fi->heads);
	fhead = firstblk / fi->sectors;
	fsector = firstblk % fi->sectors + 1;
	while (nblocks > 0) {
		if ((xferror = rdflag ? dskread() : dskwrite()) != 0) {
			kprintf("fd_%s: error %x at HCS %d/%d/%d\n",
			    rdflag ? "read":"write", xferror & 0xFF,
			    fhead, ftrack, fsector);
			if (retry-- == 0)
				break;
			reset();
			continue;
		}
		if (++fsector > fi->sectors) {
			fsector = 1;
			if (++fhead >= fi->heads) {
				fhead = 0;
				++ftrack;
			}
		}
		fbuf += SEC_SIZE;
		--nblocks;
	}
#endif /* ORION_HOSTED */
#endif /* PC_HOSTED */
#endif /* MSX_HOSTED */
#endif /* __ONFILE */
	if (xferror != 0) {
Err:		return -1;
	}
	if (rawflag) return	nbytes;
	return 0;
}

#ifdef __ONFILE
extern char DriveImage[MAXDRIV][MAXPATH];
#endif

/* open device - checks for floppy geometry */
GBL int fd_open(minor)
	uchar minor;
{
#ifdef __ONFILE
	char* name;   /* [80], *p = __ONFILE; */
/*	int i;  */

	minor &= (MAXDRIV-1);
	if (fdinfo[(uint)minor].mediaid != (uchar)-1)
		goto Err;		/* device is already opened */
	/* create the 'floppy-file' name */
/*	for (i = 0; p[i]; ++i)
		name[i] = p[i];
	name[i++] = minor+'0';
	name[i] = 0; */

        name = DriveImage[minor];

	return openonfile(name, minor);
#else /* __ONFILE -- real floppy */
#ifdef MSX_HOSTED
	uchar retr = 6;

	minor &= (MAXDRIV-1);
	if (fdinfo[(uint)minor].mediaid != (uchar)-1)
		goto Err;		/* device is already opened */
	fdrive = (uchar)minor;
	chkfdrive();
	if (fdrive == (uchar)-1)
		goto Err;
	fbuf = (uchar *)TEMPDBUF;
	firstblk = 0;
	while (--retr != 0) {
		fmediaid = medias[retr & 1];
		reset();
		if (dskread() == 0) {
			/* get real media id */
			fdinfo[minor].mediaid = ((uchar *)TEMPDBUF)[0x15];
			fdinfo[minor].size = ((uint *)(TEMPDBUF+0x13))[0];
			return 0;
		}
	}
#else /* MSX_HOSTED */
#ifdef ORION_HOSTED
	minor &= 15;
	if (minor>=MAXDRIV) minor=0;
	if (fdinfo[(uint)minor].mediaid != (uchar)-1)
		goto Err;		/* device is already opened */
	fdrive = (uchar)minor;
	fbuf = (uchar *)TEMPDBUF;
	firstblk = 0;
	if (dskread() == 0) {
		/* get real media id */
		fdinfo[minor].mediaid = (uchar)minor;   /* ((uchar *)TEMPDBUF)[0x15]; */
		/* fdinfo[minor].size = ((uint *)(TEMPDBUF+0x13))[0]; */
		return 0;
	}
#else /* ORION_HOSTED */
#ifdef PC_HOSTED       /* PC floppies */
	static fdinfo_t tfi[] = {
		{ 0, 80, 2, 18, 2880 },	/* 1.44M */
		{ 0, 80, 2, 15, 2400 },	/* 1.2M */
		{ 0, 80, 2, 9, 1440 },	/* 720K */
		{ 0, 40, 2, 9, 720 },	/* 360K */
	};
	uchar buf[SEC_SIZE];	/* possible problem: stack is too deep? */
	fdinfo_t *fi = tfi;

	minor &= (MAXDRIV-1);
	if (fdinfo[(uint)minor].mediaid != (uchar)-1)
		return -1;		/* device is olready opened */
	fdrive = (uchar)minor;
	fbuf = buf;
	while (fi < tfi+sizeof(tfi)/sizeof(fdinfo_t)) {
		uchar retr = 3;

		/* try to read the last sector of the track 40/0 */
		fhead = fi->heads-1;
		fsector = fi->sectors;
		ftrack = fi->tracks-40;
		fdinfo[minor].size = fi->size;
		while (--retr != 0) {
			reset();
			if (dskread() == 0) {
				/* accept this geometry if success */
				fdinfo[minor] = *fi;
				return 0;
			}
		}
		++fi;
	}
#else /* ORION_HOSTED -- */

#endif /* PC_HOSTED */
#endif /* MSX_HOSTED */
#endif /* __ONFILE */
Err:	return -1;
}

/* close device */
GBL int fd_close(minor)
	uchar minor;
{
#ifdef __ONFILE
	fdrive = minor;
	closeonfile();
#endif /* __ONFILE */
	fdinfo[(uint)minor].mediaid = (uchar)-1;
	return 0;
}

/* read data from device */
GBL int fd_read(minor, rawflag)
	uchar minor;
	uchar rawflag;
{
	return fd(1, (uchar)minor, (uchar)rawflag);
}

/* write data to device */
GBL int fd_write(minor, rawflag)
	uchar minor;
	uchar rawflag;
{
	return fd(0, (uchar)minor, (uchar)rawflag);
}

/* init device */
GBL int fd_init(minor)
	uchar minor;
{
	fdinfo[(uint)minor].mediaid = (uchar)-1;
#ifdef  BCB_COMPILER
    fdinfo[(uint)minor].fHandle = NULL;
    fdinfo[(uint)minor].fOffset = 0;
    fdinfo[(uint)minor].fParSize = 0;
	return 0;
#else
    return dskinit(minor);
#endif /* BCB_COMPILER */
}

#if 0
/* do ioctl on device - return error now */
GBL int fd_ioctl(minor, req, ptr)
	uchar minor;
	int req;
	void *ptr;
{
	NOTUSED(req);
	NOTUSED(ptr);
	NOTUSED(minor);
	return -1;
}
#endif


