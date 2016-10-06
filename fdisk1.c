/*
 * Copyright (c) 2008 Serge 
 * All Rights Reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cpm.h>
#include "dsk.h"
#include "idebdos.h"
#include "integer.h"
#include "fdisk.h"

extern int in;				/* Source File Descriptor */
extern int out;				/* Destination File Descriptor */
extern int byte_tx;			/* Bytes Transfered */
extern BYTE buffer_in[sector_size];	/* Source Buffer */
extern BYTE buffer_out[sector_size];    /* Destination Buffer */
extern DWORD DriveSize[2];
extern DWORD dw3;
extern BYTE ParType;
extern char* OpenSrc;
char drvbuf[70];

/* 
#define	MEMSET(ptr,val,cnt)  for (ij=0; ij<cnt; ij++) (ptr)[ij]=(val) 
static int ij;
*/

BOOL GetCommit(char* msg)
{
  register int ii;
  printf("%s. Continue?(Y/N):", msg);
  ii=Inkey();
  return ((ii=='Y')||(ii=='y'));
}

#define	DEBUG(st) /* GetCommit(st) */

char* CalcKM(char* cbuf, DWORD value)
{
  value>>=1;
  if (value<1024)
    sprintf(cbuf, "%luK", value);
  else 
    sprintf(cbuf, "%luM", (value+512)/1024);
  return cbuf;
}

BOOL CheckPartitionTable(BYTE* tbl)  /* check for overloaded partitions */
{
  register int ii;
  int jj;
  DWORD iLBAbeg, iLBAend, jLBAbeg, jLBAend;

  for (ii=0; ii<4; ii++) {
    iLBAbeg = *((DWORD*)&tbl[16 * ii + 8]);
    iLBAend = iLBAbeg + *((DWORD*)&tbl[16 * ii + 12]) - 1;
    for (jj=0; jj<4; jj++)
      if ((ii!=jj) && (tbl[16 * jj + MBR_PART_TYPE])
                   && (tbl[16 * ii + MBR_PART_TYPE])) {
	jLBAbeg = *((DWORD*)&tbl[16 * jj + 8]);
	jLBAend = jLBAbeg + *((DWORD*)&tbl[16 * jj + 12]) - 1;
	if ( ( (iLBAbeg>=jLBAbeg) && (iLBAbeg<=jLBAend) ) ||
             ( (iLBAend>=jLBAbeg) && (iLBAend<=jLBAend) ) )
          return FALSE;
      }
  }
  return TRUE;
}

int SortPartitionTable(BYTE* ptab, BYTE* tmpbuf, DWORD pbeg)
{
  register int ii;
  int di, mi, resnum=-1, pi=0;
  DWORD pbegmin, pbegcur;
  memcpy(tmpbuf, ptab, 64);
  memset(ptab, 0, 64);
  for (di=0; di<4; di++) {
    mi=-1;
    pbegmin=0;  
    for (ii=0; ii<4; ii++) 
      if (tmpbuf[16 * ii + MBR_PART_TYPE]) { 
        pbegcur=*(DWORD*)&tmpbuf[16 * ii + 8];
        if ((pbegmin==0) || (pbegcur<pbegmin)) {
          pbegmin=pbegcur;
          mi=ii; 
        }
      } 
    if (mi>=0) {
      if (pbeg==pbegmin) resnum=pi;
      memcpy(&ptab[pi*16], &tmpbuf[mi*16], 16);
      pi++;
      tmpbuf[16 * mi + MBR_PART_TYPE] = 0;
    }
  }
  return resnum;
}

BOOL CpmBootValid(BYTE* buf, BYTE* crc)  /* check for CRC-protected CPM boot record */
{
  register int i;
  *crc=0x66;
  for (i=0; i<31; i++) *crc+=buf[i];
  return (*crc==buf[31]);
}

BOOL GetMBR(int drive, BYTE* buf)
{
  drvbuf[0]=(BYTE)drive+'0';
  drvbuf[1]=':';
  drvbuf[2]=0;
  in = dopen(drvbuf, O_RDONLY);
  if (in == -1) { 
    perror(OpenSrc); return FALSE;
  }
  byte_tx = pread(in, buf, sector_size);
  dclose(in);
  SortPartitionTable(&buf[bc_size], (BYTE*)&drvbuf[0], 0l); 
  return (byte_tx==sector_size); 
}

