/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file  R0.04a   (C)ChaN, 2007
/-----------------------------------------------------------------------*/

#ifndef _DISKIO

#define _READONLY	0	/* 1: Read-only mode */

#include "integer.h"


/* Definitions corresponds to multi partition */

#define	_MULTI_PARTITION	1
/* When _MULTI_PARTITION is set to 0, each logical drive is bound to same
/  physical drive number and can mount only 1st primaly partition. When it is
/  set to 1, each logical drive can mount a partition listed in Drives[]. */

#if _MULTI_PARTITION != 0	/* Multiple partition cfg */

typedef struct _PARTITION {
	BYTE pd;		/* Physical drive # (0-255) */
	WORD /*BYTE*/ pt;	/* Partition # (0-3) */
} PARTITION;

#define LD2PD(drv) (Drives[drv].pd)	/* Get physical drive# */
#define LD2PT(drv) (Drives[drv].pt)	/* Get partition# */

#else						/* Single partition cfg */

#define LD2PD(drv) (drv)		/* Physical drive# is equal to logical drive# */
#define LD2PT(drv) 0			/* Always mounts the 1st partition */

#endif


/* Status of Disk Functions */
typedef BYTE	DSTATUS;
typedef BYTE	DRESULT;

/* Results of Disk Functions */

/* +-serge*/

/*typedef enum { */
/*	RES_OK = 0,		 0: Successful */
/*	RES_ERROR,		 1: R/W Error */
/*	RES_WRPRT,		 2: Write Protected */
/*	RES_NOTRDY,		 3: Not Ready */
/*	RES_PARERR		 4: Invalid Parameter */
/*} DRESULT;*/

#define RES_OK			0		/* Successful */
#define	RES_ERROR		1		/* R/W Error */
#define	RES_WRPRT		2		/* Write Protected */
#define	RES_NOTRDY		3		/* Not Ready */
#define	RES_PARERR		4		/* Invalid Parameter */

/* serge*/

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS disk_initialize (BYTE);
DSTATUS disk_status (BYTE);
DRESULT disk_read (BYTE, BYTE*, DWORD, BYTE);
#if	_READONLY == 0
DRESULT disk_write (BYTE, BYTE*, DWORD, BYTE);
#endif
DRESULT disk_ioctl (BYTE, BYTE, void*);
void	disk_timerproc (void);




/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl() */

#define GET_SECTOR_COUNT	1
#define GET_SECTOR_SIZE		2
#define CTRL_SYNC		3
#define CTRL_POWER		4
#define CTRL_LOCK		5
#define CTRL_EJECT		6
#define MMC_GET_CSD		10
#define MMC_GET_CID		11
#define MMC_GET_OCR		12
#define ATA_GET_REV		20
#define ATA_GET_MODEL		21
#define ATA_GET_SN		22
#define GET_PARAMS_STRUCT	65

#define _DISKIO
#endif
