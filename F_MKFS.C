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


/* #include <string.h> */
#include "ff.h"			/* FatFs declarations */
#include "dsk.h"		/* Include file for user provided disk functions */

#define	SS(fs)	512		/* Fixed sector size */

#ifdef _ONLY_MBR
#define md	0xF8
#else
#define buffer_out fs->win
#endif

#define	_VOLUMES   _DRIVES
#define FR_DISK_ERR FR_RW_ERROR

/*-----------------------------------------------------------------------*/
/* Create File System on the Drive                                       */
/*-----------------------------------------------------------------------*/
#define N_ROOTDIR	512		/* Multiple of 32 */
#define N_FATS		1		/* 1 or 2 */

extern BYTE buffer_out[512];	/* Destination Buffer */
extern PARTITION Drives[];

static WORD vst[] = { 1024,   512,  256,  128,   64,    32,   16,    8,    4,    2,   0};
static WORD cst[] = {32768, 16384, 8192, 4096, 2048, 16384, 8192, 4096, 2048, 1024, 512};

/* variables all global because HiTech C bugs */

static	BYTE fmt, drv;
#ifdef _ONLY_MBR
static	WORD au=0;	
#else
static	BYTE md;
static	FATFS *fs;
#endif
static	DWORD n_clst, vs, n, wsect;
static	WORD i;
static	DWORD b_vol, b_fat, b_dir, b_data;	/* Offset (LBA) */
static	DWORD n_vol, n_rsv, n_fat, n_dir;	/* Size */
static	DSTATUS stat;