BOOL PutMBR(int drive, BYTE* buf)
{
  drvbuf[0]=(BYTE)drive+'0';
  drvbuf[1]=':';
  drvbuf[2]=0;
  in = dopen(drvbuf, O_RDWR);
  if (in == -1) { 
    perror(OpenSrc); return FALSE;
  }
  byte_tx = pwrite(in, buf, sector_size, 0);
  dclose(in);
  return (byte_tx==sector_size); 
}

TBootDPB BootDPB;

BYTE XorCRC(BYTE* buf, int cnt)
{
  register int ii;
  BYTE bb;
  for (ii=1, bb=buf[0]; ii<cnt; ii++)        /* calculate crc */
    bb^=buf[ii];
  return bb;
}

/* Creating CP/M filesystem with 16k block, 512 fcbs and 32k system offset */

#define FCBS_CNT	512		/* catalog size: 512 records */

BOOL CreateCpmFS(int drive, int pidx)
{
  BYTE bb;
  DWORD pbeg, psize=0;
  register int ii; 
  int nn=0, ne;
  if (! GetMBR(drive, buffer_out))
    return FALSE;
  for (ii=0; ii<4; ii++) {
    if (buffer_out[bc_size + 16 * ii + MBR_PART_TYPE]) nn++;
    if (nn==pidx) {
      pbeg=*(DWORD*)&buffer_out[16 * ii + bc_size + 8];		/* start lba */
      psize=*(DWORD*)&buffer_out[16 * ii + bc_size + 12];	/* size (lbas) */
      break;
    }
  }
  if (psize==0) return FALSE;
  if (disk_read((BYTE)drive, buffer_out, pbeg, 1) != RES_OK)
    return FALSE;
  if (CpmBootValid(buffer_out, &bb)) {
    strncpy(BootDPB.LABEL, (char*)&buffer_out[32], 16);
    BootDPB.LABEL[16]=0;
    if (buffer_out[1]!=0x30) BootDPB.LABEL[0]=0;
    sprintf(drvbuf, "Partition %d have valid filesystem, label:`%s`", 
            pidx, BootDPB.LABEL); 
    if (! GetCommit(drvbuf)) return FALSE;
  }
  memset(&BootDPB.JMP[0], 0, sizeof(TBootDPB));
  memset(buffer_out, 0, sector_size);

  strncpy(BootDPB.LABEL, "NEW FILESYSTEM  ", 16);
  buffer_out[0xFD]=XorCRC((BYTE*)&BootDPB.LABEL[0], 16);	/* calculate label crc */
  
  BootDPB.JMP[1]=0x30;
  BootDPB.SPT=256;			/* count of 128b sectors per track, track=32768 bytes */
  BootDPB.LEN1=2;			/* phisizal sector length = 512b */
  BootDPB.SEC=BootDPB.SPT>>2;		/* track size in 512b sectors */
  BootDPB.TRK=(WORD)(psize/BootDPB.SEC);     
  BootDPB.OFF=1;			/* 32768 bytes */
  BootDPB.BSH=7;
  BootDPB.BLM=127;			/* allocation_unit = 16384b */
  BootDPB.DSM=(WORD)((((DWORD)BootDPB.SPT)*(BootDPB.TRK-BootDPB.OFF))>>7)-1; /* 128/16384=1/128       */
  if (BootDPB.DSM<256)
    BootDPB.EXM=15; 
  else
    BootDPB.EXM=7;
  BootDPB.DRM=FCBS_CNT-1;		/* 16384 / 32 -1 */
  BootDPB.AL=0x80;			/* AL=one block, catalog size = 16384 */ 
  BootDPB.CKS=128;			/* fixed disk, by partitions can be remounted, so - removable :) */

  nn=BootDPB.SEC /* *BootDPB.OFF */ ;			/* catalog begin in 512b record offset*/
  ne=nn+(FCBS_CNT/16);					/* catalog end in 512b record offset*/

  CpmBootValid(&BootDPB.JMP[0], &BootDPB.CRC);		/* calculate bootsector crc */
  memcpy(&buffer_out[0], &BootDPB.JMP[0], 48);		/* Create BOOT sector */

  for (ii=1; ii<16; ii++) {			/* Create BOOT UserNames */
    sprintf(drvbuf, "USER_%d          ", ii);
    strncpy((char*)&buffer_out[ii*16 + 256], drvbuf, 16);
  }
  buffer_out[0xFE]=XorCRC(&buffer_out[256], 256);	/* calculate UserNames crc */

  if ((BootDPB.DSM>6400) && 
      (! GetCommit("Filesystem size>100M - it can overflow CP/M ALV buffer")))
    return FALSE;

/*write boot sector and UserNames */
  if (disk_write((BYTE)drive, buffer_out, pbeg, 1) != RES_OK)
    return FALSE;

/* write empty filedates */
  memset(buffer_out, 0, sector_size);
  if (disk_write((BYTE)drive, buffer_out, pbeg+1, 1) != RES_OK)
    return FALSE;
  if (disk_read((BYTE)drive, buffer_out, pbeg+2, 1) != RES_OK)
    return FALSE;
  memset(buffer_out, 0, 256);
  if (disk_write((BYTE)drive, buffer_out, pbeg+2, 1) != RES_OK)
    return FALSE;

/* erase catalog */
  memset(buffer_out, 0xE5, sector_size);
  for (ii=nn; ii<ne; ii++) 
    if (disk_write((BYTE)drive, buffer_out, pbeg+ii, 1) != RES_OK)
      return FALSE;
  return TRUE;
}

