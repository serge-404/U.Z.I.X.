/*--------------------------------------------------------------------------/
/  FatFs - FAT file system module include file  R0.04b       (C)ChaN, 2007
/---------------------------------------------------------------------------/
/ FatFs module is an experimenal project to implement FAT file system to
/ cheap microcontrollers. This is a free software and is opened for education,
/ research and development under license policy of following trems.
/
/  Copyright (C) 2007, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is no warranty.
/ * You can use, modify and/or redistribute it for personal, non-profit or
/   profit use without any restriction under your responsibility.
/ * Redistributions of source code must retain the above copyright notice.
/
/---------------------------------------------------------------------------*/

#ifndef _FATFS

#define _ONLY_MBR		/* serge 20101010 : for use with CPM fdisk */

#define _MCU_ENDIAN		1
/* The _MCU_ENDIAN defines which access method is used to the FAT structure.
/  1: Enable word access.
/  2: Disable word access and use byte-by-byte access instead.
/  When the architectural byte order of the MCU is big-endian and/or address
/  miss-aligned access is prohibited, the _MCU_ENDIAN must be set to 2.
/  If it is not the case, it can be set to 1 for good code efficiency. */

#define _FS_READONLY	0
/* Setting _FS_READONLY to 1 defines read only configuration. This removes
/  writing functions, f_write, f_sync, f_unlink, f_mkdir, f_chmod, f_rename
/  and useless f_getfree. */

#define _FS_MINIMIZE	0
/* The _FS_MINIMIZE option defines minimization level to remove some functions.
/  0: Full function.
/  1: f_stat, f_getfree, f_unlink, f_mkdir, f_chmod and f_rename are removed.
/  2: f_opendir and f_readdir are removed in addition to level 1.
/  3: f_lseek is removed in addition to level 2. */

/* Added by Serge - removed functions */
/* #define _FS_MINIMIZE_LSEEK */
/* #define _FS_MINIMIZE_TRUNC */
/* #define _FS_MINIMIZE_UTIME */
/* #define _FS_MINIMIZE_STAT */
/* Added by Serge*/

#define _DRIVES		8
/* Number of logical drives to be used. This affects the size of internal table. */

#define	_USE_MKFS	0
/* When _USE_MKFS is set to 1 and _FS_READONLY is set to 0, f_mkfs function is
/  enabled. */

#define _USE_FSINFO	0
/* To enable FSInfo support on FAT32 volume, set _USE_FSINFO to 1. */

#define	_USE_SJIS	1
/* When _USE_SJIS is set to 1, Shift-JIS code transparency is enabled, otherwise
/  only US-ASCII(7bit) code can be accepted as file/directory name. */

#define	_USE_NTFLAG	1
/* When _USE_NTFLAG is set to 1, upper/lower case of the file name is preserved.
/  Note that the files are always accessed in case insensitive. */


#include "integer.h"



/* Definitions corresponds to multiple sector size (not tested) */
#define	S_MAX_SIZ	512			/* Do not change */
#if S_MAX_SIZ > 512
#define	S_SIZ	(fs->s_size)
#else
#define	S_SIZ	512
#endif


