/*--------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.04b                    (C)ChaN, 2007
/---------------------------------------------------------------------------/
/ The FatFs module is an experimenal project to implement FAT file system to
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
/---------------------------------------------------------------------------/
/  Feb 26, 2006  R0.00  Prototype.
/  Apr 29, 2006  R0.01  First stable version.
/  Jun 01, 2006  R0.02  Added FAT12 support.
/                       Removed unbuffered mode.
/                       Fixed a problem on small (<32M) patition.
/  Jun 10, 2006  R0.02a Added a configuration option (_FS_MINIMUM).
/  Sep 22, 2006  R0.03  Added f_rename().
/                       Changed option _FS_MINIMUM to _FS_MINIMIZE.
/  Dec 11, 2006  R0.03a Improved cluster scan algolithm to write files fast.
/                       Fixed f_mkdir() creates incorrect directory on FAT32.
/  Feb 04, 2007  R0.04  Supported multiple drive system.
/                       Changed some interfaces for multiple drive system.
/                       Changed f_mountdrv() to f_mount().
/                       Added f_mkfs().
/  Apr 01, 2007  R0.04a Supported multiple partitions on a plysical drive.
/                       Added a capability of extending file size to f_lseek().
/                       Added minimization level 3.
/                       Fixed an endian sensitive code in f_mkfs().
/  May 05, 2007  R0.04b Added a configuration option _USE_NTFLAG.
/                       Added FSInfo support.
/                       Fixed DBCS name can result FR_INVALID_NAME.
/                       Fixed short seek (<= csize) collapses the file object.
/---------------------------------------------------------------------------*/

#include <string.h>
#include "ff.h"			/* FatFs declarations */
#include "diskio.h"		/* Include file for user provided disk functions */

/*+serge*/

#if _MULTI_PARTITION != 0	/* Multiple partition cfg */

extern PARTITION Drives[];

#endif

BYTE PartType;

/*serge*/

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

FATFS *FatFs[_DRIVES];	/* Pointer to the file system objects (logical drives) */
WORD fsid;				/* File system mount ID */


/*-----------------------------------------------------------------------*/
/* Change window offset                                                  */
/*-----------------------------------------------------------------------*/

BOOL move_window (		/* TRUE: successful, FALSE: failed */
	fs,			/* File system object */
	sector			/* Sector number to make apperance in the fs->win[] */
)						/* Move to zero only writes back dirty window */
	register FATFS *fs;
	DWORD sector;
{
	DWORD wsect;


	wsect = fs->winsect;
	if (wsect != sector) {	/* Changed current window */
#if !_FS_READONLY

/*+-serge*/
		register WORD /*BYTE*/ n;
/*-serge*/

		if (fs->winflag) {	/* Write back dirty window if needed */
			if (disk_write(fs->drive, fs->win, wsect, 1) != RES_OK)
				return FALSE;
			fs->winflag = 0;
			if (wsect < (fs->fatbase + fs->sects_fat)) {	/* In FAT area */
				for (n = fs->n_fats; n >= 2; n--) {	/* Refrect the change to FAT copy */
					wsect += fs->sects_fat;
					disk_write(fs->drive, fs->win, wsect, 1);
				}
			}
		}
#endif
		if (sector) {
			if (disk_read(fs->drive, fs->win, sector, 1) != RES_OK)
				return FALSE;
			fs->winsect = sector;
		}
	}
	return TRUE;
}




/*-----------------------------------------------------------------------*/
/* Clean-up cached data                                                  */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
FRESULT sync (			/* FR_OK: successful, FR_RW_ERROR: failed */
	fs			/* File system object */
)
	register FATFS *fs;
{
	fs->winflag = 1;
	if (!move_window(fs, 0)) return FR_RW_ERROR;
#if _USE_FSINFO
	if (fs->fs_type == FS_FAT32 && fs->fsi_flag) {		/* Update FSInfo sector if needed */
		fs->winsect = 0;
		memset(fs->win, 0, 512);
		ST_WORD(&fs->win[BS_55AA], 0xAA55);
		ST_DWORD(&fs->win[FSI_LeadSig], 0x41615252);
		ST_DWORD(&fs->win[FSI_StrucSig], 0x61417272);
		ST_DWORD(&fs->win[FSI_Free_Count], fs->free_clust);
		ST_DWORD(&fs->win[FSI_Nxt_Free], fs->last_clust);
		disk_write(0, fs->win, fs->fsi_sector, 1);
		fs->fsi_flag = 0;
	}
#endif
	if (disk_ioctl(fs->drive, CTRL_SYNC, NULL) != RES_OK) return FR_RW_ERROR;
	return FR_OK;
}
#endif




