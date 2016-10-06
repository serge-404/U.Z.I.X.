/*----------------------------------------------------------/
/  Low level hard disk & time interface modlue  file  R0.0  /
/                        (c) Serge 2006                     /
/----------------------------------------------------------*/

#include "uzix.h"
#ifdef ORION_HOSTED
/* #include "unix.h" */
/* #include "extern.h" */
#include "orion.h"
#else
#include <stdlib.h> 
#endif

#include "integer.h"
#include "dsk.h"
#include "idebdos.h"

#if _MULTI_PARTITION != 0	/* Multiple partition cfg */

#ifdef ORION_HOSTED

extern void* malloc(unsigned);
extern int   printf(char *, ...);

#define MBR_Table		446
#define MBR_PART_TYPE		4
#define MBR_PART_BEG		8
#define MBR_PART_SIZE		12

PARTITION Drives[MAXDRIV] = {
    {0, 0, 0l, 1l, 67, 0, -1},	/* Logical drive 0 ==> Physical drive 0, 1st partition */
    {1, 0, 0l, 1l, 68, 0, -1},	/* Logical drive 1 ==> Physical drive 1, 1st partition */
    {0, 1, 0l, 1l, 68, 0, -1},	/* Logical drive 2 ==> Physical drive 0, 2nd partition */
    {1, 1, 0l, 1l, 69, 0, -1},	/* Logical drive 3 ==> Physical drive 1, 2nd partition */
    {0, 2, 0l, 1l, 69, 0, -1},	/* Logical drive 4 ==> Physical drive 0, 3rd partition */
    {1, 2, 0l, 1l, 70, 0, -1},	/* Logical drive 5 ==> Physical drive 1, 3rd partition */
    {0, 3, 0l, 1l, 70, 0, -1},	/* Logical drive 6 ==> Physical drive 0, 4th partition */
    {1, 3, 0l, 1l, 71, 0, -1}, /* Logical drive 7 ==> Physical drive 1, 4th partition */
    {0,-1, 0l, 0l, 0, -1, -1},	/* Phisical drive 0==> Physical drive 0, entire volume */
    {1,-1, 0l, 0l, 0, -1, -1} 	/* Phisical drive 1==> Physical drive 1, entire volume */
};

#else
PARTITION Drives[] = {
    {0, 0},     /* Logical drive 0 ==> Physical drive 0, 1st partition */
    {1, 0},     /* Logical drive 1 ==> Physical drive 1, 1st partition */
    {0, 1},     /* Logical drive 2 ==> Physical drive 0, 2nd partition */
    {1, 1},     /* Logical drive 3 ==> Physical drive 1, 2nd partition */
    {0, 2},     /* Logical drive 4 ==> Physical drive 0, 3rd partition */
    {1, 2},     /* Logical drive 5 ==> Physical drive 1, 3rd partition */
    {0, 3},     /* Logical drive 6 ==> Physical drive 0, 4th partition */
    {1, 3}      /* Logical drive 7 ==> Physical drive 1, 4th partition */
};
#endif

#endif

union {
  DWORD lba;  /* DWORD in memory: sequential left-to-right by-byte from lower to higher (BIG-ENDIAN)*/
  struct {  
    BYTE s;
    WORD c;
    BYTE h; 
  } chs;
  struct {
    WORD btime;
    WORD bdate;
  } dt;
} ulba;

WORD LastError;
BYTE TotalDrives=0;

#ifdef ORION_HOSTED

BYTE *cbuff, *tbl;
WORD drv;

WORD record_checksum(
  BYTE Drive           /* Physical drive number */
)
{
  ulba.lba=LD2PO(Drive);
  LastError=ulba.dt.bdate+ulba.dt.btime;
  ulba.lba=LD2PS(Drive);
  return LastError+(int)LD2PD(Drive)+LD2PT(Drive)+ulba.dt.bdate+ulba.dt.btime+66;
}

#define Drive (drv & 1)		/* phisical drive index (0..1) */

