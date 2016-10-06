/*

File management functions
(c) 2010 Serge

*/

#ifndef _FILEMGR

#include <hitech.h>
#include "ff.h"

typedef struct {
  BYTE	dr;		/* drive code */
  char	name[8];	/* file name */
  char	ft[3];		/* file type */
  BYTE	ex;		/* file extent */
  BYTE	fil[2];		/* not used */
  BYTE	rc;		/* number of records in present extent */
  char	dm[16];		/* logical blocks map */
  BYTE  cr;		/* current record number */
  BYTE  R0;		/* direct positioning record lower byte */
  BYTE  R1;		/* direct positioning record medium byte */
  BYTE  R2;		/* direct positioning record high byte */
  BYTE  R3;
} CFCB;

typedef struct {
  CFCB  fcb;		/* allways first for typecasts to CFCB */
  BYTE  uid;
  BYTE  prevuid;
} CFIL;

typedef struct {
 uchar	fileDSK;	/* ORDOS disk (0=ROM, 1,2..=RAM )    */
 ushort	fileBEG;	/* ORDOS file begin address	     */
 ushort	fileEND;	/* ORDOS file end address	     */
 ushort	filePTR;	/* ORDOS file next operation address */
 char   name[16]; 
} OFIL;

#define FIND_ENUM	0
#define FIND_FIRST	1
#define FIND_FREE	2

#define FTYPEUNK	0
#define FTYPECPM	1
#define FTYPEFAT	2
#define FTYPEORD	3

#define RAMTOP		61440

typedef struct {
  union {		/* union allways first for typecasts to CFIL, FIL, OFIL */
    CFIL fileCPM;	/* CPM file structure   */
    FIL  fileFAT;	/* FAT file structure   */
    OFIL fileORD;	/* ORDOS file structure */
  } ufl;
  BYTE	OSType;		/* UNKNOWN, CPM, FAT, ORD */
} OSFILE;

#define MAX_BUFF	512
#define CPM_BLOCKSIZE	128

typedef FRESULT (*dir_callback)(FILINFO* fileinfo);

/* Functions forwards */

extern BOOL IsCPMpath(char* path);
extern BOOL IsORDpath(char* path);
extern BOOL IsFATpath(char* path);
extern BYTE GetOSType(char* path);
extern FILINFO* FcbToFInfo(CFCB* pfcb, FILINFO* finfo);
extern int PathToFcb(char* path, CFCB  *xfcb);
extern FRESULT OS_open(OSFILE* FileObject, char* path, BYTE Flags);
extern FRESULT OS_close(OSFILE* FileObject);
extern FRESULT OS_read(OSFILE* FileObject, void* buf, WORD cnt, WORD* readed); /*read MAX_BUFF bytes ! */
extern FRESULT OS_write(OSFILE* FileObject, void* buf, WORD cnt, WORD* written);
extern BOOL OS_delete(char* path);
extern BOOL OS_rename(char* src, char* dst);
extern BYTE OS_getuid();
extern void OS_setuid(int uid);
extern ushort scanORD(char* path, dir_callback OnFile, BYTE FindMode);
extern FRESULT scanCPM(char* path, dir_callback OnFile);
extern FRESULT scanFAT(char* path, dir_callback OnFile);

#define _FILEMGR
#endif
