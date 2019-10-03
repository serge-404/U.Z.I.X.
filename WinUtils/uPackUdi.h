#ifndef __UPACKUDI_H
#define __UPACKUDI_H

#include "wcxhead.h"
#define __EXTERN_H_IN_PROGRESS
#include "config.h"
#undef __EXTERN_H_IN_PROGRESS

#define  ERR_FILE_OPEN -1
#define  ERR_FILE_STRU -2
#define  ERR_FILE_SIZE -3
#define  ERR_FILE_SEEK -4

#define  ERR_NO_DISK_SPACE   -5
#define  ERR_NO_DIR_SPACE    -6
#define  ERR_PACK_FILE       -7

#define  ERR_WRONG_DPB_CRC   -8
#define  ERR_WRONG_DISK_SIZE -9

#define VIRTUAL_FOLDER          1
#define VIRTUAL_BOOTBIN         2
#define VIRTUAL_SYSTEMBIN       3

#define MAXDRIV 2
typedef unsigned long	DWORD;

typedef struct s_FileRec {
               struct s_FileRec *ParentDir;
               struct s_FileRec *PrevItem;
               struct s_FileRec *NextItem;
               DWORD FileSize;
               DWORD FileTime;
               DWORD FileAttr;
               int IsVirtual;                   // 1=BOOTBIN 2=SYSTEMBIN
               char FileName[MAX_PATH];
} FileRec_t;

typedef FileRec_t* PFileRec;

typedef struct {                // sizeof=32
          char Name[21];
          unsigned long Size;
          unsigned long Date;   // in format of FileGetDate() function
          unsigned char CRC;    // CRC66 byte
          unsigned char SIGN1;  // 0xAA
          unsigned char SIGN2;  // 0x55
} TSystemBinRec;

/* DiskParametersBlock (DPB) for UZIX floppy disks and HDD partitions */
typedef struct {                        // sizeof=30      (N/A means Not Applicable)
          unsigned char bootsig[3];     // boot signature
          char DiskName[8];             // disk name
          unsigned short SectorSize;    // sector size in bytes
          unsigned char ClusterSize;    // cluster size (in sectors)
          unsigned short Reserved;      // number of reserved sectors
          unsigned char nfat;           // number of FATs (N/A), on UZIX: reserved sectors for kernel
          unsigned short ndir;          // number of directory entries (N/A)
          unsigned short nsec;          // number of sectors on disk
          unsigned char diskID;         // disk ID
          unsigned short fatsize;       // FAT size in sectors (N/A)
          unsigned short nsectrk;       // number of sectors per track
          unsigned short nside;         // number of disk sides
          unsigned short nhidden;       // number of hidden sectors
} TDPBRec;                              // bootsig+30 - Start of boot program (must have up to 98 bytes) ;  bootsig+510 = 0AA55h - DOS boot sector signature

typedef void *HANDLE;           /* winnt.h */
#define WINAPI __stdcall

HANDLE __export WINAPI OpenArchive(tOpenArchiveData *ArchiveData);
HANDLE __export WINAPI OpenArchivePart(char *ArcName, DWORD PartOffset, DWORD PartN);
int __export WINAPI ReadHeader(HANDLE hArcData, tHeaderData *HeaderData);
int __export WINAPI ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName);
int __export WINAPI CloseArchive(HANDLE hArcData);
int __export WINAPI PackFiles(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags);
int __export WINAPI DeleteFiles(char *PackedFile, char *DeleteList);
int __export WINAPI GetPackerCaps();
char* __export WINAPI GetPartInfo(char *OdiArchiveName);
int __export WINAPI CanYouHandleThisFile(char *FileName);
void __export WINAPI SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1);
void __export WINAPI SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc);
void __export WINAPI ConfigurePacker(HANDLE Parent, DWORD DllInstance);
HANDLE __export WINAPI CreateArchivePart(char *ArcName, DWORD PartOffset, DWORD PartN, DWORD PartSize);

#endif

