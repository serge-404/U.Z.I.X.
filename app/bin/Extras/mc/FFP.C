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

/*--------------------------------------------------------------------------

   Public Functions - separated from ff.c  by Serge 20070820

--------------------------------------------------------------------------*/


#include "string.h"
#include "ff.h"			/* FatFs declarations */
#include "diskio.h"		/* Include file for user provided disk functions */

/*+serge*/

extern PARTITION Drives[];

extern BOOL move_window(FATFS *fs, DWORD sector);
extern DWORD get_cluster(FATFS *fs, DWORD clust);

#if !_FS_READONLY

extern FRESULT sync(FATFS *fs);
extern BOOL put_cluster(FATFS *fs, DWORD clust, DWORD val);
extern BOOL remove_chain(FATFS *fs, DWORD clust);
extern DWORD create_chain(FATFS *fs, DWORD clust);
extern FRESULT reserve_direntry(DIR *dirobj, BYTE **dir);

#endif /* !_FS_READONLY */

extern DWORD clust2sect(FATFS *fs, DWORD clust);
extern BOOL next_dir_entry(DIR *dirobj);

#if _FS_MINIMIZE <= 1

extern void get_fileinfo(FILINFO *finfo, BYTE *dir);

#endif /* _FS_MINIMIZE <= 1 */

extern char make_dirfile(char **path, char *dirname);
extern FRESULT trace_path(DIR *dirobj, char *fn, char *path, BYTE **dir);
extern BYTE check_fs(FATFS *fs, DWORD sect);
extern FRESULT auto_mount(char **path, FATFS **rfs, BYTE chk_wp);
extern FRESULT validate(FATFS *fs, WORD id);

/*serge*/

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

extern FATFS *FatFs[_DRIVES];	/* Pointer to the file system objects (logical drives) */
extern WORD fsid;		/* File system mount ID */


/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Locical Drive                                         */
/*-----------------------------------------------------------------------*/