BYTE init_drive(BYTE drv)	/* logical drive index (0..9) */
{
  if (! (cbuff=
#ifdef ORI_UZIX
			(BYTE*)TEMPDBUF
#else
			(BYTE*)malloc(512)
#endif
  )) return 0;
  if ((disk_ioctl(Drive, GET_SECTOR_COUNT, &LD2PS((MAXDRIV-2)+Drive)) == RES_OK) && 
      (LD2PS((MAXDRIV-2)+Drive)>1)) 
    {
		if (Drive+1>TotalDrives) TotalDrives=Drive+1;
		LD2PC((MAXDRIV-2)+Drive)=record_checksum((MAXDRIV-2)+Drive);
		if ((drv<MAXDRIV-2) && ( (STA_PROTECT | disk_status(drv))==STA_PROTECT )) {
/*  
kprintf("\ninit_drive(drv=%d)", drv);
*/ 
			LD2PS(drv)=0l;				/* Size=0 */
			LD2PO(drv)=1l;
			LD2PC(drv)=record_checksum(drv);
			bdos(BSETDMA, cbuff);
			bdos(BHOME);				/* MBR: LBA=0 */
			cbuff[510]=0;
			if ((!bdos(BREAD, 0)) &&     
				(*((unsigned short*)&cbuff[510])==0xAA55)) { /* 0xAA55 - standard MBR marker */
				tbl = &(cbuff[MBR_Table + (LD2PT(drv) * 16)]);
				LD2PY(drv)=tbl[MBR_PART_TYPE];
				if ((tbl[MBR_PART_TYPE]>1) && (tbl[MBR_PART_TYPE]<0x53)) { /*FAT,UZIX,CPM*/
					LD2PS(drv)=*((DWORD *)&tbl[MBR_PART_SIZE]);
					LD2PO(drv)=*((DWORD *)&tbl[MBR_PART_BEG]);
					LD2PC(drv)=record_checksum(drv);
				}
			}
		}	
    }
#ifndef ORI_UZIX
  free(cbuff);
#endif
  return (int)TotalDrives;
}
#undef Drive
#endif					/* ORION_HOSTED */

DSTATUS disk_status(
  BYTE Drive           /* Physical drive number */
)
{
#ifdef ORION_HOSTED
  if (record_checksum(Drive)!=LD2PC(Drive))	/* partition info record check */
	return STA_NODISK;			/* for memory crash protection */
#endif
  if (! (LastError=bdoshl(BSETDSK, 0xff-(Drive & 1)))) { 
/*
kprintf("\n\tdisk_status(Drive=%d):STA_NODISK\n", (int)Drive);
*/
	return STA_NODISK;			/* drive not exists */
  }	
  else if (LastError==0xffff)
	return STA_PROTECT;  			/* drive exists, active, read-only */
  return 0;					/* drive exists, active, read-write */
}

DSTATUS disk_initialize(
  BYTE Drive              /* Partition number - 0..7 */
)
{
  return disk_status(Drive);
}

DRESULT disk_read (
  BYTE Drive,          /* Physical drive number: 0,1=fat/fdisk drive, 0..7=uzix partition*/
  BYTE* Buffer,        /* Pointer to the read buffer */
  DWORD SectorNumber,  /* Sector number to read from */
  BYTE SectorCount     /* Number of sectors to read  */
#ifdef ORI_UZIX
 ,BYTE DstPage			/* Memory Page to read to; 0=no multyread*/
#endif
)
{
  register WORD sec;
  if ( (STA_PROTECT | disk_status(Drive))!=STA_PROTECT )
    return RES_NOTRDY;
#ifdef ORION_HOSTED
  if (SectorNumber+SectorCount>LD2PS(Drive)) return RES_ERROR;						/*partition bound*/
  SectorNumber+=LD2PO(Drive); 														/*partition offset*/
  if (SectorNumber+SectorCount>LD2PS((Drive & 1)+(MAXDRIV-2))) return RES_ERROR;	/*drive bound*/
#endif
#ifdef ORI_UZIX
  if (SectorCount && DstPage) {
    ulba.lba=SectorNumber;
    sec=ulba.chs.h;
    sec=(sec<<8) | ulba.chs.s;
    bdos(BSETDMA, Buffer);
    bdos(BSETTRK, ulba.chs.c);
    bdos(BSETSEC, sec);
/*
kprintf("disk_read: Drive:%d, DMA=%p, DE=%p\n", Drive, (WORD)Buffer, (((WORD)SectorCount)<<8) | DstPage);
*/
    if (bdos(BMREAD, (((WORD)SectorCount)<<8) | DstPage)) return RES_ERROR;     
  }
  else
#endif
  while (SectorCount) {
    ulba.lba=SectorNumber;
    sec=ulba.chs.h;
    sec=(sec<<8) | ulba.chs.s;
    bdos(BSETDMA, Buffer);
    bdos(BSETTRK, ulba.chs.c);
    bdos(BSETSEC, sec);
/*
	kprintf("disk_read: Drive:%d, LBA=%lu, DMA=%04x\n", Drive, SectorNumber, (WORD)Buffer);
*/
    if (bdos(BREAD, 0)) return RES_ERROR;     
    SectorNumber++;
    SectorCount--;
    Buffer += 512;
  }
  return RES_OK;     
}

