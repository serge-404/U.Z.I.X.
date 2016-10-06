#ifndef _FDISK

#include <stdio.h>
#include "integer.h"

/* derived from FatFs */

#define BS_FilSysType		54
#define BS_55AA			510
#define BPB_ExtFlags		40
#define BS_FilSysType32		82
#define MBR_PART_TYPE		4

/*
#define DEBUG_FDISK
*/

#define O_RDWR   1
#define O_RDONLY 2
#define O_CREAT  4

#define _DRIVES 2	/* Master,Slave */

#define pt_size 66	/* Partition Table Size in Bytes */
#define bc_size 446	/* Boot Code Size in Bytes */
#define sector_size 512	/* Sector Size in Bytes */

/* Definitions for the menus */
#define MM   0x00       /* Main Menu                     */

#define CP   0x10         /* Create partition             */

#define CPC  0x11           /* Create Primary CPM Partition */
#define CPF  0x12           /* Create Primary FAT Partition */
#define CPU  0x13           /* Create Primary UZIX Partition */
#define CPCF 0x14           /* Create CPM filesystem on Partition */
#define CPCS 0x15           /* Sysgen CPM from a file */

#define SAP  0x20         /* Set Active Partition          */

#define DP   0x30         /* Delete partition or LDD       */

#define MBR  0x40         /* MBR Functions                 */

#define BMBR 0x41           /* Write standard MBR to drive */
#define FMBR 0x42           /* Create MBR using the file   */
#define SMBR 0x43           /* Save MBR to file            */
#define RMBR 0x44           /* Remove MBR from disk        */
#define CMBR 0x45           /* Create empty MBR on disk    */

#define CD   0x50         /* Change Drive                  */


int cpm_copy(char **argv);	/* copy [count] sectors from beginning of [Source] to [Destination]*/

typedef struct {
  BOOL  IsDrv;  	/* TRUE=DRIVE, FALSE=CPM_file  */
  FILE* fileCPM;	/* CP/M file descriptor */
  int   drive;  	/* IDE drive = (0,1) */
  DWORD LBAbeg;		/* partition begin offset for IDE drive */
  DWORD LBAsize;	/* partition size in 512b blocks */
  DWORD LBAaddr;	/* partition current offset for IDE drive */
} DFILE;

typedef struct {
  BYTE JMP[8];
  BYTE PAGE1;
  BYTE PAGE2;
  BYTE LEN1;	/* phisical sector size (1=256, 2=512, 3=1024)   */
  BYTE LEN2;	/* sides (density?) (0=one_side, 1=double_sided) */
  WORD SEC;	/* phisical sectors per track */
  WORD TRK;	/* phisical tracks on disk (one side) */
/*---------------------- CP/M standard -----------------------------*/
  WORD SPT;	/* logical sectors (128) per track */
  BYTE BSH;	/* Block Shift - Block Size is given by 128 * 2^(BSH) */
  BYTE BLM;	/* Block Mask - Block Size is given by 128 * (BLM +1) */
  BYTE EXM;	/* Extent Mask */
  WORD DSM;	/* user space size in kb = SEC * (TRK-OFF) - (CKS/8) */
  WORD DRM;	/* max quantity of file records (FCBs) in catalog */
  WORD AL;	/* 16-bit Directory Allocation Pattern */
  WORD CKS;	/* Directory Check Sum = catalog size (in logical blocks) */
  WORD OFF;	/* system tracks */
  BYTE CRC;	/* simple additional sum CRC beginning with 066h */
/*------------------------------------------------------------------*/
  char LABEL[17];
} TBootDPB;

void xmenu();
int dopen(char* path, int mode);                                   /* MBR/partition or file open*/
int dclose(int FileObject);                                        /* file close*/
int dread(int FileObject, BYTE* buffer, int sec_size);             /* partition or file read  */
int dwrite(int FileObject, BYTE* buffer, int sec_size, int val);   /* partition or file write */
int pread(int FileObject, BYTE* buffer, int sec_size);             /* MBR or file read */
int pwrite(int FileObject, BYTE* buffer, int sec_size, int val);   /* MBR or file write */
void FillPartitionBufCHS(BYTE* buf, WORD bh, WORD bs, WORD bc, WORD eh, WORD es, WORD ec);
void CheckIDE();
void bc_hexdump(BYTE* buf, int count);
void pt_hexdump(BYTE* buf, int drive);
void crlf();
int pt_sign(char* srcdev, BOOL do_sign);
int bc_copy(char* srcdev, char* dstdev, BOOL do_sign, int count);
BOOL bc_copy_mem(BYTE* buf, char* devname, BOOL do_sign, int count);

char UpCase(char ch);
int Inkey();
BOOL CheckPartitionTable(BYTE* tbl);  /* check for overloaded partitions */
BOOL SysgenCPM(int drive, int pidx, char* fname);
BOOL CreateCpmFS(int drive, int pidx);
BOOL ActivatePartition(int drive, int pidx);
BOOL DeletePartition(int drive, int pidx);
BYTE GetPartitionTable(int drive, BYTE* buf);
BOOL GetFreeSegment(int drive, 
                    BYTE* buf, 
                    DWORD size,		/* size in 512b blocks of min searched seg */
                    DWORD* pb,		/* start lba of min searched segment */
		    DWORD* maxsz,	/* size in 512b blocks of max available segment */
		    DWORD* totalfree);	/* total amount of free space (512b blocks) */
WORD CreatePartition(int drive, DWORD size /*in 512b blocks*/, BYTE ptype);
BOOL GetMBR(int drive, BYTE* buf);
BOOL GetCommit(char* msg);
char* CalcKM(char* cbuf, DWORD value);

extern unsigned char f_mkfs (unsigned char drive);

#define _FDISK
#endif