/* File system object structure */
typedef struct _FATFS {
	WORD	id;				/* File system mount ID */
	WORD	n_rootdir;		/* Number of root directory entries */
	DWORD	winsect;		/* Current sector appearing in the win[] */
	DWORD	sects_fat;		/* Sectors per fat */
	DWORD	max_clust;		/* Maximum cluster# + 1 */
	DWORD	fatbase;		/* FAT start sector */
	DWORD	dirbase;		/* Root directory start sector (cluster# for FAT32) */
	DWORD	database;		/* Data start sector */
#if !_FS_READONLY
	DWORD	last_clust;		/* Last allocated cluster */
	DWORD	free_clust;		/* Number of free clusters */
#if _USE_FSINFO
	DWORD	fsi_sector;		/* fsinfo sector */
	BYTE	fsi_flag;		/* fsinfo dirty flag (1:must be written back) */
/* -serge */
	/* BYTE	pad2; */
/* serge */
#endif
#endif
	BYTE	fs_type;		/* FAT sub type */
/* +-serge */
	WORD /*BYTE*/ csize;		/* Number of sectors per cluster */
/* serge */
#if S_MAX_SIZ > 512
	WORD	s_size;			/* Sector size */
#endif
/* +-serge */
	WORD /*BYTE*/	n_fats;		/* Number of FAT copies */
/* serge */
	BYTE	drive;			/* Physical drive number */
	BYTE	winflag;		/* win[] dirty flag (1:must be written back) */
/* -serge */
	/* BYTE	pad1; */
/* serge */
	BYTE	win[S_MAX_SIZ];	/* Disk access window for Directory/FAT */
} FATFS;


/* Directory object structure */
typedef struct _DIR {
	WORD	id;			/* Owner file system mount ID */
/* +serge */
	BYTE    pad3;
/* serge */
	WORD	index;		/* Current index */
	FATFS*	fs;			/* Pointer to the owner file system object */
	DWORD	sclust;		/* Start cluster */
	DWORD	clust;		/* Current cluster */
	DWORD	sect;		/* Current sector */
} DIR;


/* File object structure */
typedef struct _FIL {
	WORD	id;				/* Owner file system mount ID */
	BYTE	flag;			/* File status flags */
/* +-serge */
	WORD /*BYTE*/ csect;			/* Sector address in the cluster */
/* serge */
	FATFS*	fs;				/* Pointer to the owner file system object */
	DWORD	fptr;			/* File R/W pointer */
	DWORD	fsize;			/* File size */
	DWORD	org_clust;		/* File start cluster */
	DWORD	curr_clust;		/* Current cluster */
	DWORD	curr_sect;		/* Current sector */
#if _FS_READONLY == 0
	DWORD	dir_sect;		/* Sector containing the directory entry */
	BYTE*	dir_ptr;		/* Ponter to the directory entry in the window */
#endif
	BYTE	buffer[S_MAX_SIZ];	/* File R/W buffer */
} FIL;


/* File status structure */
typedef struct _FILINFO {
	DWORD fsize;			/* Size */
	WORD fdate;			/* Date */
	WORD ftime;			/* Time */
	BYTE fattrib;			/* Attribute */
#ifdef ORI_UZIX
	WORD uattrib;			/* UZIX Attributes */
	char fname[DIRNAMELEN+1];	/* Name (UZIX format, DIRNAMELEN=14) */	
#else
	char fname[8+1+3+1];	/* Name (8.3 format) */
#endif
} FILINFO;



/* File function return code (FRESULT) */

typedef unsigned char	FRESULT;

#define	FR_OK 			0
#define	FR_NOT_READY		1
#define	FR_NO_FILE		2
#define	FR_NO_PATH		3
#define	FR_INVALID_NAME		4
#define	FR_INVALID_DRIVE	5
#define	FR_DENIED		6
#define	FR_EXIST		7
#define	FR_RW_ERROR		8
#define	FR_WRITE_PROTECTED	9
#define	FR_NOT_ENABLED		10
#define	FR_NO_FILESYSTEM	11
#define	FR_INVALID_OBJECT	12
#define	FR_MKFS_ABORTED		13



/*-----------------------------------------------------*/
/* FatFs module application interface                  */

