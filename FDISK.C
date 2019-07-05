/*
 * Copyright (c) 2004 Plan B 
 * All Rights Reserved.
 *   Montreal, Quebec
 *   plan_b@videotron.ca,
 *
 *   Serge A. (c) 2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "integer.h"
#include "dsk.h"
#include "fdisk.h"	/* subroutines added by serge */

int in;			/* Source File Descriptor */
int out;		/* Destination File Descriptor */
int pte;		/* Partition Table Entry Offset */
int byte_tx;		/* Bytes Transfered */
int part_dec;		/* Partition Table Entry Decimal Value */
int type_dec;			/* Partition Type Decimal Value */
BYTE buffer_in[sector_size];	/* Source Buffer */
BYTE buffer_out[sector_size];	/* Destination Buffer */
int   dfilecnt = 0;
DFILE dfile[_DRIVES]={{0,NULL,-1,0,0,0},{0,NULL,-1,0,0,0}};

BOOL AltairDos=FALSE;

char fat[]="FAT";
char cpm[]="CP/M";
char uzix[]="UZIX";
char empty[]="empty";
char other[]="Other";
char act[]="is ";
char nonact[]="non";
char un[]="un";
char RdSrcSec[]="\nRead Source Sector\n";
char WrSrcSec[]="\nWrite Source Sector\n";
char PartNum[]="\nPartition Number\n";
char PartRange[]="\nPartition Range: 1-4\n\n";
char OpenSrc[]="\nOpen Source\n";
char OpenDest[]="\nOpen Destination\n";
char RdDestSec[]="\nRead Destination Sector\n";
char WrDestSec[]="\nWrite Destination Sector\n";

unsigned short level;
unsigned short sublevel;

int getopt_argc(int argc, char **argv);
int pt_copy(char **argv);
int bc_print(char **argv);
int pt_print(char **argv);
int p_activate(char **argv);
int p_type(char **argv);
int ptbe_edit(char **argv);
int ptle_edit(char **argv);
void pt_list();
void usage();

unsigned short OsLevel(unsigned short *SubLevel)
{
#asm
  push	de
  push	bc
  ld	C,12
  call	5
  ld	(_level),hl
  exx
  ld	(_sublevel),hl
  pop	bc
  pop	de
#endasm
  *SubLevel=sublevel;
  return level;
}

main(int argc, char **argv)
{
  register int c = 0;
  int flag = 1;
  OsLevel(&sublevel);
  AltairDos=(sublevel>0xab30)&&(sublevel<0xabff);
  if (argc == 1) { flag = 0; }
  for (c = 0; c != argc; c++) {
    if ((flag == -1) || (flag == 0)) { break; }
    if (argv[c][0] == '-') {
      flag = -1;
      switch (UpCase(argv[c][1])) {
        case 'U':
        case 'H':
        case '?': { usage(); flag = 1; exit (flag); }
      }
      CheckIDE();
      if ((UpCase(argv[c][1]) == 'A') && (getopt_argc(argc - c, &argv[c]) == 3)) {
        p_activate(&argv[c + 1]); flag = 1; }
      if ((UpCase(argv[c][1]) == 'B') && (getopt_argc(argc - c, &argv[c]) == 3)) {
        bc_copy(argv[c + 1], argv[c + 2], FALSE, bc_size); flag = 1; }
      if ((UpCase(argv[c][1]) == 'T') && (getopt_argc(argc - c, &argv[c]) == 3)) {
        pt_copy(&argv[c + 1]); flag = 1; }
      if ((UpCase(argv[c][1]) == 'C') && (getopt_argc(argc - c, &argv[c]) == 9)) {
        ptbe_edit(&argv[c + 1]); flag = 1; }
      if ((UpCase(argv[c][1]) == 'L') && (getopt_argc(argc - c, &argv[c]) == 5)) {
        ptle_edit(&argv[c + 1]); flag = 1; }
      if ((UpCase(argv[c][1]) == 'I') /* && (getopt_argc(argc - c, &argv[c]) == 1)*/ ) {
        pt_list(); flag = 1; }
      if ((UpCase(argv[c][1]) == 'D') && (getopt_argc(argc - c, &argv[c]) == 2)) {
        bc_print(&argv[c + 1]); flag = 1; }
      if ((UpCase(argv[c][1]) == 'P') && (getopt_argc(argc - c, &argv[c]) == 2)) {
        pt_print(&argv[c + 1]); flag = 1;}
      if ((UpCase(argv[c][1]) == 'S') && (getopt_argc(argc - c, &argv[c]) == 2)) {
        pt_sign(argv[c + 1], TRUE); flag = 1; } 
      if ((UpCase(argv[c][1]) == 'M') && (getopt_argc(argc - c, &argv[c]) == 4)) {
        p_type(&argv[c + 1]); flag = 1; }
      if ((UpCase(argv[c][1]) == 'G') && (getopt_argc(argc - c, &argv[c]) == 4)) {
        cpm_copy(&argv[c + 1]); flag = 1; }
    }
  }
  if (flag == -1) { usage(); }
  if (flag == 0)  { xmenu(); flag = 1; }
  exit (flag);
}

