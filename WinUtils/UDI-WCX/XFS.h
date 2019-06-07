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
 UZIX system calls wrappers, used by UZIX utilities
**********************************************************/

#ifndef _XFS_H
#define _XFS_H

#include "uzix.h"
#include "unix.h"
#include "extern.h"
//#include <windows.h>

extern char *stringerr[];

extern int xfs_init __P((dev_t bootdev, uchar waitfordisk, int Panic, char* fname));
extern void xfs_end __P((dev_t bootdev));
extern int UZIXopen __P((char *name, int flag/*, ...*/));
extern int UZIXclose __P((int uindex));
extern int UZIXcreat __P((char *name, mode_t mode));
extern int UZIXlink __P((char *name1, char *name2));
extern int UZIXsymlink __P((char *name1, char *name2));
extern int UZIXunlink __P((char *path));
extern int UZIXread __P((int d, char *buf, uint nbytes));
extern int UZIXwrite __P((int d, char *buf, uint nbytes));
extern int UZIXseek __P((int file, uint offset, int flag));
extern int UZIXchdir __P((char *dir));
extern int UZIXmknod __P((char *name, mode_t mode, int dev));
extern void UZIXsync __P((void));
extern int UZIXaccess __P((char *path, int mode));
extern int UZIXutime __P((char *path, struct utimbuf *buf));
extern int UZIXchmod __P((char *path, mode_t mode));
extern int UZIXchown __P((char *path, int owner, int group));
extern int UZIXstat __P((char *path, void *buf));
extern int UZIXfstat __P((int fd, void *buf));
extern int UZIXdup __P((int oldd));
extern int UZIXdup2 __P((int oldd, int newd));
extern int UZIXumask __P((int mask));
extern int UZIXioctl __P((int fd, int request, void *data));
extern int UZIXmount __P((char *spec, char *dir, int rwflag));
extern int UZIXumount __P((char *spec));
extern int UZIXtime __P((void *tvec));

extern int UZIXgetfsys __P((dev_t dev, void *buf));

typedef struct {                // sizeof=32
          char Name[21];
          unsigned long Size;
          unsigned long Date;   // in format of FileGetDate() function
          unsigned char CRC;    // CRC66 byte
          unsigned char SIGN1;  // 0xAA
          unsigned char SIGN2;  // 0x55
} TSystemBinRec;

/* DiskParametersBlock (DPB) for UZIX floppy disks and HDD partitions */
typedef struct {                        // sizeof=30      (N/A means Not Applicable)
          unsigned char bootsig[3];     // boot signature
          char DiskName[8];             // disk name
          unsigned short SectorSize;    // sector size in bytes
          unsigned char ClusterSize;    // cluster size (in sectors)
          unsigned short Reserved;      // number of reserved sectors
          unsigned char nfat;           // number of FATs (N/A), on UZIX: reserved sectors for kernel
          unsigned short ndir;          // number of directory entries (N/A)
          unsigned short nsec;          // number of sectors on disk
          unsigned char diskID;         // disk ID
          unsigned short fatsize;       // FAT size in sectors (N/A)
          unsigned short nsectrk;       // number of sectors per track
          unsigned short nside;         // number of disk sides
          unsigned short nhidden;       // number of hidden sectors
} TDPBRec;                              // bootsig+30 - Start of boot program (must have up to 98 bytes) ;  bootsig+510 = 0AA55h - DOS boot sector signature

#endif