BOOL ActivatePartition(int drive, int pidx)
{
  register int ii;
  int nn=0;
  if (! GetMBR(drive, buffer_out))
    return FALSE;
  for (ii=0; ii<4; ii++) {
    if (buffer_out[bc_size + 16 * ii + MBR_PART_TYPE]) nn++;
    if (nn==pidx) 
      buffer_out[bc_size + 16 * ii] = 128;
    else
      buffer_out[bc_size + 16 * ii] = 00;
  }
  return PutMBR(drive, buffer_out);
}

BOOL DeletePartition(int drive, int pidx)
{
  register int ii;
  int nn=0;
  if (! GetMBR(drive, buffer_out))
    return FALSE;
  for (ii=0; ii<4; ii++) {
    if (buffer_out[bc_size + 16 * ii + MBR_PART_TYPE]) nn++;
    if (nn==pidx) 
      buffer_out[bc_size + MBR_PART_TYPE + 16 * ii] = 0;
  }
  return PutMBR(drive, buffer_out);
}

/* GetPartitionTable return:  
            1: Valid boot record but not an FAT (should be MBR), 
            2: The FAT boot record not partitioned,
            3: CP/M not partitioned, 
            4: other not partitioned, 
            0: error
*/

BYTE GetPartitionTable(int drive, BYTE* buf)
{
  BYTE crc;
  if (! GetMBR(drive, buf))
    return 0;
  if (*(WORD*)(&buf[BS_55AA]) != 0xAA55)	{	/* Check record signature (always offset 510) */
    if (CpmBootValid(buf, &crc))
      return 3;
    return 4;
  }
  if (!memcmp(&buf[BS_FilSysType], "FAT", 3))	/* Check FAT signature */
    return 2;
  if (!memcmp(&buf[BS_FilSysType32], "FAT32", 5) && !(buf[BPB_ExtFlags] & 0x80))
    return 2;
  return 1;
}

DWORD frees[12];