void crlf()
{
  printf("\n");
}

int getopt_argc(int argc, char **argv) {

  register int c = 1;
  int opt_argc = argc;

  while (opt_argc != 1) {
    if (argv[c][0] == '-') { break; }
    else { opt_argc--; c++; }
  }
  return (c);
}

int p_activate(char **argv) {

  register int c;

  part_dec = atoi(argv[1]);
  if (part_dec == -1) {
    perror(PartNum); exit (-1); }
  if ((part_dec < 1) || (part_dec > 4)) {
    printf(PartRange); exit (-1); }

  in = dopen(argv[0], O_RDWR);
  if (in == -1) {
    perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }

  for (c = 0; c < 4; c++) { pte = bc_size + 16 * c; buffer_in[pte] = 00; }

  pte = bc_size + 16 * (part_dec - 1);
  buffer_in[pte] = 128;

  byte_tx = pwrite(in, buffer_in, sector_size, 0);
  if (byte_tx != sector_size) {
    perror(WrSrcSec); exit (-1); }
  dclose(in);

  printf("\nPartition %d of %s activated.\n\n", part_dec, argv[0]);
}


BOOL bc_copy_mem(BYTE* buf, char* devname, BOOL do_sign, int count)
{
  out = dopen(devname, O_RDWR|O_CREAT /*, S_IRUSR|S_IWUSR*/);
  if (out == -1) { perror(OpenDest); return FALSE; }

  byte_tx = pread(out, buffer_out, sector_size);
  if (byte_tx == -1) {
    perror(RdDestSec); return FALSE; }

  memcpy(buffer_out, buf, count);
  if (do_sign) {
    buffer_out[510] = 85;     /* 55 */
    buffer_out[511] = 170;    /* AA */
  }
  byte_tx = pwrite(out, buffer_out, sector_size, 0);
  dclose(out);
  return (byte_tx == sector_size);
}

int bc_copy(char* srcdev, char* dstdev, BOOL do_sign, int count)
{
  in = dopen(srcdev, O_RDONLY);
  if (in == -1) { perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }

  if (! bc_copy_mem(buffer_in, dstdev, do_sign, count)) {
    perror(WrDestSec); exit(-1); }
  dclose(in);

  printf("\nMBR Code transfered from %s to %s\n\n", srcdev, dstdev);
}

int pt_copy(char **argv) {

  in = dopen(argv[0], O_RDONLY);
  if (in == -1) {
    perror(OpenSrc); exit (-1); }

  out = dopen(argv[1], O_RDWR|O_CREAT /*, S_IRUSR|S_IWUSR*/);
  if (out == -1) {
    perror(OpenDest); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }

  byte_tx = pread(out, buffer_out, sector_size);
  if (byte_tx == -1) {
    perror(RdDestSec); exit (-1); }

  memcpy(buffer_out + bc_size, buffer_in + bc_size, pt_size);

  byte_tx = pwrite(out, buffer_out, sector_size, 0);
  if (byte_tx != sector_size) {
    perror(WrDestSec); exit (-1); }
  dclose(in);
  dclose(out);

  printf("\nPartition Table transfered from %s to %s\n\n", argv[0], argv[1]);
}

