/*---------------------------------------------------------------------------/
/  Low level disk interface modlue include file for CP/M with IDEBDOS driver /
/                          (c) Serge, 2006                                   /
/---------------------------------------------------------------------------*/

#ifndef _IDEBDOS

#include "integer.h"

#define BGETVER 12	/* get OS version                                      */
#define BBASE	176   
#define BHOME   BBASE+0	/* sel LBA=0      (no params)                          */  
#define BSETDSK BBASE+1	/* set drive      (inp:E=drive;          out:HL=0->err)*/
#define BSETTRK BBASE+2	/* set cylinder   (inp:DE=cylinder;      out:nothing)  */
#define BSETSEC BBASE+3	/* set sector     (inp:E=sector, D=head; out:nothing)  */
#define BSETDMA BBASE+4	/* set buffer     (inp:DE=address;       out:nothing)  */
#define BREAD   BBASE+5	/* read sector    (inp:nothing;          out:A=0->ok)  */
#define BWRITE  BBASE+6	/* write sector   (inp:nothing;          out:A=0->ok)  */
#define BIOCTL  BBASE+7	/* get parametres (inp:DE=mode;          out:A=0->ok)  */
#define BGETDT  BBASE+8	/* get date       (inp:nothing; out:HL=MSDOS FAT date) */
#define BGETTM  BBASE+9	/* get time       (inp:nothing; out:HL=MSDOS FAT time) */
#define BSETDT  BBASE+10 /* set date      (inp:DE=MSDOS date;  out:nothing)	*/
#define BSETTM  BBASE+11 /* set time      (inp:DE=MSDOS time;  out:nothing)	*/
#define BMREAD  BBASE+12 /* read N sectors (inp:D=count,E=mempage; out:A=0->ok)	*/
#define BMWRITE BBASE+13 /* write sectors  (inp:D=count,E=mempage; out:A=0->ok)	*/
/* #define BSETISR	BBASE+14 /* 50Hz ISR hook (inp:DE=ISR address; out:nothing)	*/
			 /* b=bank_to_return - on interrupt passing into ISR	*/
			 /* all registers are stored on (bank_to_return) stack	*/
/* #define BRETISR BBASE+15 /* continue at bank_to_return (inp:E=bank; no out :) ) */

#define BIOS_BOOT	0	/* холодный старт системы                 */
#define BIOS_WBOOT	1	/* гор€чий старт системы                  */
#define BIOS_CONST	2	/* состо€ние консоли                      */ 
#define BIOS_CONIN	3	/* ввод символа с консоли                 */
#define BIOS_CONOUT	4	/* вывод на дисплей                       */
#define BIOS_LIST	5	/* вывод на принтер                       */ 
#define BIOS_PUNCH	6	/* вывод на магнитофон                    */
#define BIOS_READER	7	/* ввод с магнитофона                     */
#define BIOS_HOME	8	/* установка дорожки 0 на выбранном диске */
#define BIOS_SELDSK	9	/* выбор диска                            */
#define BIOS_SETTRK	10	/* установка номера дорожки               */
#define BIOS_SETSEK	11	/* установка номера сектора               */
#define BIOS_SETDMA	12	/* установка адреса пдп                   */
#define BIOS_READ	13	/* чтение выбранного сектора              */
#define BIOS_WRITE	14	/* запись  выбранного  сектора            */
#define BIOS_LISTST	15	/* состо€ние принтера                     */
#define BIOS_SECTRAN	16	/* перенумераци€ секторов                 */

#define IO_GET_PARAMS 0  /* BIOCTL: GET_DRIVE_PARAMS_BLOCK to DMA */

#ifndef PIOBYTE
#define PIOBYTE  ((BYTE *)0x0003)	/* IOBYTE: TTY=D1D0=00, CRT=D1D0=01 */
#endif

typedef struct {
  BYTE  DevMsk;      /* 0 for Master device, 16 for Slave          */
  BYTE  IsLBA;       /* 0=work in CHS mode, 2=Work in LBA mode     */ 
  BYTE  bbank;       /* caller (buffer) bank                       */
  WORD  Buffer;      /* 512bytes buffer address                    */
  union {
    DWORD lba;       /* current sector for LBA mode - BIG-ENDIAN   */
    struct {
      BYTE  sec;     /* sector (byte0 of 28bit LBA)                */  
      WORD  cyl;     /* cyllinder (byte1,2 of 28bit LBA)           */
      BYTE  head;    /* head (byte3[bit0..3] of 28bit LBA)         */
    } slba;
  } ul;
  WORD  MaxC;        /* Max cylinder value                         */
  BYTE  MaxH;        /* number of heads per cylinder for the disk  */
  WORD  MaxS;        /* number of sectors per track for the disk   */
  DWORD MaxLBA;      /* number of sectors (drive size) in LBA mode */
  WORD  HmulS;       /* internal specific variable = MaxH * MaxS   */
  BYTE  Ready;       /* 1=drive Ready, 0=Drive NotReady            */
  BYTE  ro;          /* ro=0 if read_write, ro=0FFh if read_only   */
  BYTE  recal;       /* recalibrate flag (for very old HDDs)       */
} IdeDevParams;

extern char  bdos(int, ...);
extern short bdoshl(int, ...);	/* bdos call returning value in hl */
extern char  bios(int, ...);

#define _IDEBDOS
#endif