/*-----------------------------------------------------------------------*/
/* Get a cluster status                                                  */
/*-----------------------------------------------------------------------*/

DWORD get_cluster (		/* 0,>=2: successful, 1: failed */
	FATFS *fs,			/* File system object */
	DWORD clust			/* Cluster# to get the link information */
)
{
	WORD wc;
	register WORD bc;
	DWORD fatsect;


	if (clust >= 2 && clust < fs->max_clust) {		/* Valid cluster# */
		fatsect = fs->fatbase;
		switch (fs->fs_type) {
		case FS_FAT12 :
			bc = (WORD)clust * 3 / 2;
			if (!move_window(fs, fatsect + (bc / S_SIZ))) break;
			wc = fs->win[bc & (S_SIZ - 1)]; bc++;
			if (!move_window(fs, fatsect + (bc / S_SIZ))) break;
			wc |= (WORD)fs->win[bc & (S_SIZ - 1)] << 8;
			return (clust & 1) ? (wc >> 4) : (wc & 0xFFF);

		case FS_FAT16 :
			if (!move_window(fs, fatsect + (clust / (S_SIZ / 2)))) break;
			return LD_WORD(&fs->win[((WORD)clust * 2) & (S_SIZ - 1)]);

		case FS_FAT32 :
			if (!move_window(fs, fatsect + (clust / (S_SIZ / 4)))) break;
			return LD_DWORD(&fs->win[((WORD)clust * 4) & (S_SIZ - 1)]) & 0x0FFFFFFF;
		}
	}

	return 1;	/* There is no cluster information, or an error occured */
}




/*-----------------------------------------------------------------------*/
/* Change a cluster status                                               */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
BOOL put_cluster (		/* TRUE: successful, FALSE: failed */
	FATFS *fs,			/* File system object */
	DWORD clust,		/* Cluster# to change */
	DWORD val			/* New value to mark the cluster */
)
{
	register WORD bc;
	BYTE *p;
	DWORD fatsect;


	fatsect = fs->fatbase;
	switch (fs->fs_type) {
	case FS_FAT12 :
		bc = (WORD)clust * 3 / 2;
		if (!move_window(fs, fatsect + (bc / S_SIZ))) return FALSE;
		p = &fs->win[bc & (S_SIZ - 1)];
		*p = (clust & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;
		bc++;
		fs->winflag = 1; 
		if (!move_window(fs, fatsect + (bc / S_SIZ))) return FALSE;
		p = &fs->win[bc & (S_SIZ - 1)];
		*p = (clust & 1) ? (BYTE)(val >> 4) : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));
		break;

	case FS_FAT16 :
		if (!move_window(fs, fatsect + (clust / (S_SIZ / 2)))) return FALSE;
		ST_WORD(&fs->win[((WORD)clust * 2) & (S_SIZ - 1)], (WORD)val);
		break;

	case FS_FAT32 :
		if (!move_window(fs, fatsect + (clust / (S_SIZ / 4)))) return FALSE;
		ST_DWORD(&fs->win[((WORD)clust * 4) & (S_SIZ - 1)], val);
		break;

	default :
		return FALSE;
	}
	fs->winflag = 1;
	return TRUE;
}
#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Remove a cluster chain                                                */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
BOOL remove_chain (		/* TRUE: successful, FALSE: failed */
	fs,			/* File system object */
	clust			/* Cluster# to remove chain from */
)
	register FATFS *fs;
	DWORD clust;
{
	DWORD nxt;


	while (clust >= 2 && clust < fs->max_clust) {
		nxt = get_cluster(fs, clust);
		if (nxt == 1) return FALSE;
		if (!put_cluster(fs, clust, 0)) return FALSE;
		if (fs->free_clust != 0xFFFFFFFF) {
			fs->free_clust++;
#if _USE_FSINFO
			fs->fsi_flag = 1;
#endif
		}
		clust = nxt;
	}
	return TRUE;
}
#endif