int ptbe_edit(char **argv) {

  WORD bc_dec;           /* Beginning Cylinder Decimal Value */
  WORD bh_dec;           /* Beginning Head     Decimal Value */
  WORD bs_dec;           /* Beginning Sector   Decimal Value */
  WORD ec_dec;           /* Ending    Cylinder Decimal Value */
  WORD eh_dec;           /* Ending    Head     Decimal Value */
  register WORD es_dec;  /* Ending    Sector   Decimal Value */

  part_dec = atoi(argv[1]);
  if (part_dec == -1) { perror(PartNum); exit (-1); }
  if ((part_dec < 1) || (part_dec > 4)) {
    printf(PartRange); exit (-1); }
  
  bc_dec = atoi(argv[2]);
  if (bc_dec == -1) { perror(PartNum); exit (-1); }
  ec_dec = atoi(argv[5]);
  if (bh_dec == -1) { perror(PartNum); exit (-1); }
  if ((bc_dec > 1024) || (ec_dec > 1024)) {
    printf("\nMaximum Cylinder Value: 1024\n\n"); exit (-1); }

  bh_dec = atoi(argv[3]);
  if (bc_dec == -1) { perror(PartNum); exit (-1); }
  eh_dec = atoi(argv[6]);
  if (eh_dec == -1) { perror(PartNum); exit (-1); }
  if ((bh_dec > 255) || (eh_dec > 255)) {
    printf("\nMaximum Head Value: 255\n\n"); exit (-1); }
 
  bs_dec = atoi(argv[4]);
  if (bs_dec == -1) { perror(PartNum); exit (-1); }
  es_dec = atoi(argv[7]);
  if (es_dec == -1) { perror(PartNum); exit (-1); }
  if ((bs_dec > 63) || (es_dec > 63)) {
    printf("\nMaximum Sector Value: 63\n\n"); exit (-1); }

  in = dopen(argv[0], O_RDWR);
  if (in == -1) { perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }

  pte = bc_size + 16 * (part_dec -1);

  FillPartitionBufCHS(&buffer_in[pte], bh_dec, bs_dec, bc_dec, eh_dec, es_dec, ec_dec);

  byte_tx = pwrite(in, buffer_in, sector_size, 0);
  if (byte_tx != sector_size) {
    perror(WrSrcSec); exit (-1); }
  dclose(in);

  printf("\nPartition %d of %s CHS parameters changed to:\n\n"
       "\tBeginning:\n"
       "\t\tCylinder: %lu\n"
       "\t\tHead:\t%lu\n"
       "\t\tSector:\t%lu\n"
       "\tEnding:\n"
       "\t\tCylinder: %lu\n"
       "\t\tHead:\t%lu\n"
       "\t\tSector:\t%lu\n\n",
       part_dec, argv[0], bc_dec, bh_dec, bs_dec, ec_dec, eh_dec, es_dec);
}

int ptle_edit(char **argv) {

  DWORD slba_dec;	/* Starting LBA Sector Decimal Value */
  DWORD lbas_dec;	/* LBA Size in Sectors Decimal Value */
  DWORD n_part;		/* Partitions quantity (disk size)   */

  part_dec = atoi(argv[1]);
  if (part_dec == -1) { perror(PartNum); exit(-1); }
  if ((part_dec < 1) || (part_dec > 4)) {
    printf(PartRange); exit (-1); }
  slba_dec = atol(argv[2]);
  if (slba_dec == -1) { perror("\nStarting LBA"); crlf(); exit(-1); }
  if (slba_dec > 1000000000) {
    printf("\nMaximum Starting LBA Value: 1000000000\n\n"); exit(-1); }
  lbas_dec = atol(argv[3]);
  if (lbas_dec == -1) { perror("\nLBA Size"); crlf(); exit(-1); }
  if (lbas_dec > 1000000000) {
    printf("\nMaximum LBA Size value: 1000000000\n\n"); exit (-1); }

  if (disk_ioctl(atoi(argv[0]) & 1, GET_SECTOR_COUNT, &n_part) != RES_OK)
    { perror("\nGet Max source size"); crlf(); exit (-1); }

  if (lbas_dec+slba_dec>n_part) { 
    printf("\nPartition upper border (maxLBA=%lu) exceeds disk limit (%lu sectors)\n", lbas_dec+slba_dec, n_part); exit (-1); }

  in = dopen(argv[0], O_RDWR);
  if (in == -1) { perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }

  pte = bc_size + 16 * (part_dec - 1);
  buffer_in[pte +  8] =  slba_dec & 255;
  buffer_in[pte +  9] = (slba_dec & 65280) >> 8;
  buffer_in[pte + 10] = (slba_dec & 16711680) >> 16;
  buffer_in[pte + 11] =  slba_dec >> 24;
  buffer_in[pte + 12] =  lbas_dec & 255;
  buffer_in[pte + 13] = (lbas_dec & 65280) >> 8;
  buffer_in[pte + 14] = (lbas_dec & 16711680) >> 16;
  buffer_in[pte + 15] =  lbas_dec >> 24;

  byte_tx = pwrite(in, buffer_in, sector_size, 0);
  if (byte_tx != sector_size) {
    perror(WrSrcSec); exit (-1); }
  dclose(in);

  printf("\nPartition %d of %s LBA parameters have been changed to:\n\n"
       "\tStarting LBA:\t%lu\n"
       "\tLBA Size:\t%lu\n\n",
       part_dec, argv[0], slba_dec, lbas_dec);
}

