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
 UZIX utilities:

 Block dump: to examine disk.
 Usage:	bd d: blkstart [blkend]
**********************************************************/

/* The device is given as drive letter.
 * Major device is always 0 (FDD), minor device is 0..7 (drives A..H)
 */

#define __MAIN__COMPILATION
#define NEED__DEVIO
#define NEED__SCALL
#define XFS

/*#include "utildos.h"*/

#include "uzix.h"
#ifdef SEPH
#include "types.h"
#include "signal.h"
#include "errno.h"
/* #error sdsdsd */
#endif
#include "unix.h"

#ifdef SEPH
#include "sys\ioctl.h"
#endif

#ifdef _MSX_DOS

#include "..\include\stdio.h"
#include "..\include\stdlib.h"
#include "..\include\ctype.h"

#else

#include "dsk.h"
#include "orion.h"

extern void bufinit(void);
extern void *bread(dev_t dev, blkno_t blk,	uchar rewrite);
#define d_open(dev)	d_openclose(dev, 1)
#define d_close(dev) d_openclose(dev, 0)
extern int d_openclose(dev_t dev, uchar open);
extern int d_init(void);

extern blkbuf_t bufpool[NBUFS];

extern int	printf(char *, ...);
extern long	atol(char *);
extern void	exit(int);
extern char  bios(int, ...);
#define xexit(n) exit(n)

extern unsigned char TotalDrives;
extern PARTITION Drives[];

char TEMPDBUF[BUFSIZE];

void list_drives()
{
int i;

printf("\n%d total drives.  Partitions:\n\nDev\tDrive#\tPart#\tSize,kb\tOffset Checksum Type\n", TotalDrives);

for (i=0; i<MAXDRIV; i++) {
    printf("fd%d\t%d\t%d\t%lu\t%lu\t%d\t%s (%d)\n", i,
      (int)(Drives[i].pd),	 /* Get physical drive# */
	   (Drives[i].pt),	 /* Get partition# */
	   (Drives[i].size/2),	 /* Get partition size */
	   (Drives[i].po),	 /* Get partition offset */
      (int)(Drives[i].pc),	 /* Get partition record checksumm */
      (Drives[i].ptype==82 ? "CP/M" : (Drives[i].ptype==33 ? "UZIX" : (Drives[i].ptype==-1 ? "PHISICAL" : (!Drives[i].ptype ? "" : "OTHER")))),
	   (int)Drives[i].ptype); /* Get partition type */
  }
}
#endif

char buf[BUFSIZE];

void dread(int dev, uint blk, char *addr) {
	char *buf = bread(dev, blk, 0);
	if (buf == NULL) {
		printf("bd: device or drive error\n");
		xexit(1);
	}
/*	
printf("\ndread(dev=%d, blk=%d, buf=%04x, addr=%04x); TEMPDBUF=%04x\n",
       dev, blk, (unsigned int)&buf[0], (unsigned int)&addr[0], (unsigned int)&TEMPDBUF[0]);
*/
	bcopy(buf, addr, BUFSIZE);
	bfree((bufptr)buf, 0);
}

int isdig(char cc) {
  if ((cc>='0') && (cc<='9')) return 1; else return 0;
}

#ifdef ORI_UTIL

void c_hex(unsigned char cc) {
	cc<10 ? bios(4,cc+'0') : bios(4,cc+'A'-10);
}

void pr_hex(unsigned char ch) {
    c_hex(ch >> 4);
    c_hex(ch & 15);
}

void bc_hexdump(unsigned char *buf, int count) {
  int c;
  register int bc=0;
  char txt[17];
  for (c = 0; c < count; c++) {
    if (!bc) {
	  pr_hex(((unsigned int)&buf[c])>>8); 
	  pr_hex((unsigned char)&buf[c]);
	  bios(4,' ');
	}
	pr_hex(buf[c]);
	bios(4,' ');
    if (buf[c]>' ') 
      txt[bc]=buf[c];
    else 
      txt[bc]='.';
    bc++;
    if (bc == 16) {
      txt[16]=0;
      bc=0;
      printf(" %s\n", txt);
    }
  }
  if (bc) {
    for (c = bc; c < 16; c++) printf("   ");
    printf(" %s\n", txt);
  }
  printf("\n"); 
}
#endif

void main(argc, argv)
	int argc;
	char *argv[];
{
#ifdef _MSX_DOS
	int i, j, k, dev;
	long addr;
#else
	int dev;
#endif
	unsigned blkno, blkend;

#ifdef _MSX_DOS
	initenv();
#endif
	if (argc < 3 || !isdig(argv[1][0]) || (argv[1][1] != ':')) {
		printf("usage: bd d: blkstart [blkend|-blkno]\n");
		xexit(1);
	}
	dev = argv[1][0] - '0';
	blkno = (unsigned)atol(argv[2]);
	if (argc < 4)
		blkend = blkno;
	else {
		if (argv[3][0] == '-')
			blkend = blkno + (unsigned)atol(argv[3]+1) - 1;
		else	blkend = (unsigned)atol(argv[3]);
	}
#ifdef _MSX_DOS
	printf("Insert disk and press RETURN: ");
	while (getchar() != '\n')
		;
	cursor(0);
#endif
	bufinit();
	d_init();
#ifdef ORI_UTIL
	if (!TotalDrives) {   /* IDEBDOS check and devices fd0..fd9 configuring */
        printf("\nNo drives or IDEBDOS v2.x not installed\n");
        xexit(1);
    }
#endif

list_drives();

	d_open(dev);
	printf("\nDevice %x, blocks %u..%u\n", dev, blkno, blkend);
#ifdef _MSX_DOS
	addr = (long)blkno * BUFSIZE;	/* starting address */
#endif
	while (blkno <= blkend) {
		printf("\nBlock %u = 0x", blkno);
		pr_hex(blkno>>8); 
		pr_hex((unsigned char)blkno);
		printf("\n");
		dread(dev, blkno++, buf);
#ifdef ORI_UTIL
		bc_hexdump((void*)buf, BUFSIZE);
#else
		for (i = 0; i < BUFSIZE / 16; ++i, addr += 16) {
			printf("%02x%04x\t", (uint)(addr >> 16), (uint)addr);
			for (j = 0; j < 16; ++j) {
				k = buf[(i<<4)+j] & 0xFF;
				printf("%02X ", k);
			}
			printf(" |");
			for (j = 0; j < 16; ++j) {
				k = buf[(i<<4)+j] & 0xFF;
				if (k < ' ' || k == 0177 || k == 0377)
					k = '.';
				printf("%c", k);
			}
			printf("|\n");
		}
		printf("\n");
		fflush(stdout);
#endif
	}
#ifdef _MSX_DOS
	cursor(255);
#endif
	exit(0);
}