/*-----------------------------------------------------------------------*/
/* Stretch or create a cluster chain                                     */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
DWORD create_chain (	/* 0: no free cluster, 1: error, >=2: new cluster number */
	fs,			/* File system object */
	clust			/* Cluster# to stretch, 0 means create new */
)
	register FATFS *fs;
	DWORD clust;
{
	DWORD cstat, ncl, scl, mcl = fs->max_clust;


	if (clust == 0) {		/* Create new chain */
		scl = fs->last_clust;			/* Get suggested start point */
		if (scl == 0 || scl >= mcl) scl = 1;
	}
	else {					/* Stretch existing chain */
		cstat = get_cluster(fs, clust);	/* Check the cluster status */
		if (cstat < 2) return 1;		/* It is an invalid cluster */
		if (cstat < mcl) return cstat;	/* It is already followed by next cluster */
		scl = clust;
	}

	ncl = scl;				/* Start cluster */
	for (;;) {
		ncl++;							/* Next cluster */
		if (ncl >= mcl) {				/* Wrap around */
			ncl = 2;
			if (ncl > scl) return 0;	/* No free custer */
		}
		cstat = get_cluster(fs, ncl);	/* Get the cluster status */
		if (cstat == 0) break;			/* Found a free cluster */
		if (cstat == 1) return 1;		/* Any error occured */
		if (ncl == scl) return 0;		/* No free custer */
	}

	if (!put_cluster(fs, ncl, 0x0FFFFFFF)) return 1;		/* Mark the new cluster "in use" */
	if (clust != 0 && !put_cluster(fs, clust, ncl)) return 1;	/* Link it to previous one if needed */

	fs->last_clust = ncl;				/* Update fsinfo */
	if (fs->free_clust != 0xFFFFFFFF) {
		fs->free_clust--;
#if _USE_FSINFO
		fs->fsi_flag = 1;
#endif
	}

	return ncl;		/* Return new cluster number */
}
#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/

DWORD clust2sect (	/* !=0: sector number, 0: failed - invalid cluster# */
	fs,		/* File system object */
	clust		/* Cluster# to be converted */
)
	register FATFS *fs;
	DWORD clust;
{
	clust -= 2;
	if (clust >= (fs->max_clust - 2)) return 0;		/* Invalid cluster# */
	return clust * fs->csize + fs->database;
}




/*-----------------------------------------------------------------------*/
/* Move directory pointer to next                                        */
/*-----------------------------------------------------------------------*/

BOOL next_dir_entry (	/* TRUE: successful, FALSE: could not move next */
	DIR *dirobj			/* Pointer to directory object */
)
{
	DWORD clust;
	WORD idx;
	register FATFS *fs = dirobj->fs;


	idx = dirobj->index + 1;
	if ((idx & ((S_SIZ - 1) / 32)) == 0) {		/* Table sector changed? */
		dirobj->sect++;			/* Next sector */
		if (!dirobj->clust) {		/* In static table */
			if (idx >= fs->n_rootdir) return FALSE;	/* Reached to end of table */
		} else {					/* In dynamic table */
			if (((idx / (S_SIZ / 32)) & (fs->csize - 1)) == 0) {	/* Cluster changed? */
				clust = get_cluster(fs, dirobj->clust);		/* Get next cluster */
				if (clust < 2 || clust >= fs->max_clust)	/* Reached to end of table */
					return FALSE;
				dirobj->clust = clust;				/* Initialize for new cluster */
				dirobj->sect = clust2sect(fs, clust);
			}
		}
	}
	dirobj->index = idx;	/* Lower 4 bit of dirobj->index indicates offset in dirobj->sect */
	return TRUE;
}




/*-----------------------------------------------------------------------*/
/* Get file status from directory entry                                  */
/*-----------------------------------------------------------------------*/

