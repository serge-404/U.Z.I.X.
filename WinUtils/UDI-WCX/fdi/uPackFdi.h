#ifndef __UPACKUDI_H
#define __UPACKUDI_H

#include "wcxhead.h"

#define  ERR_FILE_OPEN -1
#define  ERR_FILE_STRU -2
#define  ERR_FILE_SIZE -3
#define  ERR_FILE_SEEK -4

#define  ERR_NO_DISK_SPACE   -5
#define  ERR_NO_DIR_SPACE    -6
#define  ERR_PACK_FILE       -7

#define  ERR_WRONG_DPB_CRC   -8
#define  ERR_WRONG_DISK_SIZE -9

typedef struct s_FileRec {
               struct s_FileRec *ParentDir;
               struct s_FileRec *PrevItem;
               struct s_FileRec *NextItem;
               DWORD FileSize;
               DWORD FileTime;
               DWORD FileAttr;
               char FileName[MAX_PATH];
} FileRec_t;

typedef FileRec_t* PFileRec;
typedef unsigned int uint;

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

