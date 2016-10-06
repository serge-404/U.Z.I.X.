/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file  R0.04a   (C)ChaN, 2007
/-----------------------------------------------------------------------*/

#ifndef _DISKIO

#define _READONLY	0	/* 1: Read-only mode */

/* #include "integer.h" */
#define BYTE unsigned char	

/* Definitions corresponds to multi partition */

#define	_MULTI_PARTITION	1
/* When _MULTI_PARTITION is set to 0, each logical drive is bound to same
/  physical drive number and can mount only 1st primaly partition. When it is
/  set to 1, each logical drive can mount a partition listed in Drives[]. */

#if _MULTI_PARTITION != 0	/* Multiple partition cfg */

typedef struct _PARTITION {
	BYTE pd;				/* Physical drive # (0-255) */
	int /*BYTE*/  pt;		/* Partition # (0-3) */
#ifdef ORION_HOSTED
	unsigned long size;		/* Partition size (in LBA blocks) */
	unsigned long po;		/* Partition offset (LBA addr); 0=global/entire drive */
	unsigned int pc;		/* Partition offset checksumm */
	char  ptype;			/* Partition type */
	char  mediaid;			/* -1 = not opened, otherwise minor number */
#endif
} PARTITION;

#define LD2PD(drv) (Drives[drv].pd)	/* Get physical drive# */
#define LD2PT(drv) (Drives[drv].pt)	/* Get partition# */
#ifdef ORION_HOSTED
#define LD2PS(drv) (Drives[drv].size)	/* Get partition size */
#define LD2PO(drv) (Drives[drv].po)		/* Get partition offset */
#define LD2PC(drv) (Drives[drv].pc)		/* Get partition record checksumm */
#define LD2PY(drv) (Drives[drv].ptype)	/* Get partition type */
#endif

#else						/* Single partition cfg */

#define LD2PD(drv) (drv)		/* Physical drive# is equal to logical drive# */
#define LD2PT(drv) 0			/* Always mounts the 1st partition */
#ifdef ORION_HOSTED
#define LD2PS(drv) 0			/* Must be initialized */
#define LD2PO(drv) 0			/* Always whole disk */
#define LD2PC(drv) 66			/* magic number */
#define LD2PY(drv) 0			/* partition type */
#endif

#endif


/* Status of Disk Functions */
typedef unsigned char DSTATUS;
typedef unsigned char DRESULT;

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

#ifdef ORION_HOSTED

unsigned char init_drive(unsigned char drv);

#endif

DSTATUS disk_initialize (BYTE);
DSTATUS disk_status (BYTE);
#ifdef ORI_UZIX
DRESULT disk_read (BYTE, BYTE*, unsigned long, BYTE, BYTE);
#else
DRESULT disk_read (BYTE, BYTE*, unsigned long, BYTE);
#endif
#if	_READONLY == 0
#ifdef ORI_UZIX
DRESULT disk_write (BYTE, BYTE*, unsigned long, BYTE, BYTE);
#else
DRESULT disk_write (BYTE, BYTE*, unsigned long, BYTE);
#endif
#endif
DRESULT disk_ioctl (BYTE, BYTE, void*);
/*void	disk_timerproc (void);*/




/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl() */

#define GET_SECTOR_COUNT	1
#define GET_SECTOR_SIZE		2
#define CTRL_SYNC		3
/*
#define CTRL_POWER		4
#define CTRL_LOCK		5
#define CTRL_EJECT		6
#define MMC_GET_CSD		10
#define MMC_GET_CID		11
#define MMC_GET_OCR		12
#define ATA_GET_REV		20
#define ATA_GET_MODEL		21
#define ATA_GET_SN		22
*/
#define GET_PARAMS_STRUCT	65

#define _DISKIO
#endif