#if _FS_MINIMIZE <= 1
void get_fileinfo (		/* No return code */
	FILINFO *finfo, 	/* Ptr to store the file information */
	BYTE *dir		/* Ptr to the directory entry */
)
{
	BYTE n, a;
	register BYTE c;
	char *p;


	p = &finfo->fname[0];
/* +-serge*/

#if _USE_NTFLAG <= 0
	a = 0 ;
#else
	a = dir[DIR_NTres];		/* NT flag */
#endif /* _USE_NTFLAG */

/* serge*/

	for (n = 0; n < 8; n++) {	/* Convert file name (body) */
		c = dir[n];
		if (c == ' ') break;
		if (c == 0x05) c = 0xE5;
		if (a & 0x08 && c >= 'A' && c <= 'Z') c += 0x20;
		*p++ = c;
	}
	if (dir[8] != ' ') {		/* Convert file name (extension) */
		*p++ = '.';
		for (n = 8; n < 11; n++) {
			c = dir[n];
			if (c == ' ') break;
			if (a & 0x10 && c >= 'A' && c <= 'Z') c += 0x20;
			*p++ = c;
		}
	}
	*p = '\0';

	finfo->fattrib = dir[DIR_Attr];					/* Attribute */
	finfo->fsize = LD_DWORD(&dir[DIR_FileSize]);	/* Size */
	finfo->fdate = LD_WORD(&dir[DIR_WrtDate]);		/* Date */
	finfo->ftime = LD_WORD(&dir[DIR_WrtTime]);		/* Time */
}
#endif /* _FS_MINIMIZE <= 1 */




/*-----------------------------------------------------------------------*/
/* Pick a paragraph and create the name in format of directory entry     */
/*-----------------------------------------------------------------------*/

char make_dirfile (			/* 1: error - detected an invalid format, '\0'or'/': next character */
	char **path,			/* Pointer to the file path pointer */
	char *dirname			/* Pointer to directory name buffer {Name(8), Ext(3), NT flag(1)} */
)
{
	BYTE n, t, a, b;
	register BYTE c;

	memset(dirname, ' ', 8+3);	/* Fill buffer with spaces */
	a = 0; b = 0x18;	/* NT flag */
	n = 0; t = 8;
	for (;;) {
		c = *(*path)++;
		if (c == '\0' || c == '/') {		/* Reached to end of str or directory separator */
			if (n == 0) break;
#if _USE_NTFLAG <= 0
			dirname[11] = 0;
#else
			dirname[11] = (a & b);
#endif
			return c;
		}
		if (c <= ' ' || c == 0x7F) break;		/* Reject invisible chars */
		if (c == '.') {
			if (!(a & 1) && n >= 1 && n <= 8) {	/* Enter extension part */
				n = 8; t = 11; continue;
			}
			break;
		}
#if _USE_SJIS > 0
		if ((c >= 0x81 && c <= 0x9F) ||			/* Accept S-JIS code */
		    (c >= 0xE0 && c <= 0xFC)) {
			if (n == 0 && c == 0xE5)		/* Change heading \xE5 to \x05 */
				c = 0x05;
			a ^= 1; goto md_l2;
		}
#endif
		if (c == '"') break;				/* Reject " */
		if (c <= ')') goto md_l1;			/* Accept ! # $ % & ' ( ) */
		if (c <= ',') break;				/* Reject * + , */
		if (c <= '9') goto md_l1;			/* Accept - 0-9 */
		if (c <= '?') break;				/* Reject : ; < = > ? */
		if (!(a & 1)) {	/* These checks are not applied to S-JIS 2nd byte */
			if (c == '|') break;			/* Reject | */
			if (c >= '[' && c <= ']') break;/* Reject [ \ ] */
#if _USE_NTFLAG > 0
			if (c >= 'A' && c <= 'Z')
				(t == 8) ? (b &= 0xF7) : (b &= 0xEF);
#endif
			if (c >= 'a' && c <= 'z') {		/* Convert to upper case */
				c -= 0x20;
#if _USE_NTFLAG > 0
				(t == 8) ? (a |= 0x08) : (a |= 0x10);
#endif
			}
		}
	md_l1:
		a &= 0xFE;
	md_l2:
		if (n >= t) break;
		dirname[n++] = c;
	}
	return 1;
}




/*-----------------------------------------------------------------------*/
/* Trace a file path                                                     */
/*-----------------------------------------------------------------------*/