/* #if _FS_READONLY != 0 */

DRESULT disk_write (
  BYTE Drive,          /* Physical drive number: 0,1=fat/fdisk drive, 0..7=uzix partition*/
  BYTE* Buffer,        /* Pointer to the write buffer */
  DWORD SectorNumber,  /* Sector number to write from */
  BYTE SectorCount     /* Number of sectors to write  */
#ifdef ORI_UZIX
 ,BYTE DstPage			/* Memory Page to read to; 0=no multyread*/
#endif
)
{
  register WORD sec;
  if ( (STA_PROTECT | disk_status(Drive))!=STA_PROTECT )
    return RES_NOTRDY;
#ifdef ORION_HOSTED
  if (SectorNumber+SectorCount>LD2PS(Drive)) return RES_ERROR;		/*partition bound*/
  SectorNumber+=LD2PO(Drive); 						/*partition offset*/
  if (SectorNumber+SectorCount>LD2PS((Drive & 1)+(MAXDRIV-2))) return RES_ERROR; /*drive bound*/
#endif
#ifdef ORI_UZIX
  if (SectorCount && DstPage) {
    ulba.lba=SectorNumber;
    sec=ulba.chs.h;
    sec=(sec<<8) | ulba.chs.s;
    bdos(BSETDMA, Buffer);
    bdos(BSETTRK, ulba.chs.c);
    bdos(BSETSEC, sec);
/*
kprintf("disk_write: Drive:%d, DMA=%p, DE=%p\n", Drive, (WORD)Buffer, (((WORD)SectorCount)<<8) | DstPage);
*/
    if (bdos(BMWRITE, (((WORD)SectorCount)<<8) | DstPage)) return RES_ERROR;     
  }
  else
#endif
  while (SectorCount) {
    ulba.lba=SectorNumber;
    sec=ulba.chs.h;
    sec=(sec<<8) | ulba.chs.s;
    bdos(BSETDMA, Buffer);
    bdos(BSETTRK, ulba.chs.c);
    bdos(BSETSEC, sec);
/*
    kprintf("\n disk_write: LBA=%lu, trk(c)=%d, sec(hs)=%d, @buf(DMA)=%d",
           SectorNumber, ulba.chs.c, sec, (WORD)Buffer); 
*/
    if (bdos(BWRITE, 0)) return RES_ERROR;     
    SectorNumber++;
    SectorCount--;
    Buffer += 512;
  }
  return RES_OK;     
}

#ifndef ORI_UZIX
DWORD get_fattime()  /* 31-25: Year(0-127 +1980), 24-21: Month(1-12), 20-16: Day(1-31) */
{                    /* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
  ulba.dt.bdate=bdoshl(BGETDT, 0);
  ulba.dt.btime=bdoshl(BGETTM, 0);
  return ulba.lba;
}
#endif

/* #endif */

DRESULT disk_ioctl(
  BYTE Drive,          /* Physical drive number */
  BYTE Mode,	       /* Command number */
  void* Buffer         /* Pointer to the read buffer */
)
{
  register void* buff;
  DRESULT res=RES_ERROR;
  if ( (STA_PROTECT | disk_status(Drive%2))!=STA_PROTECT )
    return RES_NOTRDY;
  if (buff=
#ifdef ORI_UZIX
	   TEMPDBUF
#else
	   malloc(512)
#endif
	) {
    ((IdeDevParams*)buff)->MaxLBA=0;
    bdos(BSETDMA, buff); 
    if (! bdos(BIOCTL, IO_GET_PARAMS))	/* 0 = read IDE device parameters block */
     switch (Mode) {
      case GET_SECTOR_COUNT: if (Buffer) *(DWORD*)Buffer=((IdeDevParams*)buff)->MaxLBA;
                             break;
      case GET_SECTOR_SIZE: if (Buffer) *(WORD*)Buffer=512;
                            break;
      case GET_PARAMS_STRUCT: if (Buffer) *(IdeDevParams*)Buffer=*(IdeDevParams*)buff;
      case CTRL_SYNC: break;
      default: ; 
     }
#ifndef ORI_UZIX
    free(buff);
#endif
    res=RES_OK; 
  }
  return res;
}