BOOL GetFreeSegment(int drive, 
                    BYTE* buf, 
                    DWORD size,		/* size in 512b blocks of min searched seg */
                    DWORD* pb,		/* start lba of min searched segment */
		    DWORD* maxsz,	/* size in 512b blocks of max available segment */
		    DWORD* totalfree)	/* total amount of free space (512b blocks) */
{
  DWORD pbeg, pend;			/* freesize-четные, freebeg-нечетные */
  register int ii;
  int freecnt=0, freeseg=-1; 
  if (ParType!=1) return FALSE;				/* not a MBR scheme   */
  *maxsz=0;
  *totalfree=0;
  frees[0]=0;
  frees[1]=1;						/* free segment start */

DEBUG("\n Step1");

  for (ii=0; ii<4; ii++) { 
    if (buf[16 * ii + MBR_PART_TYPE]) {			/* record free segment */
      pbeg=*(long*)&buf[16 * ii + 8];			/* start lba */
      pend=*(long*)&buf[16 * ii + 12]+pbeg-1;		/*  end  lba */
      if (frees[freecnt*2+1]<pbeg) {
        frees[freecnt*2]=pbeg-frees[freecnt*2+1];	/* free segment size */
        freecnt++;
	frees[freecnt*2]=0;				/* stop flag */
      }
      frees[freecnt*2+1]=pend+1;			/* next free segment start */
    }
  }

DEBUG("\n Step2");

  if ((frees[0]==0l)&&(frees[1]>=DriveSize[drive]-1l))
    return FALSE;					/* no free segments */

DEBUG("\n Step3");

  if (frees[freecnt*2+1]<DriveSize[drive]) {
    frees[freecnt*2]=DriveSize[drive]-frees[freecnt*2+1];
    frees[freecnt*2+2]=0;				/* stop flag */
  }

DEBUG("\n Step4");

  for (ii=0; (ii<10)&&(frees[ii]!=0l); ii+=2) {		/* search for lowest available free segment */

DEBUG("\n Step4.1");

    *totalfree+=frees[ii];
    if (frees[ii]>*maxsz) *maxsz=frees[ii];

DEBUG("\n Step4.2");

    if (size<=frees[ii]) {				/* segment OK */
      if (freeseg<0)
        freeseg=ii;					/* if first available */
      else
        if (frees[freeseg]>frees[ii]) freeseg=ii;	/* if lowest */
    }
  }

DEBUG("\n Step5");

  *pb=frees[freeseg+1];					/* free segment beg */
  return freeseg>=0;
}

/*
; The equations to convert from LBA to CHS follow:
;    TEMP = LBA % (HPC * SPT)
;     CYL = LBA / (HPC * SPT)
;    HEAD = TEMP / SPT
;    SECT = TEMP % SPT + 1
; Where:
;     LBA: linear base address of the block
;     CYL: value of the cylinder CHS coordinate
;     HPC: number of heads per cylinder for the disk
;    HEAD: value of the head CHS coordinate
;     SPT: number of sectors per track for the disk
;    SECT: value of the sector CHS coordinate
;    TEMP: buffer to hold a temporary value
*/

WORD CreatePartition(int drive, DWORD size /*in 512b blocks*/, BYTE ptype)
{
  BYTE pt, *buf;
  register int pidx=0;						/* partition slot */ 
  IdeDevParams params;
  WORD bh=0, bs=0, bc=0, eh=0, es=0, ec=0;
  DWORD pbeg, pend, temp;
  if ((pt=GetPartitionTable(drive, buffer_out))==0)	/* if read error */
    return FALSE;
  if (pt!=1)						/* if non-MBR scheme */
    return FALSE;
  if (! CheckPartitionTable(&buffer_out[bc_size]))	/* if wrong Partition Table*/
    return FALSE;
  while ((buffer_out[16 * pidx + bc_size + MBR_PART_TYPE])&&(pidx<4)) pidx++;
  if (pidx>=4) return FALSE;   				/* no free partition slots */
  if (! GetFreeSegment(drive, &buffer_out[bc_size], size, &pbeg, &temp, &dw3))
    return FALSE;					/* no available free space segment */
  pend=pbeg+size-1;
  if (disk_ioctl((BYTE)drive, GET_PARAMS_STRUCT, &params) != RES_OK)
    return FALSE;					/* IO error */
  buf = &buffer_out[16 * pidx + bc_size];
  if ((params.HmulS) && (params.MaxS)) {
    temp = pbeg % params.HmulS;
    bc = pbeg / params.HmulS;
    bh = temp / params.MaxS;
    bs = temp / params.MaxS + 1; 
    temp = pend % params.HmulS;
    ec = pend / params.HmulS;
    eh = temp / params.MaxS; 
    es = temp / params.MaxS + 1; 
  }
  FillPartitionBufCHS(buf, bh, bs, bc, eh, es, ec);
  buf[0] = 0;						/* nonactive */
  buf[MBR_PART_TYPE] = ptype;
  (*(DWORD*)&buf[8])  = pbeg;
  (*(DWORD*)&buf[12]) = pend-pbeg+1;  			/* psize */
  pidx=SortPartitionTable(&buffer_out[bc_size], buffer_in, pbeg); 
  return (CheckPartitionTable(&buffer_out[bc_size]) && PutMBR(drive, buffer_out)) ? pidx : 255;
}