FRESULT trace_path (		/* FR_OK(0): successful, !=0: error code */
	DIR *dirobj,		/* Pointer to directory object to return last directory */
	char *fn,		/* Pointer to last segment name to return {file(8),ext(3),attr(1)} */
	char *path,		/* Full-path string to trace a file or directory */
	BYTE **dir		/* Directory pointer in Win[] to retutn */
)
{
	DWORD clust;
	char ds;
	register BYTE *dptr = NULL;
	FATFS *fs = dirobj->fs;	/* Get logical drive from the given DIR structure */


	/* Initialize directory object */
	clust = fs->dirbase;
	if (fs->fs_type == FS_FAT32) {
		dirobj->clust = dirobj->sclust = clust;
		dirobj->sect = clust2sect(fs, clust);
	} else {
		dirobj->clust = dirobj->sclust = 0;
		dirobj->sect = clust;
	}
	dirobj->index = 0;

	if (*path == '\0') {					/* Null path means the root directory */
		*dir = NULL; return FR_OK;
	}

	for (;;) {
		ds = make_dirfile(&path, fn);			/* Get a paragraph into fn[] */
		if (ds == 1) return FR_INVALID_NAME;
		for (;;) {
			if (!move_window(fs, dirobj->sect)) return FR_RW_ERROR;
			dptr = &fs->win[(dirobj->index & ((S_SIZ - 1) / 32)) * 32];	/* Pointer to the directory entry */
			if (dptr[DIR_Name] == 0)						/* Has it reached to end of dir? */
				return !ds ? FR_NO_FILE : FR_NO_PATH;
			if (dptr[DIR_Name] != 0xE5						/* Matched? */
				&& !(dptr[DIR_Attr] & AM_VOL)
				&& !memcmp(&dptr[DIR_Name], fn, 8+3) ) break;
			if (!next_dir_entry(dirobj))					/* Next directory pointer */
				return !ds ? FR_NO_FILE : FR_NO_PATH;
		}
		if (!ds) { *dir = dptr; return FR_OK; }				/* Matched with end of path */
		if (!(dptr[DIR_Attr] & AM_DIR)) return FR_NO_PATH;	/* Cannot trace because it is a file */
		clust = ((DWORD)LD_WORD(&dptr[DIR_FstClusHI]) << 16) | LD_WORD(&dptr[DIR_FstClusLO]); /* Get cluster# of the directory */
		dirobj->clust = dirobj->sclust = clust;				/* Restart scanning at the new directory */
		dirobj->sect = clust2sect(fs, clust);
		dirobj->index = 2;
	}
}




/*-----------------------------------------------------------------------*/
/* Reserve a directory entry                                             */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
FRESULT reserve_direntry (	/* FR_OK: successful, FR_DENIED: no free entry, FR_RW_ERROR: a disk error occured */
	DIR *dirobj,			/* Target directory to create new entry */
	BYTE **dir				/* Pointer to pointer to created entry to retutn */
)
{
	DWORD clust, sector;
	BYTE c;
	register BYTE *dptr;
/* +-serge */
        WORD /*BYTE*/ n;
/* serge */

	FATFS *fs = dirobj->fs;

	/* Re-initialize directory object */
	clust = dirobj->sclust;
	if (clust) {	/* Dyanmic directory table */
		dirobj->clust = clust;
		dirobj->sect = clust2sect(fs, clust);
	} else {		/* Static directory table */
		dirobj->sect = fs->dirbase;
	}
	dirobj->index = 0;

	do {
		if (!move_window(fs, dirobj->sect)) return FR_RW_ERROR;
		dptr = &fs->win[(dirobj->index & ((S_SIZ - 1) / 32)) * 32];	/* Pointer to the directory entry */
		c = dptr[DIR_Name];
		if (c == 0 || c == 0xE5) {			/* Found an empty entry! */
			*dir = dptr; return FR_OK;
		}
	} while (next_dir_entry(dirobj));				/* Next directory pointer */
	/* Reached to end of the directory table */

	/* Abort when static table or could not stretch dynamic table */
	if (!clust || !(clust = create_chain(fs, dirobj->clust))) return FR_DENIED;
	if (clust == 1 || !move_window(fs, 0)) return FR_RW_ERROR;

	fs->winsect = sector = clust2sect(fs, clust);		/* Cleanup the expanded table */
	memset(fs->win, 0, S_SIZ);
	for (n = fs->csize; n; n--) {
		if (disk_write(fs->drive, fs->win, sector, 1) != RES_OK)
			return FR_RW_ERROR;
		sector++;
	}
	fs->winflag = 1;
	*dir = fs->win;
	return FR_OK;
}
#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Load boot record and check if it is a FAT boot record                 */
/*-----------------------------------------------------------------------*/

