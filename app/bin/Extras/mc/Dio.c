/*----------------------------------------------------------/
/  Low level hard disk & time interface modlue  file  R0.0  /
/                        (c) Serge 2006                     /
/----------------------------------------------------------*/

#include "stdlib.h" 
#include "diskio.h"

#if _MULTI_PARTITION != 0	/* Multiple partition cfg */

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

DSTATUS disk_status(
  BYTE Drive           /* Physical drive number */
)
{
  if (! (LastError=bdoshl(BSETDSK, 0xff-Drive))) 
	return STA_NODISK;			/* drive not exists */
  else if (LastError==0xffff)
	return STA_PROTECT;  			/* drive exists, active, read-only */
  return 0;					/* drive exists, active, read-write */
}

DSTATUS disk_initialize(
  BYTE Drive              /* Physical drive number */
)
{
  return disk_status(Drive);
}

DRESULT disk_read (
  BYTE Drive,          /* Physical drive number      */
  BYTE* Buffer,        /* Pointer to the read buffer */
  DWORD SectorNumber,  /* Sector number to read from */
  BYTE SectorCount     /* Number of sectors to read  */
)
{
  register WORD sec;
  if ( (STA_PROTECT | disk_status(Drive))!=STA_PROTECT )
    return RES_NOTRDY;
  while (SectorCount) {
    ulba.lba=SectorNumber;
    sec=ulba.chs.h;
    sec=(sec<<8) | ulba.chs.s;
    bdos(BSETDMA, Buffer);
    bdos(BSETTRK, ulba.chs.c);
    bdos(BSETSEC, sec);
/*
    kprintf("\n disk_read: LBA=%lu, trk(c)=%d, sec(hs)=%d, @buf(DMA)=%d",
           SectorNumber, ulba.chs.c, sec, (WORD)Buffer); 
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
  BYTE Drive,          /* Physical drive number       */
  BYTE* Buffer,        /* Pointer to the write buffer */
  DWORD SectorNumber,  /* Sector number to write from */
  BYTE SectorCount     /* Number of sectors to write  */
)
{
  register WORD sec;
  if ( (STA_PROTECT | disk_status(Drive))!=STA_PROTECT )
    return RES_NOTRDY;
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

DWORD get_fattime()  /* 31-25: Year(0-127 +1980), 24-21: Month(1-12), 20-16: Day(1-31) */
{                    /* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
  ulba.dt.bdate=bdoshl(BGETDT, 0);
  ulba.dt.btime=bdoshl(BGETTM, 0);
  return ulba.lba;
}

/* #endif */

DRESULT disk_ioctl(
  BYTE Drive,          /* Physical drive number */
  BYTE Mode,	       /* Command number */
  void* Buffer         /* Pointer to the read buffer */
)
{
  register void* buff;
  DRESULT res=RES_ERROR;
  if ( (STA_PROTECT | disk_status(Drive))!=STA_PROTECT )
    return RES_NOTRDY;
  if (buff=malloc(512)) {
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
    free(buff);
    res=RES_OK; 
  }
  return res;
}

void disk_timerproc()
{
  return;
}