FRESULT f_mkfs (
	BYTE drive		/* Logical drive number */
#ifndef _ONLY_MBR
	, BYTE sfd,		/* Partitioning rule 0:FDISK, 1:SFD */
	WORD au			/* Allocation unit size [bytes] */
#endif
)
{
	register BYTE *tbl;

	/* Check mounted drive and clear work area */
	if (drv >= _VOLUMES) return FR_INVALID_DRIVE;
#ifndef _ONLY_MBR
	fs = FatFs[drv];
	if (!fs) return FR_NOT_ENABLED;
	fs->fs_type = 0;
#endif
	drv = LD2PD(drive);			/* drv = phisical drive number */

	/* Get disk statics */
	stat = disk_initialize(drv);
	if (stat & STA_NOINIT) return FR_NOT_READY;
	if (stat & STA_PROTECT) return FR_WRITE_PROTECTED;

#ifdef _ONLY_MBR
        if (disk_read(drv, buffer_out, 0, 1) != RES_OK) 	/* Load MRB */
	  return FR_DISK_ERR;
	tbl = &buffer_out[MBR_Table + LD2PT(drive) * 16];	/* Partition table */
	if (! tbl[MBR_PART_TYPE]) 				/* Is the partition existing? */
	  return FR_INVALID_DRIVE;
	b_vol = LD_DWORD(&tbl[8]);				/* Partition offset in LBA */
	switch(tbl[MBR_PART_TYPE]) {
	  case 0x05:
	  case 0x0F:
	  case 0x42:
	  case 0x85: if (disk_read(drv, buffer_out, b_vol, 1) != RES_OK) 	/* Load secondary partition MRB */
			return FR_DISK_ERR;
		     tbl = &buffer_out[MBR_Table];				/* Partition table, 1st record */
		     b_vol += LD_DWORD(&tbl[8]);				/* Partition offset in LBA */
		     break;
	  default: ;
	}
	n_vol = LD_DWORD(&tbl[12]);			/* Partition size in LBA */
#else
	if (disk_ioctl(drv, GET_SECTOR_COUNT, &n_vol) != RES_OK || n_vol < 128)
		return FR_DISK_ERR;

	b_vol = (sfd) ? 0 : 63;		/* Volume start sector */
	n_vol -= b_vol;			/* sector count ( partition size )*/

	if (au & (au - 1)) au = 0;		/* Check validity of the allocation unit size */
#endif

	if (!au) {						/* AU auto selection */
		vs = n_vol / (2000 / (SS(fs) / 512));
		for (i = 0; vs < vst[i]; i++) ;
		au = cst[i];
	}
	au /= SS(fs);		/* Number of sectors per cluster */
	if (au == 0) au = 1;
	if (au > 128) au = 128;

	/* Pre-compute number of clusters and FAT syb-type */
	n_clst = n_vol / au;
	fmt = FS_FAT12;
	if (n_clst >= MIN_FAT16) fmt = FS_FAT16;
	if (n_clst >= MIN_FAT32) fmt = FS_FAT32;

	/* Determine offset and size of FAT structure */
	if (fmt == FS_FAT32) {
		n_fat = ((n_clst * 4) + 8 + SS(fs) - 1) / SS(fs);
		n_rsv = 32;
		n_dir = 0;
	} else {
		n_fat = (fmt == FS_FAT12) ? (n_clst * 3 + 1) / 2 + 3 : (n_clst * 2) + 4;
		n_fat = (n_fat + SS(fs) - 1) / SS(fs);
		n_rsv = 1;
		n_dir = N_ROOTDIR * 32l / SS(fs);
	}
	b_fat = b_vol + n_rsv;				/* FAT area start sector */
	b_dir = b_fat + n_fat * N_FATS;		/* Directory area start sector */
	b_data = b_dir + n_dir;				/* Data area start sector */
	if (n_vol < b_data + au) return FR_MKFS_ABORTED;	/* Too small volume */

	/* Align data start sector to erase block boundary (for flash memory media) */
#ifndef _ONLY_MBR
	if (disk_ioctl(drv, GET_BLOCK_SIZE, &n) != RES_OK || !n || n > 32768)
#endif
	  n = 1;
	n = (b_data + n - 1) & ~(n - 1);	/* Next nearest erase block from current data start */
	n = (n - b_data) / N_FATS;
	if (fmt == FS_FAT32) {		/* FAT32: Move FAT offset */
		n_rsv += n;
		b_fat += n;
	} else {					/* FAT12/16: Expand FAT size */
		n_fat += n;
	}

	/* Determine number of cluster and final check of validity of the FAT sub-type */
	n_clst = (n_vol - n_rsv - n_fat * N_FATS - n_dir) / au;
	if (   (fmt == FS_FAT16 && n_clst < MIN_FAT16)
		|| (fmt == FS_FAT32 && n_clst < MIN_FAT32))
		return FR_MKFS_ABORTED;

#ifndef _ONLY_MBR
	/* Create partition table if required */
	if (sfd) {
		md = 0xF0;
	} else {
		DWORD n_disk = b_vol + n_vol;

		memset(buffer_out, 0, SS(fs));
		tbl = buffer_out+MBR_Table;
		ST_DWORD(tbl, 0x00010180);			/* Partition start in CHS */
		if (n_disk < 63l * 255 * 1024) {	/* Partition end in CHS */
			n_disk = n_disk / 63 / 255;
			tbl[7] = (BYTE)n_disk;
			tbl[6] = (BYTE)((n_disk >> 2) | 63);
		} else {
			ST_WORD(&tbl[6], 0xFFFF);
		}
		tbl[5] = 254;
		if (fmt != FS_FAT32)				/* System ID */
			tbl[4] = (n_vol < 0x10000) ? 0x04 : 0x06;
		else
			tbl[4] = 0x0c;
		ST_DWORD(tbl+8, 63);				/* Partition start in LBA */
		ST_DWORD(tbl+12, n_vol);			/* Partition size in LBA */
		ST_WORD(tbl+64, 0xAA55);			/* Signature */
		if (disk_write(drv, buffer_out, 0, 1) != RES_OK)
			return FR_DISK_ERR;
		md = 0xF8;
	}
#endif
	/* Create volume boot record */
	tbl = buffer_out;							/* Clear sector */
	memset(tbl, 0, SS(fs));
	memcpy(tbl, "\xEB\xFE\x90" "MSDOS5.0", 11);/* Boot code, OEM name */
	i = SS(fs);								/* Sector size */
	ST_WORD(tbl+BPB_BytsPerSec, i);
	tbl[BPB_SecPerClus] = (BYTE)au;			/* Sectors per cluster */
	ST_WORD(tbl+BPB_RsvdSecCnt, n_rsv);		/* Reserved sectors */
	tbl[BPB_NumFATs] = N_FATS;				/* Number of FATs */
	i = (fmt == FS_FAT32) ? 0 : N_ROOTDIR;	/* Number of rootdir entries */
	ST_WORD(tbl+BPB_RootEntCnt, i);
	if (n_vol < 0x10000) {					/* Number of total sectors */
		ST_WORD(tbl+BPB_TotSec16, n_vol);
	} else {
		ST_DWORD(tbl+BPB_TotSec32, n_vol);
	}
	tbl[BPB_Media] = md;					/* Media descriptor */
	ST_WORD(tbl+BPB_SecPerTrk, 63);			/* Number of sectors per track */
	ST_WORD(tbl+BPB_NumHeads, 255);			/* Number of heads */
	ST_DWORD(tbl+BPB_HiddSec, b_vol);		/* Hidden sectors */
	n = get_fattime();						/* Use current time as VSN */
	if (fmt == FS_FAT32) {
		ST_DWORD(tbl+BS_VolID32, n);		/* VSN */
		ST_DWORD(tbl+BPB_FATSz32, n_fat);	/* Number of sectors per FAT */
		ST_DWORD(tbl+BPB_RootClus, 2);		/* Root directory start cluster (2) */
		ST_WORD(tbl+BPB_FSInfo, 1);			/* FSInfo record offset (VBR+1) */
		ST_WORD(tbl+BPB_BkBootSec, 6);		/* Backup boot record offset (VBR+6) */
		tbl[BS_DrvNum32] = 0x80;			/* Drive number */
		tbl[BS_BootSig32] = 0x29;			/* Extended boot signature */
		memcpy(tbl+BS_VolLab32, "NO NAME    " "FAT32   ", 19);	/* Volume label, FAT signature */
	} else {
		ST_DWORD(tbl+BS_VolID, n);			/* VSN */
		ST_WORD(tbl+BPB_FATSz16, n_fat);	/* Number of sectors per FAT */
		tbl[BS_DrvNum] = 0x80;				/* Drive number */
		tbl[BS_BootSig] = 0x29;				/* Extended boot signature */
		memcpy(tbl+BS_VolLab, "NO NAME    " "FAT     ", 19);	/* Volume label, FAT signature */
	}
	ST_WORD(tbl+BS_55AA, 0xAA55);			/* Signature (Offset is fixed here regardless of sector size) */
	if (disk_write(drv, tbl, b_vol, 1) != RES_OK)/* Write original (VBR) */
		return FR_DISK_ERR;
	if (fmt == FS_FAT32)					/* Write backup (VBR+6) */
		disk_write(drv, tbl, b_vol + 6, 1);

	/* Initialize FAT area */
	wsect = b_fat;
	for (i = 0; i < N_FATS; i++) {
		memset(tbl, 0, SS(fs));			/* 1st sector of the FAT  */
		n = md;								/* Media descriptor byte */
		if (fmt != FS_FAT32) {
			n |= (fmt == FS_FAT12) ? 0x00FFFF00 : 0xFFFFFF00;
			ST_DWORD(tbl+0, n);				/* Reserve cluster #0-1 (FAT12/16) */
		} else {
			n |= 0xFFFFFF00;
			ST_DWORD(tbl+0, n);				/* Reserve cluster #0-1 (FAT32) */
			ST_DWORD(tbl+4, 0xFFFFFFFF);
			ST_DWORD(tbl+8, 0x0FFFFFFF);	/* Reserve cluster #2 for root dir */
		}
		if (disk_write(drv, tbl, wsect++, 1) != RES_OK)
			return FR_DISK_ERR;
		memset(tbl, 0, SS(fs));			/* Fill following FAT entries with zero */
		for (n = 1; n < n_fat; n++) {		/* This loop may take a time on FAT32 volume due to many single sector write */
			if (disk_write(drv, tbl, wsect++, 1) != RES_OK)
				return FR_DISK_ERR;
		}
	}

	/* Initialize root directory */
	i = (fmt == FS_FAT32) ? au : n_dir;
	do {
		if (disk_write(drv, tbl, wsect++, 1) != RES_OK)
			return FR_DISK_ERR;
	} while (--i);

	/* Create FSInfo record if needed */
	if (fmt == FS_FAT32) {
		ST_WORD(&tbl[BS_55AA], 0xAA55);
		ST_DWORD(&tbl[FSI_LeadSig], 0x41615252);
		ST_DWORD(&tbl[FSI_StrucSig], 0x61417272);
		ST_DWORD(&tbl[FSI_Free_Count], n_clst - 1);
		ST_DWORD(&tbl[FSI_Nxt_Free], 0xFFFFFFFF);
		disk_write(drv, tbl, b_vol + 1, 1);	
		disk_write(drv, tbl, b_vol + 7, 1);	
	}

	return (disk_ioctl(drv, CTRL_SYNC, (void*)0) == RES_OK) ? FR_OK : FR_DISK_ERR;
}