BYTE check_fs (		/* 0:The FAT boot record, 1:Valid boot record but not an FAT, 2:Not a boot record or error */
	fs,		/* File system object */
	sect		/* Sector# (lba) to check if it is a FAT boot record or not */
)
	register FATFS *fs;
	DWORD sect;
{
	PartType = FS_UNKNOWN;

	if (disk_read(fs->drive, fs->win, sect, 1) != RES_OK)	/* Load boot record */
		return 2;

	if (LD_WORD(&fs->win[BS_55AA]) != 0xAA55)				/* Check record signature (always offset 510) */
		return 2;

	if (!memcmp(&fs->win[BS_FilSysType], "FAT", 3))	{		/* Check FAT signature */
		PartType = FS_FAT16;
		return 0;
	}
	if (!memcmp(&fs->win[BS_FilSysType32], "FAT32", 5) && !(fs->win[BPB_ExtFlags] & 0x80)) {
		PartType = FS_FAT32;
		return 0;
	}
/* +serge */
	if (fs->win[BPB_NTFlags]==0x80) 
		PartType = FS_NTFS;
/* serge */
	return 1;
}




/*-----------------------------------------------------------------------*/
/* Make sure that the file system is valid                               */
/*-----------------------------------------------------------------------*/

FRESULT auto_mount (		/* FR_OK(0): successful, !=0: any error occured */
	char **path,		/* Pointer to pointer to the path name (drive number) */
	FATFS **rfs,		/* Pointer to pointer to the found file system object */
	BYTE chk_wp		/* !=0: Check media write protection for wrinting fuctions */
)
{
	BYTE drv, fmt, *tbl;
	DSTATUS stat;
	DWORD bootsect, fatsize, totalsect, maxclust;
	char *p = *path;
	register FATFS *fs;


	/* Get drive number from the path name */
	while (*p == ' ') p++;		/* Strip leading spaces */
	drv = p[0] - '0';			/* Is there a drive number? */
	if (drv <= 9 && p[1] == ':')
		p += 2;			/* Found a drive number, get and strip it */
	else
		drv = 0;		/* No drive number is given, select drive 0 in default */
	if (*p == '/') p++;	/* Strip heading slash */
	*path = p;			/* Return pointer to the path name */

	/* Check if the drive number is valid or not */
	if (drv >= _DRIVES) return FR_INVALID_DRIVE;	/* Is the drive number valid? */

	if (!(fs = FatFs[drv])) return FR_NOT_ENABLED;	/* Is the file system object registered? */
	*rfs = fs;			/* Returen pointer to the corresponding file system object */

	/* Check if the logical drive has been mounted or not */
	if (fs->fs_type) {
		stat = disk_status(fs->drive);
		if (!(stat & STA_NOINIT)) {				/* If the physical drive is kept initialized */
#if !_FS_READONLY
			if (chk_wp && (stat & STA_PROTECT))	/* Check write protection if needed */
				return FR_WRITE_PROTECTED;
#endif
			return FR_OK;						/* The file system object is valid */
		}
	}

	/* The logical drive has not been mounted, following code attempts to mount the logical drive */

	memset(fs, 0, sizeof(FATFS));		/* Clean-up the file system object */
	fs->drive = LD2PD(drv);				/* Bind the logical drive and a physical drive */

	stat = disk_initialize(fs->drive);	/* Initialize low level disk I/O layer */

	if (stat & STA_NOINIT)				/* Check if the drive is ready */
		return FR_NOT_READY;
#if S_MAX_SIZ > 512						/* Check disk sector size */
	if (disk_ioctl(drv, GET_SECTOR_SIZE, &S_SIZ) != RES_OK || S_SIZ > S_MAX_SIZ)
		return FR_NO_FILESYSTEM;
#endif
#if !_FS_READONLY
	if (chk_wp && (stat & STA_PROTECT))	/* Check write protection if needed */
		return FR_WRITE_PROTECTED;
#endif
	/* Search FAT partition on the drive */
        bootsect = 0;
	fmt = check_fs(fs, bootsect);	/* Check sector 0 as an SFD format */

	if (fmt == 1) {						/* Not a FAT boot record, it may be patitioned */
		/* Check a partition listed in top of the partition table */
		tbl = &fs->win[MBR_Table + LD2PT(drv) * 16];	/* Partition table */
		if (tbl[MBR_PART_TYPE]) {								/* Is the partition existing? */

			bootsect = LD_DWORD(&tbl[8]);			/* Partition offset in LBA */
/* +serge */
			switch(tbl[MBR_PART_TYPE]) {
			  case 0x05:
			  case 0x0F:
			  case 0x42:
			  case 0x85: PartType=FS_SECND_MBR;
                                     if (disk_read(fs->drive, fs->win, bootsect, 1) == RES_OK) {	/* Load secondary partition MRB */
 				       tbl = &fs->win[MBR_Table];					/* Partition table, 1st record */
				       bootsect = bootsect + LD_DWORD(&tbl[8]);			/* Partition offset in LBA */
				     }
                                     break;
			  case 0xA5:
			  case 0xA6:
			  case 0xA9: PartType=FS_BSD;
				     break;
			  case 0xEE: PartType=FS_EFI;
				     break;
			  case 0x0A: PartType=FS_IBMBOOT;
				     break;
			  case 0x82:
			  case 0x83: PartType=FS_LINUX;
				     break;
			  default: ;
			}
/* serge */
			fmt = check_fs(fs, bootsect);			/* Check the partition */
		}
	}
	if (fmt || LD_WORD(&fs->win[BPB_BytsPerSec]) != S_SIZ)	/* No valid FAT patition is found */
		return FR_NO_FILESYSTEM;

	/* Initialize the file system object */
	fatsize = LD_WORD(&fs->win[BPB_FATSz16]);			/* Number of sectors per FAT */
	if (!fatsize) fatsize = LD_DWORD(&fs->win[BPB_FATSz32]);
	fs->sects_fat = fatsize;
	fs->n_fats = fs->win[BPB_NumFATs];					/* Number of FAT copies */
	fatsize *= fs->n_fats;								/* (Number of sectors in FAT area) */
	fs->fatbase = bootsect + LD_WORD(&fs->win[BPB_RsvdSecCnt]); /* FAT start sector (lba) */
	fs->csize = fs->win[BPB_SecPerClus];			/* Number of sectors per cluster */
	fs->n_rootdir = LD_WORD(&fs->win[BPB_RootEntCnt]);	/* Nmuber of root directory entries */
	totalsect = LD_WORD(&fs->win[BPB_TotSec16]);		/* Number of sectors on the file system */
	if (!totalsect) totalsect = LD_DWORD(&fs->win[BPB_TotSec32]);
	fs->max_clust = maxclust = (totalsect				/* Last cluster# + 1 */
		- LD_WORD(&fs->win[BPB_RsvdSecCnt]) - fatsize - fs->n_rootdir / (S_SIZ/32)
		) / fs->csize + 2;

	fmt = FS_FAT12;										/* Determine the FAT sub type */
	if (maxclust >= 0xFF7) fmt = FS_FAT16;
	if (maxclust >= 0xFFF7) fmt = FS_FAT32;

	if (fmt == FS_FAT32)
		fs->dirbase = LD_DWORD(&fs->win[BPB_RootClus]);	/* Root directory start cluster */
	else
		fs->dirbase = fs->fatbase + fatsize;			/* Root directory start sector (lba) */
	fs->database = fs->fatbase + fatsize + fs->n_rootdir / (S_SIZ/32);	/* Data start sector (lba) */

#if !_FS_READONLY
	fs->free_clust = 0xFFFFFFFF;
#if _USE_FSINFO
	/* Load fsinfo sector if needed */
	if (fmt == FS_FAT32) {
		fs->fsi_sector = bootsect + LD_WORD(&fs->win[BPB_FSInfo]);
		if (disk_read(fs->drive, fs->win, fs->fsi_sector, 1) == RES_OK &&
			LD_WORD(&fs->win[BS_55AA]) == 0xAA55 &&
			LD_DWORD(&fs->win[FSI_LeadSig]) == 0x41615252 &&
			LD_DWORD(&fs->win[FSI_StrucSig]) == 0x61417272) {
			fs->last_clust = LD_DWORD(&fs->win[FSI_Nxt_Free]);
			fs->free_clust = LD_DWORD(&fs->win[FSI_Free_Count]);
		}
	}
#endif
#endif
	fs->fs_type = fmt;			/* FAT syb-type */
	fs->id = ++fsid;			/* File system mount ID */
	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Check if the file/dir object is valid or not                          */
/*-----------------------------------------------------------------------*/

FRESULT validate (	/* FR_OK(0): The object is valid, !=0: Not valid */
	fs,		/* Pointer to the file system object */
	id		/* id member of the target object to be checked */
)
	register FATFS *fs;
	WORD id;
{
	if (!fs || !fs->fs_type || fs->id != id)
		return FR_INVALID_OBJECT;
	if (disk_status(fs->drive) & STA_NOINIT)
		return FR_NOT_READY;

	return FR_OK;
}




