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


#define BIOS_BOOT	0	/* �������� ����� �������                 */
#define BIOS_WBOOT	1	/* ������� ����� �������                  */
#define BIOS_CONST	2	/* ��������� �������                      */ 
#define BIOS_CONIN	3	/* ���� ������� � �������                 */
#define BIOS_CONOUT	4	/* ����� �� �������                       */
#define BIOS_LIST	5	/* ����� �� �������                       */ 
#define BIOS_PUNCH	6	/* ����� �� ����������                    */
#define BIOS_READER	7	/* ���� � �����������                     */
#define BIOS_HOME	8	/* ��������� ������� 0 �� ��������� ����� */
#define BIOS_SELDSK	9	/* ����� �����                            */
#define BIOS_SETTRK	10	/* ��������� ������ �������               */
#define BIOS_SETSEK	11	/* ��������� ������ �������               */
#define BIOS_SETDMA	12	/* ��������� ������ ���                   */
#define BIOS_READ	13	/* ������ ���������� �������              */
#define BIOS_WRITE	14	/* ������  ����������  �������            */
#define BIOS_LISTST	15	/* ��������� ��������                     */
#define BIOS_SECTRAN	16	/* ������������� ��������                 */

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

#define _IDEBDOS
#endif