BOOL SysgenCPM(int drive, int pidx, char* fname)	/* sysgen AltairDOS only! */
{
  BYTE bb;
  DWORD pbeg=0;
  register int ii;
  int syssec, nn=0;
  BYTE* buff;
  BOOL dates;
  if (! GetMBR(drive, buffer_out))
    return FALSE;
  for (ii=0; ii<4; ii++) {
    if (buffer_out[bc_size + 16 * ii + MBR_PART_TYPE]) nn++;
    if (nn==pidx) {
      pbeg=*(DWORD*)&buffer_out[16 * ii + bc_size + 8];		/* start lba */
      break;
    }
  }
  if (pbeg==0) return FALSE;
  if (disk_read((BYTE)drive, buffer_out, pbeg, 1) != RES_OK)	/* bootsec, usernames */
    return FALSE;
  if (! CpmBootValid(buffer_out, &bb)) {
    printf("\nError: Destination partition %d have not valid DPB (filesystem)\n", pidx);
    return FALSE;
  }
  buff=malloc(1025);
  if (! buff) return FALSE;
  if (disk_read((BYTE)drive, buff, pbeg+1, 2) != RES_OK) {	/* filedates */
    free(buff);
    return FALSE;
  }
  memcpy(&BootDPB, buffer_out, sizeof(BootDPB));
  syssec=(BootDPB.SPT * BootDPB.OFF)>>2;

  in = dopen(fname, O_RDONLY);
  if (in == -1) {
    perror(OpenSrc); free(buff); return FALSE;
  }
  buffer_in[31]=0;
  byte_tx=sector_size;
  for(ii=0; (ii<syssec)&&(byte_tx==sector_size); ii++) {
    byte_tx = dread(in, buffer_in, sector_size);
    if (ii==0) {
      if (! CpmBootValid(buffer_in, &bb)) {
        printf("Source have not valid DPB\n");
        free(buff); dclose(in);
        return FALSE;
      }
      for (nn=0x30; (nn<253) && (buffer_in[nn]!=0xF7); nn++);
      if (nn<253) {
        printf("Source is floppy-boot version");
        free(buff); dclose(in);
        return FALSE;
      }
      memcpy(&buffer_in[1], &buffer_out[1], 31);	/* save DPB, skip JMP operand */
      CpmBootValid(buffer_in, &buffer_in[31]);          /* recalculate DPB crc */ 

      if (XorCRC(&buffer_out[32], 16)==buffer_out[253])	{
        memcpy(&buffer_in[32], &buffer_out[32], 16);	/* save label */
        buffer_in[253]=buffer_out[253];			/* label crc */
      }
      printf("bootsector");
      if (XorCRC(&buffer_out[256], 256)==buffer_out[254]) {
        memcpy(&buffer_in[256], &buffer_out[256], 256);   /* usernames */
        buffer_in[254]=buffer_out[254];		 	  /* usernames crc */
        printf("..usernames");
      }
      dates=(XorCRC(buff, 0x300)==buffer_out[255]);
      if (dates) 
        buffer_in[255]=buffer_out[255];                   /* filedates crc */
    }
    else if (ii==1) {
      if (dates) {
        memcpy(buffer_in, buff, 512);   		/* filedates */
        printf(".filedates");
      }
    }
    else if (ii==2) {
      if (dates)
        memcpy(buffer_in, buff, 256);   		/* filedates */
      printf(".code");
    }
    if (disk_write((BYTE)drive, buffer_in, pbeg, 1) != RES_OK) {
      perror("\nError Write Sector"); crlf(); free(buff); return FALSE;
    }
    printf(".");
    pbeg++;
  }  
  dclose(in);
  free(buff);
  return TRUE;
}
