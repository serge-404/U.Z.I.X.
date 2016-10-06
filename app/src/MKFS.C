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

 Make Filesystem: to make an UZIX disk.
 Usage:	mkfs [-yqv] d: fsize isize [rsize]
**********************************************************/

/* The device is given as /dev/fdM.
 * Major device is always 0 (FDD), minor device (M) is 0..7 
 */

#define NEED__FSONLY
#define ORION_HOSTED
#define SEPH

#include "types.h"
#include "stat.h"
#include "unix.h"

#include "stdio.h"
/*#include "string.h"*/
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
 
#define	PF	printf
#define bzero(ar,sz) memset(ar,0,sz)

char bootblock[512] = {
	0xEB, 0xFE, 0x90, 'U',	'Z',  'I',  'X',  'd',
	'i',  's',  'k',  0x00, 0x02, 0x02, 0x01, 0x00,
	0x00, 0x00, 0x00, 0xA0, 0x05, 0xF9, 0x00, 0x00,
	0x09, 0x00, 0x02, 0x00, 0x00, 0x00, 0xD0, 0x36,
	0x56, 0x23, 0x36, 0xC0, 0x31, 0x1F, 0xF5, 0x11,
	0x4A, 0xC0, 0x0E, 0x09, 0xCD, 0x7D, 0xF3, 0x0E,
	0x08, 0xCD, 0x7D, 0xF3, 0xFE, 0x1B, 0xCA, 0x22,
	0x40, 0xF3, 0xDB, 0xA8, 0xE6, 0xFC, 0xD3, 0xA8,
	0x3A, 0xFF, 0xFF, 0x2F, 0xE6, 0xFC, 0x32, 0xFF,
	0xFF, 0xC7, 0x57, 0x41, 0x52, 0x4E, 0x49, 0x4E,
	0x47, 0x21, 0x07, 0x0D, 0x0A, 0x0A, 0x54, 0x68,
	0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x6E,
	0x20, 0x55, 0x5A, 0x49, 0x58, 0x20, 0x64, 0x69,
	0x73, 0x6B, 0x2C, 0x20, 0x6E, 0x6F, 0x6E, 0x20,
	0x62, 0x6F, 0x6F, 0x74, 0x61, 0x62, 0x6C, 0x65,
	0x2E, 0x0D, 0x0A, 0x55, 0x73, 0x69, 0x6E, 0x67,
	0x20, 0x69, 0x74, 0x20, 0x75, 0x6E, 0x64, 0x65,
	0x72, 0x20, 0x4D, 0x53, 0x58, 0x44, 0x4F, 0x53,
	0x20, 0x63, 0x61, 0x6E, 0x20, 0x64, 0x61, 0x6D,
	0x61, 0x67, 0x65, 0x20, 0x69, 0x74, 0x2E, 0x0D,
	0x0A, 0x0A, 0x48, 0x69, 0x74, 0x20, 0x45, 0x53,
	0x43, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x42, 0x41,
	0x53, 0x49, 0x43, 0x20, 0x6F, 0x72, 0x20, 0x61,
	0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74,
	0x6F, 0x20, 0x72, 0x65, 0x62, 0x6F, 0x6F, 0x74,
	0x2E, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* This makes a filesystem */
dev_t dev;
direct_t dirbuf[DIRECTPERBLOCK] = {
	{ ROOTINODE, "." },
	{ ROOTINODE, ".."}
};
dinode_t inode[DINODESPERBLOCK];

uchar _yes = 0; 	/* yes to all questions */
uchar _quick = 0;	/* quick initialization */
uchar _verb = 0;	/* be verbose */

static char dbuf[BUFSIZE];

void dwrite __P((uint, void *));
void dread __P((uint, void *));
int yes __P((char *));
void mkfs __P((uint, uint,uint));

void dwrite(blk, addr)
	uint blk;
	void *addr;
{
    if (lseek(dev, blk * (long)BUFSIZE, 0) == -1) {
        perror("lseek");
        exit(1);
    }
    if (write(dev, addr, BUFSIZE) != BUFSIZE) {
        perror("write");
        exit(1);
    }
}

void dread(blk, addr)
	uint blk;
	void *addr;
{
    if (lseek(dev, blk * (long)BUFSIZE, 0) == -1) {
        perror("lseek");
        exit(1);
    }
    if (read(dev, addr, BUFSIZE) != BUFSIZE) {
        perror("read");
        exit(1);
    }
}

int yes(char* p) {
    char line[20];
	PF(p);
	fflush(stdout);
	if (_yes)
		PF("Y\n");
    if (!fgets(line, sizeof(line), stdin))
		return (0);
	PF("\n");
	fflush(stdout);
	return ((*line=='y') || (*line=='Y'));
}

void mkfs(fsize, isize, rsize)
	uint fsize, isize, rsize;
{
	uint j, lm;
	char *buf;
	filesys_t fs;
	char zeroed[BUFSIZE];
	buf = &dbuf[0];
	
	dread(0, buf);

	if (_verb) {
		PF("Creating boot block...\n");	
	}
	/* Preserve disk data (number of sectors, format type, etc) */
	for (j = 11; j < 30; j++) bootblock[j] = buf[j];
	/* Preserve other relevant data (we just use first 256 bytes) */
	for (j = 256; j < 512; j++) bootblock[j] = buf[j];
	/* Write new boot block */
	bootblock[0x10] = rsize;
	dwrite(0,bootblock);
	/* Zero out the blocks */
	bzero(zeroed, BUFSIZE);
	if (_verb) {
		PF("Initializing inodes, be patient...\n"); 
	}
	if (_quick) {
		dwrite(fsize-1,zeroed); 	/* Last block */
		lm = SUPERBLOCK+1+rsize+isize;	/* Super+Reserv+Inodes */
	}
	else	lm = fsize;			/* All blocks of filesys */
	j = 1;
	while (j < lm) {
		if (_verb && j % 9 == 0) {
			PF("Blk #%d\r", j); 
		}
		dwrite(j++, zeroed);
	}
	if (_verb) {
		PF("Blk #%d\n",--j); 
	}
	/* Initialize the super-block */
	if (_verb) {
		PF("Creating super-block...\n"); 
	}
	bzero(&fs,sizeof(fs));
	fs.s_mounted = SMOUNTED;	/* Magic number */
	fs.s_reserv = SUPERBLOCK+1+rsize;
	fs.s_isize = isize;
	fs.s_fsize = fsize;
	fs.s_tinode = DINODESPERBLOCK * isize - 2;
	/* Free each block, building the free list */
	j = fsize - 1;
	while (j > SUPERBLOCK+1+rsize+isize) {
		if (fs.s_nfree == FSFREEBLOCKS) {
			dwrite(j, (char *) &fs.s_nfree);
			fs.s_nfree = 0;
			bzero(fs.s_free,sizeof(fs.s_free));
		}
		fs.s_tfree++;
		fs.s_free[fs.s_nfree++] = j--;
	}
	/* The inodes are already zeroed out */
	/* create the root dir */
	inode[ROOTINODE].i_mode = S_IFDIR | 0755;
	inode[ROOTINODE].i_nlink = 3;
	inode[ROOTINODE].i_size = sizeof(direct_t)*2;
	inode[ROOTINODE].i_addr[0] = SUPERBLOCK+1+rsize+isize;
	/* Reserve reserved inode */
	inode[0].i_nlink = 1;
	inode[0].i_mode = ~0;
	/* Free inodes in first inode block */
	j = ROOTINODE+1;
	while (j < DINODESPERBLOCK) {
		if (fs.s_ninode == FSFREEINODES)
			break;
		fs.s_inode[fs.s_ninode++] = j++;
	}
	dwrite(SUPERBLOCK+1+rsize, inode);
	dwrite(SUPERBLOCK+1+rsize+isize, dirbuf);
	/* Write out super block */
	dwrite(SUPERBLOCK, &fs);
}

void main(argc, argv)
	int argc;
	char *argv[];
{
	uint fsize, isize, rsize = 0, ap = 1;
	uchar *p;

	while (*(p = (uchar *)argv[ap]) == '-') {
		++p;
		++ap;
		--argc;
		while (*p) {
			switch (*p++) {
			case 'y': case 'Y':	_yes = 1;	break;
			case 'q': case 'Q':	_quick = 1;	break;
			case 'v': case 'V':	_verb = 1;	break;
			default:
				printf("Illegal switch %s\n", p);
				exit(1);
			}
		}
	}
	if (argc < 4) {
		/* parameters order agree with FSCK */
		printf("usage: mkfs [-yqv] /dev/fd# fsize isize [rsize]\n\t#=0..7, fsize>100, isize<fsize/30, rsize<100\n");
		exit(1);
	}
	fsize = (uint) atoi(argv[ap+1]);
	isize = (uint) atoi(argv[ap+2]);
	if (argc > 4)
		rsize = (uint) atoi(argv[ap+3]);
	if (fsize < 100 || isize >= fsize/30 || rsize > 100) {
		printf("Bad parameter values\n");
		exit(1);
	}
	PF("\nMaking filesystem on %s, fsize %u, isize %u, rsize %u. ",
	      argv[ap] , fsize, isize, rsize); 
	if (!yes("Confirm? "))
		exit(1);
	PF("\n");
	if ((int)(dev = open(argv[ap], O_RDWR))<0) {
		printf("Can't open device %s",argv[ap]);
		exit(1);
	}
	mkfs(fsize, isize, rsize);
	sync();
	if (_verb)
		PF("Filesystem on %s successfully initialized!\n", argv[ap]);
	exit(0);
}