void pt_hexdump(BYTE* buf, int drive)
{
  register int c;
  int bc=0, cc=0;
  crlf();
  for (c = bc_size; c<bc_size+64; c++) {
    if (!bc) printf("%1d : ", cc + drive);
    printf("%02x", buf[c]);
    switch (bc) {
      case 0:   
      case 3:
      case 4:
      case 7:
      case 11:
      case 15:	printf(" ");
		break;
    }
    bc++;
    if (bc == 16) {
      bc=0; cc+=2;
      crlf();
    }
  }
}

void bc_hexdump(BYTE* buf, int count)
{
  int c;
  register int bc=0;
  char txt[17];
  crlf();
  for (c = 0; c < count; c++) {
    printf("%02x ", buf[c]);
    if ((char)buf[c]>' ') 
      txt[bc]=(char)buf[c];
    else 
      txt[bc]='.';
    bc++;
    if (bc == 16) {
      txt[16]=0;
      bc=0;
      printf(" %s\n", txt);
    }
  }
  if (bc) {
    for (c = bc; c < 16; c++) printf("   ");
    printf(" %s\n", txt);
  }
}

int bc_print(char **argv) {

  in = dopen(argv[0], O_RDONLY);
  if (in == -1) { perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }
  bc_hexdump(buffer_in, bc_size);
  dclose(in);
}

char* TextPartType(BYTE pt)
{
  switch (pt){
	case 0x01:
	case 0x04:
	case 0x06:
	case 0x0B:
	case 0x0C:
	case 0x0E: 
	case 0x0F:
	case 0x11:
	case 0x14:
	case 0x16:
	case 0x1B:
	case 0x1C:
	case 0x1E: return &fat[0];
	case 0x21: return &uzix[0];
	case 0x52: return &cpm[0];
	case 0x00: return &empty[0];
	default:   return &other[0];
  }
}

int pt_print(char **argv) {

  register int c;

  in = dopen(argv[0], O_RDONLY);
  if (in == -1) { perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }
  pt_hexdump(buffer_in, atoi(argv[0]));
  for (c = 0; c!= 4; c++) {
    pte = bc_size + 16 * c;
    printf("\nPartition%2d %sactive=%02x, type %02x=%s,"
      " LBA start %lu and LBA size %lu\n",
      c + 1,
      (buffer_in[pte]==0x80) ? act : nonact,
      buffer_in[pte],
      buffer_in[pte + 4],
      TextPartType(buffer_in[pte + 4]),
/*      buffer_in[pte + 8] + (buffer_in[pte + 9] * 256)
        + (buffer_in[pte + 10] * 65536)
        + (buffer_in[pte + 11] * 16777216),  */
      *((DWORD*)&buffer_in[pte + 8]),
/*      buffer_in[pte + 12] + (buffer_in[pte + 13] * 256)
        + (buffer_in[pte + 14] * 65536)
        + (buffer_in[pte + 15] * 16777216)); */
      *((DWORD*)&buffer_in[pte + 12]));

    printf("\tBeginning: %4d cylinder, %3d head, %2d sector\n",
      ((buffer_in[pte + 2] & 0xc0) << 2) + buffer_in[pte + 3],
        buffer_in[pte + 1], (buffer_in[pte + 2] & 0x3f));

    printf("\tEnding:    %4d cylinder, %3d head, %2d sector\n",
      ((buffer_in[pte + 6] & 0xc0) << 2) + buffer_in[pte + 7],
        buffer_in[pte + 5], (buffer_in[pte + 6] & 0x3f));
  }
  dclose(in);
}