FRESULT f_mount (
	BYTE drv,		/* Logical drive number to be mounted/unmounted */
	FATFS *fs		/* Pointer to new file system object (NULL for unmount)*/
)
{
	register FATFS *fsobj;

	if (drv >= _DRIVES) return FR_INVALID_DRIVE;
	fsobj = FatFs[drv];
	FatFs[drv] = fs;
/* +-serge */
	if (fsobj) memset(fsobj, 0, sizeof(FATFS));
	if (fs) memset(fs, 0, sizeof(FATFS));
/* serge */

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_open (
	FIL *fp,	/* Pointer to the blank file object */
	char *path,	/* Pointer to the file name */
	BYTE mode	/* Access mode and file open mode flags */
)
{
	register FRESULT res;
	BYTE *dir;
	DIR dirobj;
	char fn[8+3+1];
	FATFS *fs;

	fp->fs = NULL;
#if !_FS_READONLY
	mode &= (FA_READ|FA_WRITE|FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW);
	res = auto_mount(&path, &fs, (BYTE)(mode & (FA_WRITE|FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW)));
#else
	mode &= FA_READ;
	res = auto_mount(&path, &fs, 0);
#endif
	if (res != FR_OK) return res;
	dirobj.fs = fs;

	/* Trace the file path */
	res = trace_path(&dirobj, fn, path, &dir);
#if !_FS_READONLY

	/* Create or Open a file */
	if (mode & (FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW)) {
		DWORD ps, rs;
		if (res != FR_OK) {		/* No file, create new */
			if (res != FR_NO_FILE) return res;
			res = reserve_direntry(&dirobj, &dir);
			if (res != FR_OK) return res;
/* +-serge 20100903 */
			memset(dir, 0, 32);						/* Initialize the new entry with open name */
/* serge */
			memcpy(&dir[DIR_Name], fn, 8+3);
			dir[DIR_NTres] = fn[11];
			mode |= FA_CREATE_ALWAYS;
		}
		else {					/* Any object is already existing */
			if (mode & FA_CREATE_NEW)			/* Cannot create new */
				return FR_EXIST;
			if (dir == NULL || (dir[DIR_Attr] & (AM_RDO|AM_DIR)))	/* Cannot overwrite it (R/O or DIR) */
				return FR_DENIED;
			if (mode & FA_CREATE_ALWAYS) {		/* Resize it to zero if needed */
				rs = ((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);	/* Get start cluster */
				ST_WORD(&dir[DIR_FstClusHI], 0);	/* cluster = 0 */
				ST_WORD(&dir[DIR_FstClusLO], 0);
				ST_DWORD(&dir[DIR_FileSize], 0);	/* size = 0 */
				fs->winflag = 1;
				ps = fs->winsect;				/* Remove the cluster chain */
				if (!remove_chain(fs, rs) || !move_window(fs, ps))
					return FR_RW_ERROR;
				fs->last_clust = rs - 1;		/* Reuse the cluster hole */
			}
		}
		if (mode & FA_CREATE_ALWAYS) {
			dir[DIR_Attr] = 0;					/* Reset attribute */
			ps = get_fattime();
			ST_DWORD(&dir[DIR_CrtTime], ps);	/* Created time */
			fs->winflag = 1;
			mode |= FA__WRITTEN;				/* Set file changed flag */
		}
	}
	/* Open an existing file */
	else {
#endif /* !_FS_READONLY */
		if (res != FR_OK) return res;		/* Trace failed */
		if (dir == NULL || (dir[DIR_Attr] & AM_DIR))	/* It is a directory */
			return FR_NO_FILE;
#if !_FS_READONLY
		if ((mode & FA_WRITE) && (dir[DIR_Attr] & AM_RDO)) /* R/O violation */
			return FR_DENIED;
	}
	fp->dir_sect = fs->winsect;			/* Pointer to the directory entry */
	fp->dir_ptr = dir;
#endif
	fp->flag = mode;					/* File access mode */
	fp->org_clust =						/* File start cluster */
		((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);
	fp->fsize = LD_DWORD(&dir[DIR_FileSize]);	/* File size */
	fp->fptr = 0; fp->csect = 255;		/* File pointer */
	fp->curr_sect = 0;
	fp->fs = fs; fp->id = fs->id;		/* Owner file system object of the file */

	return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_read (
	FIL *fp, 		/* Pointer to the file object */
	void *buff,		/* Pointer to data buffer */
	WORD btr,		/* Number of bytes to read */
	WORD *br		/* Pointer to number of bytes read */
)
{
	DWORD clust, sect, remain;
	register WORD rcnt;
	BYTE *rbuff = buff;
/* +-serge */
	WORD /*BYTE*/ cc;
/* serge */
	FRESULT res;
	FATFS *fs = fp->fs;


	*br = 0;
	res = validate(fs, fp->id);						/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
	if (!(fp->flag & FA_READ)) return FR_DENIED;	/* Check access mode */
	remain = fp->fsize - fp->fptr;
	if (btr > remain) btr = (WORD)remain;			/* Truncate read count by number of bytes left */

	for ( ;  btr;									/* Repeat until all data transferred */
		rbuff += rcnt, fp->fptr += rcnt, *br += rcnt, btr -= rcnt) {
		if ((fp->fptr & (S_SIZ - 1)) == 0) {		/* On the sector boundary */
			if (fp->csect >= fs->csize) {		/* On the cluster boundary? */
				clust = (fp->fptr == 0) ?			/* On the top of the file? */
					fp->org_clust : get_cluster(fp->fs, fp->curr_clust);
				if (clust < 2 || clust >= fs->max_clust) goto fr_error;
				fp->curr_clust = clust;				/* Update current cluster */
				fp->csect = 0;						/* Reset sector address in the cluster */
			}
			sect = clust2sect(fp->fs, fp->curr_clust) + fp->csect;	/* Get current sector */
			cc = btr / S_SIZ;					/* When remaining bytes >= sector size, */
			if (cc) {								/* Read maximum contiguous sectors directly */
				if (fp->csect + cc > fs->csize)	/* Clip at cluster boundary */
					cc = fs->csize - fp->csect;
				if (disk_read(fs->drive, rbuff, sect, (BYTE)cc) != RES_OK)
					goto fr_error;
				fp->csect += (BYTE)cc;				/* Next sector address in the cluster */
				rcnt = cc *  S_SIZ;				/* Number of bytes transferred */
				continue;
			}
			if (sect != fp->curr_sect) {			/* Is window offset changed? */
#if !_FS_READONLY
				if (fp->flag & FA__DIRTY) {			/* Write back file I/O buffer if needed */
					if (disk_write(fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
						goto fr_error;
					fp->flag &= (BYTE)~FA__DIRTY;
				}
#endif
				if (disk_read(fs->drive, fp->buffer, sect, 1) != RES_OK)	/* Fill file I/O buffer with file data */
					goto fr_error;
				fp->curr_sect = sect;
			}
			fp->csect++;							/* Next sector address in the cluster */
		}
		rcnt = S_SIZ - ((WORD)fp->fptr & (S_SIZ - 1));				/* Copy fractional bytes from file I/O buffer */
		if (rcnt > btr) rcnt = btr;
		memcpy(rbuff, &fp->buffer[fp->fptr & (S_SIZ - 1)], rcnt);
	}

	return FR_OK;

fr_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}




#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_write (
	FIL *fp,	/* Pointer to the file object */
	void *buff,	/* Pointer to the data to be written */
	WORD btw,			/* Number of bytes to write */
	WORD *bw			/* Pointer to number of bytes written */
)
{
	DWORD clust, sect;
	register WORD wcnt;
/* +-serge */
	WORD /*BYTE*/ cc;
/* serge */
	FRESULT res;
	BYTE *wbuff = buff;
	FATFS *fs = fp->fs;


	*bw = 0;
	res = validate(fs, fp->id);						/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
	if (!(fp->flag & FA_WRITE)) return FR_DENIED;	/* Check access mode */
	if (fp->fsize + btw < fp->fsize) return FR_OK;	/* File size cannot reach 4GB */

	for ( ;  btw;									/* Repeat until all data transferred */
		wbuff += wcnt, fp->fptr += wcnt, *bw += wcnt, btw -= wcnt) {
		if ((fp->fptr & (S_SIZ - 1)) == 0) {		/* On the sector boundary */
			if (fp->csect >= fs->csize) {		/* On the cluster boundary? */
				if (fp->fptr == 0) {				/* On the top of the file? */
					clust = fp->org_clust;			/* Follow from the origin */
					if (clust == 0)					/* When there is no cluster chain, */
						fp->org_clust = clust = create_chain(fp->fs, 0);	/* Create a new cluster chain */
				} else {							/* Middle or end of the file */
					clust = create_chain(fp->fs, fp->curr_clust);			/* Trace or streach cluster chain */
				}
				if (clust == 0) break;				/* Could not allocate a new cluster (disk full) */
				if (clust == 1 || clust >= fs->max_clust) goto fw_error;
				fp->curr_clust = clust;				/* Update current cluster */
				fp->csect = 0;						/* Reset sector address in the cluster */
			}
			sect = clust2sect(fp->fs, fp->curr_clust) + fp->csect;	/* Get current sector */
			cc = btw / S_SIZ;					/* When remaining bytes >= sector size, */
			if (cc) {								/* Write maximum contiguous sectors directly */
				if (fp->csect + cc > fs->csize)	/* Clip at cluster boundary */
					cc = fs->csize - fp->csect;
				if (disk_write(fs->drive, wbuff, sect, (BYTE)cc) != RES_OK)
					goto fw_error;
				fp->csect += (BYTE)cc;				/* Next sector address in the cluster */
				wcnt = cc * S_SIZ;				/* Number of bytes transferred */
				continue;
			}
			if (sect != fp->curr_sect) {			/* Is window offset changed? */
				if (fp->flag & FA__DIRTY) {			/* Write back file I/O buffer if needed */
					if (disk_write(fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
						goto fw_error;
					fp->flag &= (BYTE)~FA__DIRTY;
				}
				if (fp->fptr < fp->fsize &&  		/* Fill file I/O buffer with file data */
					disk_read(fs->drive, fp->buffer, sect, 1) != RES_OK)
						goto fw_error;
				fp->curr_sect = sect;
			}
			fp->csect++;							/* Next sector address in the cluster */
		}
		wcnt = S_SIZ - ((WORD)fp->fptr & (S_SIZ - 1));	/* Copy fractional bytes to file I/O buffer */
		if (wcnt > btw) wcnt = btw;
		memcpy(&fp->buffer[fp->fptr & (S_SIZ - 1)], wbuff, wcnt);
		fp->flag |= FA__DIRTY;
	}

	if (fp->fptr > fp->fsize) fp->fsize = fp->fptr;	/* Update file size if needed */
	fp->flag |= FA__WRITTEN;						/* Set file changed flag */
	return FR_OK;

fw_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}




/*-----------------------------------------------------------------------*/
/* Synchronize between File and Disk                                     */
/*-----------------------------------------------------------------------*/

FRESULT f_sync (
	FIL *fp		/* Pointer to the file object */
)
{
	DWORD tim;
	BYTE *dir;
	register FRESULT res;
	FATFS *fs = fp->fs;


	res = validate(fs, fp->id);			/* Check validity of the object */
	if (res == FR_OK) {
		if (fp->flag & FA__WRITTEN) {	/* Has the file been written? */
			/* Write back data buffer if needed */
			if (fp->flag & FA__DIRTY) {
				if (disk_write(fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
					return FR_RW_ERROR;
				fp->flag &= (BYTE)~FA__DIRTY;
			}
			/* Update the directory entry */
			if (!move_window(fs, fp->dir_sect))
				return FR_RW_ERROR;
			dir = fp->dir_ptr;
			dir[DIR_Attr] |= AM_ARC;						/* Set archive bit */
			ST_DWORD(&dir[DIR_FileSize], fp->fsize);		/* Update file size */
			ST_WORD(&dir[DIR_FstClusLO], fp->org_clust);	/* Update start cluster */
			ST_WORD(&dir[DIR_FstClusHI], fp->org_clust >> 16);
			tim = get_fattime();					/* Updated time */
			ST_DWORD(&dir[DIR_WrtTime], tim);
			fp->flag &= (BYTE)~FA__WRITTEN;
			res = sync(fs);
		}
	}
	return res;
}

#endif /* !_FS_READONLY */




/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT f_close (
	FIL *fp		/* Pointer to the file object to be closed */
)
{
	register FRESULT res;


#if !_FS_READONLY
	res = f_sync(fp);
#else
	res = validate(fp->fs, fp->id);
#endif
	if (res == FR_OK)
		fp->fs = NULL;
	return res;
}




#if _FS_MINIMIZE <= 2
/*-----------------------------------------------------------------------*/
/* Seek File R/W Pointer                                                 */
/*-----------------------------------------------------------------------*/

#ifdef _FS_MINIMIZE_LSEEK

FRESULT f_lseek (
	FIL *fp,		/* Pointer to the file object */
	DWORD ofs		/* File pointer from top of file */
)
{
	DWORD clust, csize, nsect, ifptr;
	FRESULT res;
	register FATFS *fs = fp->fs;

	res = validate(fs, fp->id);			/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;
	if (ofs > fp->fsize					/* In read-only mode, clip offset with the file size */
#if !_FS_READONLY
		 && !(fp->flag & FA_WRITE)
#endif
		) ofs = fp->fsize;

	ifptr = fp->fptr;
	fp->fptr = 0; fp->csect = 255;
	nsect = 0;
	if (ofs > 0) {
		csize = (DWORD)fs->csize * S_SIZ;	/* Cluster size (byte) */
		if (ifptr > 0 &&
			(ofs - 1) / csize >= (ifptr - 1) / csize) {/* When seek to same or following cluster, */
			fp->fptr = (ifptr - 1) & ~(csize - 1);	/* start from the current cluster */
			ofs -= fp->fptr;
			clust = fp->curr_clust;
		} else {									/* When seek to back cluster, */
			clust = fp->org_clust;					/* start from the first cluster */
#if !_FS_READONLY
			if (clust == 0) {						/* If no cluster chain, create a new chain */
				clust = create_chain(fp->fs, 0);
				if (clust == 1) goto fk_error;
				fp->org_clust = clust;
			}
#endif
			fp->curr_clust = clust;
		}
		if (clust != 0) {
			while (ofs > csize) {					/* Cluster following loop */
#if !_FS_READONLY
				if (fp->flag & FA_WRITE) {			/* Check if in write mode or not */
					clust = create_chain(fp->fs, clust);	/* Force streached if in write mode */
					if (clust == 0) {				/* When disk gets full, clip file size */
						ofs = csize; break;
					}
				} else
#endif
					clust = get_cluster(fp->fs, clust);	/* Follow cluster chain if not in write mode */
				if (clust < 2 || clust >= fs->max_clust) goto fk_error;
				fp->curr_clust = clust;
				fp->fptr += csize;
				ofs -= csize;
			}
			fp->fptr += ofs;
			fp->csect = (BYTE)(ofs / S_SIZ);	/* Sector offset in the cluster */
			if (ofs & (S_SIZ - 1)) {
				nsect = clust2sect(fp->fs, clust) + fp->csect;	/* Current sector */
				fp->csect++;
			}
		}
	}
	if (nsect && nsect != fp->curr_sect) {
#if !_FS_READONLY
		if (fp->flag & FA__DIRTY) {			/* Write-back dirty buffer if needed */
			if (disk_write(fs->drive, fp->buffer, fp->curr_sect, 1) != RES_OK)
				goto fk_error;
			fp->flag &= (BYTE)~FA__DIRTY;
		}
#endif
		if (disk_read(fs->drive, fp->buffer, nsect, 1) != RES_OK)
			goto fk_error;
		fp->curr_sect = nsect;
	}

#if !_FS_READONLY
	if (fp->fptr > fp->fsize) {			/* Set changed flag if the file was extended */
		fp->fsize = fp->fptr;
		fp->flag |= FA__WRITTEN;
	}
#endif

	return FR_OK;

fk_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}

#endif _FS_MINIMIZE_LSEEK


#if _FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Create a directroy object                                             */
/*-----------------------------------------------------------------------*/

FRESULT f_opendir (
	DIR *dj,			/* Pointer to directory object to create */
	char *path	/* Pointer to the directory path */
)
{
	register FRESULT res;
	BYTE *dir;
	char fn[8+3+1];

	res = auto_mount(&path, &dj->fs, 0);
	if (res == FR_OK) {
		res = trace_path(dj, fn, path, &dir);	/* Trace the directory path */
		if (res == FR_OK) {						/* Trace completed */
			if (dir) {							/* It is not the root dir */
				if (dir[DIR_Attr] & AM_DIR) {	/* The entry is a directory */
					dj->clust = ((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);
					dj->sect = clust2sect(dj->fs, dj->clust);
					dj->index = 2;
				} else {						/* The entry is not a directory */
					res = FR_NO_FILE;
				}
			}
			dj->id = dj->fs->id;
		}
	}

	return res;
}


/*-----------------------------------------------------------------------*/
/* Read Directory Entry in Sequense                                      */
/*-----------------------------------------------------------------------*/

FRESULT f_readdir (
	DIR *dj,			/* Pointer to the directory object */
	FILINFO *finfo		/* Pointer to file information to return */
)
{
	register BYTE *dir;
	BYTE c, res;

	res = validate(dj->fs, dj->id);			/* Check validity of the object */
	if (res != FR_OK) return res;

	finfo->fname[0] = 0;
	while (dj->sect) {
		if (!move_window(dj->fs, dj->sect))
			return FR_RW_ERROR;
		dir = &dj->fs->win[(dj->index & ((S_SIZ - 1) >> 5)) * 32];	/* pointer to the directory entry */
		c = dir[DIR_Name];
		if (c == 0) break;							/* Has it reached to end of dir? */
		if (c != 0xE5 && !(dir[DIR_Attr] & AM_VOL))	/* Is it a valid entry? */
			get_fileinfo(finfo, dir);
		if (!next_dir_entry(dj)) dj->sect = 0;		/* Next entry */
		if (finfo->fname[0]) break;					/* Found valid entry */
	}
	return FR_OK;
}


#if _FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/

#ifdef _FS_MINIMIZE_STAT

FRESULT f_stat (
	char *path,	/* Pointer to the file path */
	FILINFO *finfo		/* Pointer to file information to return */
)
{
	register FRESULT res;
	DIR dj;
	BYTE *dir;
	char fn[8+3+1];

	res = auto_mount(&path, &dj.fs, 0);
	if (res == FR_OK) {
		res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */
		if (res == FR_OK) {						/* Trace completed */
			if (dir)	/* Found an object */
				get_fileinfo(finfo, dir);
			else		/* It is root dir */
				res = FR_INVALID_NAME;
		}
	}
	return res;
}

#endif _FS_MINIMIZE_STAT


#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/

#ifdef _FS_MINIMIZE_TRUNC

FRESULT f_truncate (
	FIL *fp		/* Pointer to the file object */
)
{
	register FRESULT res;
	DWORD ncl;


	res = validate(fp->fs, fp->id);		/* Check validity of the object */
	if (res != FR_OK) return res;
	if (fp->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
	if (!(fp->flag & FA_WRITE)) return FR_DENIED;	/* Check access mode */

	if (fp->fsize > fp->fptr) {
		fp->fsize = fp->fptr;	/* Set file size to current R/W point */
		fp->flag |= FA__WRITTEN;
		if (fp->fptr == 0) {	/* When set file size to zero, remove entire cluster chain */
			if (!remove_chain(fp->fs, fp->org_clust)) goto ft_error;
			fp->org_clust = 0;
		} else {				/* When truncate a part of the file, remove remaining clusters */
			ncl = get_cluster(fp->fs, fp->curr_clust);
			if (ncl < 2) goto ft_error;
			if (ncl < fp->fs->max_clust) {
				if (!put_cluster(fp->fs, fp->curr_clust, 0x0FFFFFFF)) goto ft_error;
				if (!remove_chain(fp->fs, ncl)) goto ft_error;
			}
		}
	}

	return FR_OK;

ft_error:	/* Abort this file due to an unrecoverable error */
	fp->flag |= FA__ERROR;
	return FR_RW_ERROR;
}

#endif _FS_MINIMIZE_TRUNC

/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/

FRESULT f_getfree (
	char *drv,		/* Logical drive number */
	DWORD *nclust,		/* Pointer to the double word to return number of free clusters */
	FATFS **fatfs		/* Pointer to pointer to the file system object to return */
)
{
	DWORD n, clust, sect;
	BYTE fat, f;
	register BYTE *p;
	FRESULT res;
	FATFS *fs;


	/* Get drive number */
	res = auto_mount(&drv, &fs, 0);
	if (res != FR_OK) return res;
	*fatfs = fs;

	/* If number of free cluster is valid, return it without cluster scan. */
	if (fs->free_clust <= fs->max_clust - 2) {
		*nclust = fs->free_clust;
		return FR_OK;
	}

	/* Count number of free clusters */
	fat = fs->fs_type;
	n = 0;
	if (fat == FS_FAT12) {
		clust = 2;
		do {
			if ((WORD)get_cluster(fs, clust) == 0) n++;
		} while (++clust < fs->max_clust);
	} else {
		clust = fs->max_clust;
		sect = fs->fatbase;
		f = 0; p = 0;
		do {
			if (!f) {
				if (!move_window(fs, sect++)) return FR_RW_ERROR;
				p = fs->win;
			}
			if (fat == FS_FAT16) {
				if (LD_WORD(p) == 0) n++;
				p += 2; f += 1;
			} else {
				if (LD_DWORD(p) == 0) n++;
				p += 4; f += 2;
			}
		} while (--clust);
	}
	fs->free_clust = n;
#if _USE_FSINFO
	if (fat == FS_FAT32) fs->fsi_flag = 1;
#endif

	*nclust = n;
	return FR_OK;
}


/*-----------------------------------------------------------------------*/
/* Delete a File or a Directory                                          */
/*-----------------------------------------------------------------------*/

FRESULT f_unlink (
	char *path			/* Pointer to the file or directory path */
)
{
	BYTE *dir, *sdir;
	DWORD dclust, dsect;
	char fn[8+3+1];
	register FRESULT res;
	DIR dirobj;
	FATFS *fs;


	res = auto_mount(&path, &fs, 1);
	if (res != FR_OK) return res;
	dirobj.fs = fs;

	res = trace_path(&dirobj, fn, path, &dir);	/* Trace the file path */
	if (res != FR_OK) return res;				/* Trace failed */
	if (dir == NULL) return FR_INVALID_NAME;	/* It is the root directory */
	if (dir[DIR_Attr] & AM_RDO) return FR_DENIED;	/* It is a R/O object */
	dsect = fs->winsect;
	dclust = ((DWORD)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);

	if (dir[DIR_Attr] & AM_DIR) {				/* It is a sub-directory */
		dirobj.clust = dclust;					/* Check if the sub-dir is empty or not */
		dirobj.sect = clust2sect(fs, dclust);
		dirobj.index = 2;
		do {
			if (!move_window(fs, dirobj.sect)) return FR_RW_ERROR;
			sdir = &fs->win[(dirobj.index & ((S_SIZ - 1) >> 5)) * 32];
			if (sdir[DIR_Name] == 0) break;
			if (sdir[DIR_Name] != 0xE5 && !(sdir[DIR_Attr] & AM_VOL))
				return FR_DENIED;	/* The directory is not empty */
		} while (next_dir_entry(&dirobj));
	}

	if (!move_window(fs, dsect)) return FR_RW_ERROR;	/* Mark the directory entry 'deleted' */
	dir[DIR_Name] = 0xE5;
	fs->winflag = 1;
	if (!remove_chain(fs, dclust)) return FR_RW_ERROR;	/* Remove the cluster chain */

	return sync(fs);
}




/*-----------------------------------------------------------------------*/
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/

FRESULT f_mkdir (
	char *path		/* Pointer to the directory path */
)
{
	BYTE *dir;
	register BYTE *fw;
/* +-serge */
        WORD /*BYTE*/ n;
/* serge */
	char fn[8+3+1];
	DWORD sect, dsect, dclust, pclust, tim;
	FRESULT res;
	DIR dirobj;
	FATFS *fs;


	res = auto_mount(&path, &fs, 1);
	if (res != FR_OK) return res;
	dirobj.fs = fs;

	res = trace_path(&dirobj, fn, path, &dir);	/* Trace the file path */
	if (res == FR_OK) return FR_EXIST;			/* Any file or directory is already existing */
	if (res != FR_NO_FILE) return res;

	res = reserve_direntry(&dirobj, &dir); 		/* Reserve a directory entry */
	if (res != FR_OK) return res;
	sect = fs->winsect;
	dclust = create_chain(fs, 0);				/* Allocate a cluster for new directory table */
	if (dclust == 1) return FR_RW_ERROR;
	dsect = clust2sect(fs, dclust);
	if (!dsect) return FR_DENIED;
	if (!move_window(fs, dsect)) return FR_RW_ERROR;

	fw = fs->win;
/* +-serge */
	memset(fw, 0, S_SIZ);						/* Clear the new directory table */
/* serge */
	for (n = 1; n < fs->csize; n++) {
		if (disk_write(fs->drive, fw, ++dsect, 1) != RES_OK)
			return FR_RW_ERROR;
	}
/* +-serge */
	memset(&fw[DIR_Name], ' ', 8+3);		/* Create "." entry */
/* serge */
	fw[DIR_Name] = '.';
	fw[DIR_Attr] = AM_DIR;
	tim = get_fattime();
	ST_DWORD(&fw[DIR_WrtTime], tim);
	memcpy(&fw[32], &fw[0], 32); fw[33] = '.';	/* Create ".." entry */
	ST_WORD(&fw[   DIR_FstClusLO], dclust);

	ST_WORD(&fw[   DIR_FstClusHI], dclust >> 16);
	pclust = dirobj.sclust;
	if (fs->fs_type == FS_FAT32 && pclust == fs->dirbase) pclust = 0;
	ST_WORD(&fw[32+DIR_FstClusLO], pclust);
	ST_WORD(&fw[32+DIR_FstClusHI], pclust >> 16);
	fs->winflag = 1;

	if (!move_window(fs, sect)) return FR_RW_ERROR;
/* +-serge 20100903 */
	memset(&dir, 0, 32);						/* Initialize the new entry */
/* serge */
	memcpy(&dir[DIR_Name], fn, 8+3);			/* Name */
	dir[DIR_NTres] = fn[11];
	dir[DIR_Attr] = AM_DIR;						/* Attribute */
	ST_DWORD(&dir[DIR_WrtTime], tim);			/* Crated time */
	ST_WORD(&dir[DIR_FstClusLO], dclust);		/* Table start cluster */
	ST_WORD(&dir[DIR_FstClusHI], dclust >> 16);

	return sync(fs);
}




/*-----------------------------------------------------------------------*/
/* Change File Attribute                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_chmod (
	char *path,	/* Pointer to the file path */
	BYTE value,			/* Attribute bits */
	BYTE mask			/* Attribute mask to change */
)
{
	register FRESULT res;
	BYTE *dir;
	DIR dirobj;
	char fn[8+3+1];
	FATFS *fs;


	res = auto_mount(&path, &fs, 1);
	if (res == FR_OK) {
		dirobj.fs = fs;
		res = trace_path(&dirobj, fn, path, &dir);	/* Trace the file path */
		if (res == FR_OK) {			/* Trace completed */
			if (dir == NULL) {
				res = FR_INVALID_NAME;
			} else {
				mask &= AM_RDO|AM_HID|AM_SYS|AM_ARC;	/* Valid attribute mask */
				dir[DIR_Attr] = (value & mask) | (dir[DIR_Attr] & (BYTE)~mask);	/* Apply attribute change */
				res = sync(fs);
			}
		}
	}
	return res;
}



/*-----------------------------------------------------------------------*/
/* Change Timestamp                                                      */
/*-----------------------------------------------------------------------*/

#ifdef _FS_MINIMIZE_UTIME

FRESULT f_utime (
	char *path,		/* Pointer to the file/directory name */
	FILINFO *finfo	/* Pointer to the timestamp to be set */
)
{
	register FRESULT res;
	DIR dj;
	BYTE *dir;
	char fn[8+3+1];


	res = auto_mount(&path, &dj.fs, 1);
	if (res == FR_OK) {
		res = trace_path(&dj, fn, path, &dir);	/* Trace the file path */
		if (res == FR_OK) {				/* Trace completed */
			if (!dir) {
				res = FR_INVALID_NAME;	/* Root directory */
			} else {
				ST_WORD(&dir[DIR_WrtTime], finfo->ftime);
				ST_WORD(&dir[DIR_WrtDate], finfo->fdate);
				res = sync(dj.fs);
			}
		}
	}
	return res;
}

#endif _FS_MINIMIZE_UTIME

/*-----------------------------------------------------------------------*/
/* Rename File/Directory                                                 */
/*-----------------------------------------------------------------------*/

FRESULT f_rename (
	char *path_old,	/* Pointer to the old name */
	char *path_new	/* Pointer to the new name */
)
{
	register FRESULT res;
	DWORD sect_old;
	BYTE *dir_old, *dir_new, direntry[32-11];
	DIR dirobj;
	char fn[8+3+1];
	FATFS *fs;


	res = auto_mount(&path_old, &fs, 1);
	if (res != FR_OK) return res;
	dirobj.fs = fs;

	res = trace_path(&dirobj, fn, path_old, &dir_old);	/* Check old object */
	if (res != FR_OK) return res;			/* The old object is not found */
	if (!dir_old) return FR_NO_FILE;
	sect_old = fs->winsect;					/* Save the object information */
	memcpy(direntry, &dir_old[DIR_Attr], 32-11);

	res = trace_path(&dirobj, fn, path_new, &dir_new);	/* Check new object */
	if (res == FR_OK) return FR_EXIST;			/* The new object name is already existing */
	if (res != FR_NO_FILE) return res;			/* Is there no old name? */
	res = reserve_direntry(&dirobj, &dir_new); 	/* Reserve a directory entry */
	if (res != FR_OK) return res;

	memcpy(&dir_new[DIR_Attr], direntry, 32-11);	/* Create new entry */
	memcpy(&dir_new[DIR_Name], fn, 8+3);
	dir_new[DIR_NTres] = fn[11];
	fs->winflag = 1;

	if (!move_window(fs, sect_old)) return FR_RW_ERROR;	/* Remove old entry */
	dir_old[DIR_Name] = 0xE5;

	return sync(fs);
}



#if _USE_MKFS

/*-----------------------------------------------------------------------*/
/* Create File System on the Drive                                       */
/*-----------------------------------------------------------------------*/
#define N_ROOTDIR	512			/* Multiple of 32 and <= 2048 */
#define N_FATS		1			/* 1 or 2 */
#define MAX_SECTOR	64000000UL	/* Maximum partition size */
#define MIN_SECTOR	2000UL		/* Minimum partition size */


FRESULT f_mkfs (
	BYTE drv,			/* Logical drive number */
	BYTE partition,		/* Partitioning rule 0:FDISK, 1:SFD */
	WORD allocsize		/* Allocation unit size [bytes] */
)
{
	BYTE fmt, m;
	register BYTE *tbl;
	DWORD b_part, b_fat, b_dir, b_data;		/* Area offset (LBA) */
	DWORD n_part, n_rsv, n_fat, n_dir;		/* Area size */
	DWORD n_clust, n;
	FATFS *fs;
	DSTATUS stat;


	/* Check validity of the parameters */
	if (drv >= _DRIVES) return FR_INVALID_DRIVE;
	if (partition >= 2) return FR_MKFS_ABORTED;
	for (n = 512; n <= 32768U && n != allocsize; n <<= 1);
	if (n != allocsize) return FR_MKFS_ABORTED;

	/* Check mounted drive and clear work area */
	fs = FatFs[drv];
	if (!fs) return FR_NOT_ENABLED;
	fs->fs_type = 0;


	drv = LD2PD(drv);





	/* Get disk statics */
	stat = disk_initialize(drv);
	if (stat & STA_NOINIT) return FR_NOT_READY;
	if (stat & STA_PROTECT) return FR_WRITE_PROTECTED;
	if (disk_ioctl(drv, GET_SECTOR_COUNT, &n_part) != RES_OK || n_part < MIN_SECTOR)
		return FR_MKFS_ABORTED;
	if (n_part > MAX_SECTOR) n_part = MAX_SECTOR;
	b_part = (!partition) ? 63 : 0;		/* Boot sector */
	n_part -= b_part;
#if S_MAX_SIZ > 512						/* Check disk sector size */
	if (disk_ioctl(drv, GET_SECTOR_SIZE, &S_SIZ) != RES_OK
		|| S_SIZ > S_MAX_SIZ
		|| S_SIZ > allocsize)
		return FR_MKFS_ABORTED;
#endif
	allocsize /= S_SIZ;		/* Number of sectors per cluster */

	/* Pre-compute number of clusters and FAT type */
	n_clust = n_part / allocsize;
	fmt = FS_FAT12;
	if (n_clust >= 0xFF5) fmt = FS_FAT16;
	if (n_clust >= 0xFFF5) fmt = FS_FAT32;

	/* Determine offset and size of FAT structure */
	switch (fmt) {
	case FS_FAT12:
		n_fat = ((n_clust * 3 + 1) / 2 + 3 + S_SIZ - 1) / S_SIZ;
		n_rsv = 1 + partition;
		n_dir = N_ROOTDIR * 32 / S_SIZ;
		break;
	case FS_FAT16:
		n_fat = ((n_clust * 2) + 4 + S_SIZ - 1) / S_SIZ;
		n_rsv = 1 + partition;
		n_dir = N_ROOTDIR * 32 / S_SIZ;
		break;
	default:
		n_fat = ((n_clust * 4) + 8 + S_SIZ - 1) / S_SIZ;
		n_rsv = 33 - partition;
		n_dir = 0;
	}
	b_fat = b_part + n_rsv;			/* FATs start sector */
	b_dir = b_fat + n_fat * N_FATS;	/* Directory start sector */
	b_data = b_dir + n_dir;			/* Data start sector */

	/* Align data start sector to erase block boundary (for flash memory media) */
	if (disk_ioctl(drv, GET_BLOCK_SIZE, &n) != RES_OK) return FR_MKFS_ABORTED;
	n = (b_data + n - 1) & ~(n - 1);

	n_fat += (n - b_data) / N_FATS;
	/* b_dir and b_data are no longer used below */

	/* Determine number of cluster and final check of validity of the FAT type */
	n_clust = (n_part - n_rsv - n_fat * N_FATS - n_dir) / allocsize;
	if (   (fmt == FS_FAT16 && n_clust < 0xFF5)
		|| (fmt == FS_FAT32 && n_clust < 0xFFF5))
		return FR_MKFS_ABORTED;

	/* Create partition table if needed */
	if (!partition) {
		DWORD n_disk = b_part + n_part;

		tbl = &fs->win[MBR_Table];
		ST_DWORD(&tbl[0], 0x00010180);	/* Partition start in CHS */
		if (n_disk < 63UL * 255 * 1024) {	/* Partition end in CHS */
			n_disk = n_disk / 63 / 255;
			tbl[7] = (BYTE)n_disk;
			tbl[6] = (BYTE)((n_disk >> 2) | 63);
		} else {
			ST_WORD(&tbl[6], 0xFFFF);
		}
		tbl[5] = 254;
		if (fmt != FS_FAT32)			/* System ID */
			tbl[4] = (n_part < 0x10000) ? 0x04 : 0x06;
		else
			tbl[4] = 0x0c;
		ST_DWORD(&tbl[8], 63);			/* Partition start in LBA */
		ST_DWORD(&tbl[12], n_part);		/* Partition size in LBA */
		ST_WORD(&tbl[64], 0xAA55);		/* Signature */
		if (disk_write(drv, fs->win, 0, 1) != RES_OK)
			return FR_RW_ERROR;
	}

	/* Create boot record */
	tbl = fs->win;								/* Clear buffer */
/* +-serge */
	memset(tbl, 0, S_SIZ);
/* serge */

	ST_DWORD(&tbl[BS_jmpBoot], 0x90FEEB);		/* Boot code (jmp $, nop) */
	ST_WORD(&tbl[BPB_BytsPerSec], S_SIZ);		/* Sector size */
	tbl[BPB_SecPerClus] = (BYTE)allocsize;		/* Sectors per cluster */
	ST_WORD(&tbl[BPB_RsvdSecCnt], n_rsv);		/* Reserved sectors */
	tbl[BPB_NumFATs] = N_FATS;					/* Number of FATs */
	ST_WORD(&tbl[BPB_RootEntCnt], S_SIZ / 32 * n_dir); /* Number of rootdir entries */
	if (n_part < 0x10000) {						/* Number of total sectors */
		ST_WORD(&tbl[BPB_TotSec16], n_part);
	} else {
		ST_DWORD(&tbl[BPB_TotSec32], n_part);
	}
	tbl[BPB_Media] = 0xF8;						/* Media descripter */
	ST_WORD(&tbl[BPB_SecPerTrk], 63);			/* Number of sectors per track */
	ST_WORD(&tbl[BPB_NumHeads], 255);			/* Number of heads */
	ST_DWORD(&tbl[BPB_HiddSec], b_part);		/* Hidden sectors */
	n = get_fattime();							/* Use current time as a VSN */
	if (fmt != FS_FAT32) {
		ST_DWORD(&tbl[BS_VolID], n);			/* Volume serial number */
		ST_WORD(&tbl[BPB_FATSz16], n_fat);		/* Number of secters per FAT */
		tbl[BS_DrvNum] = 0x80;					/* Drive number */
		tbl[BS_BootSig] = 0x29;					/* Extended boot signature */
		memcpy(&tbl[BS_VolLab], "NO NAME    FAT     ", 19);	/* Volume lavel, FAT signature */
	} else {
		ST_DWORD(&tbl[BS_VolID32], n);			/* Volume serial number */
		ST_DWORD(&tbl[BPB_FATSz32], n_fat);		/* Number of secters per FAT */
		ST_DWORD(&tbl[BPB_RootClus], 2);		/* Root directory cluster (2) */
		ST_WORD(&tbl[BPB_FSInfo], 1);			/* FSInfo record (bs+1) */
		ST_WORD(&tbl[BPB_BkBootSec], 6);		/* Backup boot record (bs+6) */
		tbl[BS_DrvNum32] = 0x80;				/* Drive number */
		tbl[BS_BootSig32] = 0x29;				/* Extended boot signature */
		memcpy(&tbl[BS_VolLab32], "NO NAME    FAT32   ", 19);	/* Volume lavel, FAT signature */
	}
	ST_WORD(&tbl[BS_55AA], 0xAA55);			/* Signature */
	if (disk_write(drv, tbl, b_part+0, 1) != RES_OK)
		return FR_RW_ERROR;
	if (fmt == FS_FAT32)
		disk_write(drv, tbl, b_part+6, 1);

	/* Initialize FAT area */
	for (m = 0; m < N_FATS; m++) {
/* +-serge */
		memset(tbl, 0, S_SIZ);		/* 1st sector of the FAT  */
/* serge */

		if (fmt != FS_FAT32) {
			n = (fmt == FS_FAT12) ? 0x00FFFFF8 : 0xFFFFFFF8;
			ST_DWORD(&tbl[0], n);			/* Reserve cluster #0-1 (FAT12/16) */
		} else {
			ST_DWORD(&tbl[0], 0xFFFFFFF8);	/* Reserve cluster #0-1 (FAT32) */
			ST_DWORD(&tbl[4], 0xFFFFFFFF);
			ST_DWORD(&tbl[8], 0x0FFFFFFF);	/* Reserve cluster #2 for root dir */
		}
		if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
			return FR_RW_ERROR;
/* +-serge */
		memset(tbl, 0, S_SIZ);		/* Following FAT entries are filled by zero */
/* serge */

		for (n = 1; n < n_fat; n++) {
			if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
				return FR_RW_ERROR;
		}
	}

	/* Initialize Root directory */
	m = (BYTE)((fmt == FS_FAT32) ? allocsize : n_dir);
	do {
		if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
			return FR_RW_ERROR;
	} while (--m);

	/* Create FSInfo record if needed */
	if (fmt == FS_FAT32) {
		ST_WORD(&tbl[BS_55AA], 0xAA55);
		ST_DWORD(&tbl[FSI_LeadSig], 0x41615252);
		ST_DWORD(&tbl[FSI_StrucSig], 0x61417272);
		ST_DWORD(&tbl[FSI_Free_Count], n_clust - 1);
		ST_DWORD(&tbl[FSI_Nxt_Free], 0xFFFFFFFF);
		disk_write(drv, tbl, b_part+1, 1);
		disk_write(drv, tbl, b_part+7, 1);
	}

	return (disk_ioctl(drv, CTRL_SYNC, NULL) == RES_OK) ? FR_OK : FR_RW_ERROR;
}

#endif /* _USE_MKFS */
#endif /* !_FS_READONLY */
#endif /* _FS_MINIMIZE == 0 */
#endif /* _FS_MINIMIZE <= 1 */
#endif /* _FS_MINIMIZE <= 2 */

