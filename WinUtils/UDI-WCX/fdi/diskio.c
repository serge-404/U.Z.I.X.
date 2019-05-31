/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "stdio.h"		/* file functions */

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_IMG         3       /* on-file image */

extern char DriveImage[];
extern DWORD PartitionOffset;
extern void* fHandle;	        /* opened file descriptor */
extern int file_exists(char* fname);
extern DWORD file_size(char* fname);

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
/*	case DEV_RAM :
		result = RAM_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		result = USB_disk_status();

		// translate the reslut code here

		return stat;
*/
        case DEV_IMG: {
            if (file_exists(DriveImage) && (file_size(DriveImage)>PartitionOffset))
              return 0;
          }
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
 /*	case DEV_RAM :
		result = RAM_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		result = MMC_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		result = USB_disk_initialize();

		// translate the reslut code here

		return stat;
*/
        case DEV_IMG: {
             if (fHandle=fopen(DriveImage, "r+b"))
               return 0;
          }
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
/*	case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
*/
        case DEV_IMG: {
            if (fseek(fHandle, sector*512, SEEK_SET)) return STA_NOINIT;
            return fread(buff, 512, count, fHandle)!=count;
          }
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
/*	case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
*/
        case DEV_IMG: {
            if (fseek(fHandle, sector*512, SEEK_SET)) return STA_NOINIT;
            return fwrite(buff, 512, count, fHandle)!=count;
          }
	}
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
/*	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		// Process of the command for the MMC/SD card

		return res;

	case DEV_USB :

		// Process of the command the USB drive

		return res;
*/
        case DEV_IMG: {
            switch (cmd) {
              case CTRL_SYNC: return 0;
              case GET_SECTOR_COUNT: {
                *((LPDWORD)buff)=file_size(fHandle)>>9;  /*  /512 */
                return *((LPDWORD)buff) ? 0 : STA_NOINIT;
              }
              case GET_SECTOR_SIZE: {
                *((LPDWORD)buff)=512;
                return 0;
              }
              case GET_BLOCK_SIZE: {
                *((LPDWORD)buff)=1;
                return 0;
              }
              case CTRL_TRIM: return 0;
              default: return 0;
            }
	}
    }
    return RES_PARERR;
}

DWORD get_fattime()  /* 31-25: Year(0-127 +1980), 24-21: Month(1-12), 20-16: Day(1-31) */
{                    /* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
  SYSTEMTIME Systime;
  union {
    DWORD wdt;
    struct {
      WORD btime;
      WORD bdate;
    } dt;
  } udt;
  GetSystemTime(&Systime);

  udt.dt.bdate=(Systime.wYear-1980)<<9 |
               Systime.wMonth<<5 |
               Systime.wDay;
  udt.dt.btime=Systime.wHour<<11 |
               Systime.wMinute<<5 |
               Systime.wSecond>1;
  return udt.wdt;
}