int p_type(char **argv) {

  part_dec = atoi(argv[1]);
  if (part_dec == -1) { perror(PartNum); exit(-1); }
  if ((part_dec < 1) || (part_dec > 4)) {
    printf(PartRange); exit (-1); }

  type_dec = atoi(argv[2]);
  if ((type_dec == -1) ||
      (type_dec > 255)) {
    printf("\nWrong Partition Type.\n\n"); pt_list(); exit (-1); }

  in = dopen(argv[0], O_RDWR);
  if (in == -1) { perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }

  pte = bc_size + 16 * (part_dec - 1);
  buffer_in[pte + 4] = type_dec;

  byte_tx = pwrite(in, buffer_in, sector_size, 0);
  if (byte_tx != sector_size) {
    perror(WrSrcSec); exit (-1); }
  dclose(in);

  printf("\nPartition %d of %s type has been changed to: %d.\n\n",
    part_dec, argv[0], type_dec);
}

int pt_sign(char* srcdev, BOOL do_sign) {

  in = dopen(srcdev, O_RDWR);
  if (in == -1) { perror(OpenSrc); exit (-1); }

  byte_tx = pread(in, buffer_in, sector_size);
  if (byte_tx != sector_size) {
    perror(RdSrcSec); exit (-1); }

  if (do_sign) {
    buffer_in[510] = 85;     /* 55 */
    buffer_in[511] = 170;    /* AA */
  }
  else {
    buffer_in[510] = 255;    /* FF */
    buffer_in[511] = 255;    /* FF */
  }
  byte_tx = pwrite(in, buffer_in, sector_size, 0);
  if (byte_tx != sector_size) {
    perror(WrSrcSec); exit (-1); }
  dclose(in);
  printf("\nMBR of %s has been %ssigned.\n\n", srcdev, do_sign ? &un[2] : un);
  return TRUE;
}

void pt_list() {
  printf("\nPartition types (HEX):\n\n");
  printf("00 Empty\t\t\t\tFF Xenix bad block table\n"
    "01 12-bit FAT primary partition\t\t5C Priam EDISK\n"
    "02 XENIX root file system\t\t61 SpeedStor\n"
    "03 XENIX /usr file system\t\t63 Unix System V\n"
    "04 16-bit FAT primary partition\t\t64 Novell Netware 286\n"
    "05 Extended partition\t\t\t65 Novell Netware 386\n"
    "06 BIGDOS FAT primary partition\t\t69 Novell Netware 5+\n"
    "07 NTFS primary partition\t\t82 Linux swap\n"
    "08 AIX boot, OS/2 (v1.0-1.3), C128 DOS\t83 Linux native partition\n"
    "09 AIX data, Coherent filesystem\t85 Linux extended partition\n" 
    "0A OS/2 Boot Mgr Coherent swap, OPUS\t86 NTFS volume set\n"
    "0B Windows95 with 32-bit FAT\t\t87 NTFS volume set\n");
  printf(
    "0C Windows95 with 32-bit FAT (LBA)\t8b FAT32 volume\n"
    "0E LBA VFAT (same as 06h but LBA)\t8c FAT32 volume\n"
    "0F LBA VFAT (same as 05h but LBA)\tA0 Laptop hibernation partition\n"
    "1B hidden Windows95 FAT32 partition\tA5 NetBSD, FreeBSD, 386BSD\n"
    "1C hidden Windows95 FAT32 part. (LBA)\tA6 OpenBSD\n"
    "1E hidden LBA VFAT partition\t\tA9 NetBSD\n"
    "21 UZIX filesystem\t\t\tBE Solaris boot partition\n"
    "40 VENIX 80286\t\t\t\tC0 CTOS\n"
    "51 NOVELL\t\t\t\tD8 CP/M-86\n"
    "52 CP/M\t\t\t\t\tDB Concurrent CP/M\n");
}