FRESULT f_mount (BYTE, FATFS*);			/* Mount/Unmount a logical drive */
FRESULT f_open (FIL*, char*, BYTE);		/* Open or create a file */
FRESULT f_read (FIL*, void*, WORD, WORD*);	/* Read data from a file */
FRESULT f_write (FIL*, void*, WORD, WORD*);	/* Write data to a file */
#ifdef _FS_MINIMIZE_LSEEK
FRESULT f_lseek (FIL*, DWORD);			/* Move file pointer of a file object */
#endif
FRESULT f_close (FIL*);				/* Close an open file object */
FRESULT f_opendir (DIR*, char*);		/* Open an existing directory */
FRESULT f_readdir (DIR*, FILINFO*);		/* Read a directory item */
#ifdef _FS_MINIMIZE_STAT
FRESULT f_stat (char*, FILINFO*);		/* Get file status */
#endif
FRESULT f_getfree (char*, DWORD*, FATFS**);	/* Get number of free clusters on the drive */
#ifdef _FS_MINIMIZE_TRUNC
FRESULT f_truncate (FIL*);			/* Truncate file */
#endif
FRESULT f_sync (FIL*);				/* Flush cached data of a writing file */
FRESULT f_unlink (char*);			/* Delete an existing file or directory */
FRESULT	f_mkdir (char*);			/* Create a new directory */
FRESULT f_chmod (char*, BYTE, BYTE);		/* Change file/dir attriburte */
#ifdef _FS_MINIMIZE_UTIME
FRESULT f_utime (char*, FILINFO*);		/* Change file/dir timestamp */
#endif
FRESULT f_rename (char*, char*);		/* Rename/Move a file or directory */
/* FRESULT f_mkfs (BYTE, BYTE, WORD);		 Create a file system on the drive */


/* User defined function to give a current time to fatfs module */

DWORD get_fattime (void);	/* 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
							/* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */



/* File access control and file status flags (FIL.flag) */

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00
#if _FS_READONLY == 0
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#endif
#define FA__ERROR			0x80


/* FAT sub type (FATFS.fs_type) */
#define FS_UNKNOWN	0
#define FS_FAT12	1
#define FS_FAT16	2
#define FS_FAT32	3
#define FS_SECND_MBR	16
#define FS_IBMBOOT	17
#define FS_NTFS		18
#define FS_BSD		19
#define FS_EFI		20
#define FS_LINUX	21


/* File attribute bits for directory entry */

#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */



/* Offset of FAT structure members */

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec		11
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_NTFlags		38
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82

#define	FSI_LeadSig			0
#define	FSI_StrucSig		484
#define	FSI_Free_Count		488
#define	FSI_Nxt_Free		492

#define MBR_Table			446
#define MBR_PART_TYPE		4

#define	DIR_Name			0
#define	DIR_Attr			11
#define	DIR_NTres			12
#define	DIR_CrtTime			14
#define	DIR_CrtDate			16
#define	DIR_FstClusHI		20
#define	DIR_WrtTime			22
#define	DIR_WrtDate			24
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28

#define MIN_FAT16	4086	/* Minimum number of clusters for FAT16 */
#define	MIN_FAT32	65526	/* Minimum number of clusters for FAT32 */


/* Multi-byte word access macros  */

#if _MCU_ENDIAN == 1	/* Use word access */
#define	LD_WORD(ptr)		(WORD)(*(WORD*)(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(*(DWORD*)(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(WORD*)(BYTE*)(ptr)=(WORD)(val)
#define	ST_DWORD(ptr,val)	*(DWORD*)(BYTE*)(ptr)=(DWORD)(val)
#else
#if _MCU_ENDIAN == 2	/* Use byte-by-byte access */
#define	LD_WORD(ptr)		(WORD)(((WORD)*(BYTE*)((ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*(BYTE*)((ptr)+3)<<24)|((DWORD)*(BYTE*)((ptr)+2)<<16)|((WORD)*(BYTE*)((ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *(BYTE*)((ptr)+1)=(BYTE)((WORD)(val)>>8)
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *(BYTE*)((ptr)+1)=(BYTE)((WORD)(val)>>8); *(BYTE*)((ptr)+2)=(BYTE)((DWORD)(val)>>16); *(BYTE*)((ptr)+3)=(BYTE)((DWORD)(val)>>24)
#else
/* -serge */
  /* #error Do not forget to set _MCU_ENDIAN properly! */
/* serge */
#endif
#endif


#define _FATFS
#endif /* _FATFS */