void usage() {
  printf("\n"
    "fdisk - Fixed disks Partition Manager\n"
    "\n"
    "SYNOPSYS\n"
    "\tfdisk [-a:b:t:c:l:h:i:d:p:s:m:g]"
    " [source] [destination] [partition]\n"
    "\t  [type] [count] [CHS start, CHS end] [LBA start, LBA size]\n"
    "\n"
    "\t-a\tActivate [partition] on [source].\n"
    "\t-b\tCopy Boot Code from [source] to [destination].\n"
    "\t-t\tCopy Partition Table from [source] to [destination].\n"
    "\t-c\tEdit [partition] CHS parameters on [source].\n"
    "\t-l\tEdit [partition] LBA parameters on [source].\n"
    "\t-h , -u\tPrint help.\n"
    "\t-i\tPrint Partition Type list.\n"
    "\t-d\tDump Boot Code from [source].\n"
    "\t-p\tPrint Partition Table from [source].\n"
    "\t-s\tSign the Partition Table.\n"
    "\t-m\tModify [source] [partition] [type]\n"
    "\t-g\tCopy from [source] to [destination] [count] 512b sectors.\n"
    "[source], [destination] is a file (U:D:FILENAME.EXT) or drive (0:, 1:, ...7:),\n"
    " where U=user; D=disk; 0,2,4,6=MasterIDE partitions; 1,3,5,7=SlaveIDE partitions\n");
}

int cpm_copy(char **argv)	/* copy from [Source] to [Destination] [count] sectors */
{
  register int i;
  int cnt = atoi(argv[2]);
  in = dopen(argv[0], O_RDONLY);
  if (in == -1) {
    perror(OpenSrc); exit (-1); }
  out = dopen(argv[1], O_RDWR|O_CREAT);
  if (out == -1) {
    perror(OpenDest); exit (-1); }
  if ((dfile[out].IsDrv) && (cnt>dfile[out].LBAsize)) {	/* destination partition upper limit */
    perror("\nNo Destination partition space"); crlf(); exit (-1); }
  for (i=1; i<=cnt; i++) {
    printf("\0x0d %d", i);
    byte_tx = dread(in, buffer_in, sector_size);
    if (byte_tx != sector_size) {
      perror(RdSrcSec); exit (-1); }

    byte_tx = dwrite(out, buffer_in, sector_size, 0);
    if (byte_tx != sector_size) {
      perror(WrDestSec); exit (-1); }
  }
  dclose(in);
  dclose(out);
  printf("\n%d 512b sectors has been copied from %s to %s\n\n",cnt, argv[0], argv[1]);
}

int dopen(char* path, int mode)  /* return dfile handle or -1 if error*/
{                                
	register int drv;
	DSTATUS stat;
	char *p = path;
	BYTE* buff;
        dfilecnt=0;
        while (((dfile[dfilecnt].fileCPM!=NULL) ||
                (dfile[dfilecnt].drive!=-1)) && (dfilecnt<_DRIVES))
          dfilecnt++;
	if (dfilecnt>=_DRIVES) return -1;

	/* Get drive number from the path name */
	while (*p == ' ') p++;		/* Strip leading spaces */
	drv = p[0] - '0';		/* Is there a drive number? */
	if (drv <= 9 && p[1] == ':')
		p += 2;			/* Found a drive number, get and strip it */
	else
		drv = -1;		/* No drive number is given */
	if (*p == '/') p++;		/* Strip heading slash */

	/* Check if the drive number is valid or not */
	if ((drv>=0) && (drv < 8)) {		/* Is the drive number valid? */
		if ( (STA_PROTECT | (stat=disk_status(drv & 1)))!=STA_PROTECT )
			return -1;		/* not ready */
		if ((!(mode & O_RDWR)) && (stat & STA_PROTECT))	
			return -1;		/* Check write protection */
		dfile[dfilecnt].fileCPM = NULL;
		dfile[dfilecnt].IsDrv = TRUE;
                dfile[dfilecnt].drive = drv;
		if (! (buff=malloc(sector_size+1)) ) return -1;
		byte_tx = pread(dfilecnt, buff, sector_size);
		if (byte_tx != sector_size) {
			perror(RdSrcSec); free(buff); return -1; }
		dfile[dfilecnt].LBAbeg = *((DWORD*)&buff[16 * (drv>>1) + 8 + bc_size]);
		dfile[dfilecnt].LBAaddr = dfile[dfilecnt].LBAbeg;
		dfile[dfilecnt].LBAsize = *((DWORD*)&buff[16 * (drv>>1) + 12 + bc_size]);
		free(buff);
		return dfilecnt;
	}
	else 				/* Is it CPM filespec? */
        {
		dfile[dfilecnt].IsDrv = FALSE;
		dfile[dfilecnt].LBAbeg = 0;
		dfile[dfilecnt].LBAaddr = 0;
		dfile[dfilecnt].LBAsize = 0;
                dfile[dfilecnt].drive = -1;
		if (mode & O_RDWR)
		  dfile[dfilecnt].fileCPM=fopen(path, "wb+");
		else
		  dfile[dfilecnt].fileCPM=fopen(path, "rb+");
		if (dfile[dfilecnt].fileCPM) { 
		    return dfilecnt;
 		}
	}
	return -1;
}

int dclose(int FileObject)
{
  if (! dfile[FileObject].IsDrv) {
    if (fclose(dfile[FileObject].fileCPM)!=0)
      return -1;
  }
  dfile[FileObject].fileCPM=NULL; dfile[FileObject].drive=-1;
  return FileObject;
}

int dread(int FileObject, BYTE* buffer, int sec_size /*ignored*/)
{
    if (dfile[FileObject].IsDrv) {
	if (! buffer) 
		return -1;
	if (disk_read(dfile[FileObject].drive & 1, buffer, dfile[FileObject].LBAaddr, 1) != RES_OK)
		return -1;
        if (dfile[FileObject].LBAaddr>=dfile[FileObject].LBAbeg+dfile[FileObject].LBAsize)	/* partition upper limit */
                return -1; 
	dfile[FileObject].LBAaddr++;
	return sector_size;
    } else {
        return fread(buffer, 1, sec_size, dfile[FileObject].fileCPM);
/* fread(buffer, sec_size, 1, dfile[FileObject].fileCPM)*sec_size; because to read last non-512b sector */
    }
}

int dwrite(int FileObject, BYTE* buffer, int sec_size, int val /*ignored*/)
{
    if (dfile[FileObject].IsDrv) {
	if (! buffer) 
		return -1;
	if (disk_write(dfile[FileObject].drive & 1, buffer, dfile[FileObject].LBAaddr, 1) != RES_OK)
		return -1;
        if (dfile[FileObject].LBAaddr>=dfile[FileObject].LBAbeg+dfile[FileObject].LBAsize)	/* partition upper limit */
                return -1; 
	dfile[FileObject].LBAaddr++;
	return sector_size;
    } else {
	return fwrite(buffer, sec_size, 1, dfile[FileObject].fileCPM)*sec_size; 
    }
}

int pread(int FileObject, BYTE* buffer, int sec_size /*ignored*/)
{
    if (dfile[FileObject].IsDrv) {
	if (! buffer) 
		return -1;
	if (disk_read(dfile[FileObject].drive & 1, buffer, 0, 1) != RES_OK) {
		return -1;
	}
	return sector_size;
    } else {
	fseek(dfile[FileObject].fileCPM, 0 /*offset*/, 0 /*from beginning*/);
        return fread(buffer, sec_size, 1, dfile[FileObject].fileCPM)*sec_size; 
    }
}

int pwrite(int FileObject, BYTE* buffer, int sec_size /*ignored*/, int val /*ignored*/)
{
    if (dfile[FileObject].IsDrv) {
	if (! buffer) 
		return -1;
	if (disk_write(dfile[FileObject].drive & 1, buffer, 0, 1) != RES_OK) {
		return -1;
        }
	return sector_size;
    } else {
	fseek(dfile[FileObject].fileCPM, 0, 0);
        return fwrite(buffer, sec_size, 1, dfile[FileObject].fileCPM)*sec_size; 
    }
}

char UpCase(char ch)
{
  if (islower(ch)) return toupper(ch); else return ch;
}

void FillPartitionBufCHS(BYTE* buf, WORD bh, WORD bs, WORD bc, WORD eh, WORD es, WORD ec)
{
  if (bc>768) {
    buf[1] = 0xFE;
    buf[2] = 0xFF;
    buf[3] = 0xFE;
  }
  else {
    buf[1] = bh;
    buf[2] = bs + ((bc & 768) >> 2);
    buf[3] = bc & 255;
  }
  if (ec>768) {
    buf[5] = 0xFE;
    buf[6] = 0xFF;
    buf[7] = 0xFE;
  }
  else {
    buf[5] = eh;
    buf[6] = es + ((ec & 768) >> 2);
    buf[7] = ec & 255;
  }
}

