//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------
//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be performing new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you
//---------------------------------------------------------------------------

#pragma argsused

#define __MAIN__COMPILATION
#define PC_UTILS_TARGET
#define BCB_COMPILER
//#define BCB_DEBUG
#define NO_ASM

#include <stdio.h>
#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>

#define fmOpenRead       0x0
#define fmOpenWrite      0x1
#define fmOpenReadWrite  0x2
#define fmShareCompat    0x0
#define fmShareExclusive 0x10
#define fmShareDenyWrite 0x20
#define fmShareDenyRead  0x30
#define fmShareDenyNone  0x40

#define faReadOnly 0x1
#define faHidden   0x2
#define faSysFile  0x4          // =executable flag
#define faVolumeID 0x8
#define faDirectory 0x10
#define faArchive  0x20
#define faContentF 0x0100       // fake dir flag - shows dir wih file(s)
#define faContentD 0x0200       // fake dir flag - shows dir wih sub-dir(s)
#define faPipe     0x0200       // fake file flag - shows if file is pipe
#define faLink     0x0400       // fake file flag - shows if file is symlink
#define faDevChar  0x0800       // fake file flag - char device

#include "ffconf.h"
#include "ff.h"
#include "uPackFdi.h"

#define PluginTitle "FAT disk images (FDI) serving plugin. (p)2019 Serge"
#define MAXDRIV 2

int rdev=0;                                                     // 0..MAXDRIV-1
char DriveImage[MAXDRIV][MAX_PATH];
void* fHandle=NULL;                                             /* opened file descriptor */
FATFS ffs[MAXDRIV];
PARTITION VolToPart[]={{3,1},{3,2},{3,3},{3,4}};	        /* Volume - Partition resolution table */
/*
typedef struct {
	BYTE pd;	                                        // Physical drive number
	BYTE pt;	                                        // Partition: 0:Auto detect, 1-4:Forced partition)
} PARTITION;
*/
typedef union {
      DWORD wdt;
      struct {
        WORD btime;
        WORD bdate;
      } dt;
} FUDT;

int xget (char *arg, char *fatn);
int xput (char *arg, char *dosn);
//
int FileListPos = 0;
char TmpBuf[MAX_PATH], FPath[MAX_PATH], FPath2[MAX_PATH];
char ArcFileName[MAX_PATH];
char IniFileName[MAX_PATH];
DWORD PartitionOffset = 0;
DWORD PartitionN = 0;
int Panic = 0;
PFileRec FileList=NULL, LastItem=NULL, FileListItem=NULL, PrevFileListItem=NULL;
int FileListCount=0;                                                  //

//////////////////// strings utilities ////////////////////////

char* AddSlash(char *sss)
{
  int ii;
  if ( (ii=strlen(sss)) && (sss[ii-1] != '\\'))
    strcat(sss, "\\");
  return sss;
}

void GetIniSettings(char *IniName)
{
return;
}

void DisposeFileList(PFileRec FirstMember)
{
  PFileRec NextMember;
  while (FirstMember) {
    NextMember=FirstMember->NextItem;
    free(FirstMember);
    FirstMember=NextMember;
  }
  FileListCount=0;
  FileList=NULL;
}

int cursize;   // nonrecursive variable for GetFullPath()

char* GetFullPath(char* FullPath, PFileRec FRec, int maxsize, int AddRoot, char RootCh)
{
  int ii;
  if (FRec->ParentDir)
     GetFullPath(FullPath, FRec->ParentDir, maxsize, AddRoot, RootCh);
  ii=0;
  if (cursize || AddRoot) {
    FullPath[cursize]=RootCh; cursize++;
  }
  while ((cursize<maxsize)&&(FRec->FileName[ii])) {
    FullPath[cursize]=FRec->FileName[ii];
    cursize++; ii++;
  }
  FullPath[cursize]=0;
  return FullPath;
}

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
  char *pt;
  switch(reason){
    case DLL_PROCESS_ATTACH:      // initialization
      {
        // opened with LoadLibrary?  if(lpvReserved == NULL){
        PartitionOffset=0;                   // 0=for ODI, 512..x for OHI
        TmpBuf[sizeof(TmpBuf)-1]=0;
        FileList=NULL;
        LastItem=NULL;
        FileListItem=PrevFileListItem=NULL;
        FileListCount=0;
        if (GetModuleFileName(hinst, IniFileName, sizeof(IniFileName)-1)>0) {
          if (pt=strrchr(IniFileName, '.')) strcpy(pt, ".INI");
        }
        else
          strcpy(IniFileName,"fdi.ini");
        GetIniSettings(IniFileName);
        break;
      }
   case DLL_PROCESS_DETACH:      // finalization
      {
        // opened with FreeLibrary -   if(lpvReserved == NULL)
        DisposeFileList(FileList);
        break;
      }
  }
  return 1;
}

int file_exists(char* fname)
{
    FILE *file;
    if (file = fopen(fname, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

DWORD file_size(char* fname)
{
    struct stat st;
    if (stat(fname, &st) == 0)
        return st.st_size;

    return 0;
}

int xfs_init(int bootdev, char* fname)
{
   int res=0;
   rdev=bootdev;
   strncpy(DriveImage[rdev],fname,MAX_PATH-1);
   DriveImage[rdev][MAX_PATH-1]=0;
   sprintf(FPath2, "%d:", PartitionN);
   if (!(res=f_mount(&ffs[rdev], FPath2, 0))) {         /* Mount the default drive. res=0 if OK */
     if (res=f_chdrive(FPath2))                         /* Set drive 2 as current drive */
       return res;
     else {
       return f_chdir("/");                             /* Change current direcoty of the current drive to root directory) */
     }
   }
   return res;
}

int xfs_end(int bootdev)
{
  int res=f_mount(0, "", 0);              // umount current drive
  if (fHandle) {
    fclose(fHandle);
    fHandle=NULL;
  }
  return res;
}

int xget(char *arg, char *fatn)
{
	int d, nread, bw;
        FRESULT fr;                     /* FatFs function common result code */
	FILE *fp = fopen(arg, "rb");
        FIL fdst;                       /* File objects */
	char cbuf[512];

	if (fp == NULL) {
		MessageBox(0, "Source file not found", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	if (*fatn == 0) {
		fatn = arg+strlen(arg);
		while (--fatn > arg) {
			if (*fatn == '\\' || *fatn == ':') {
				++fatn;
				break;
			}
		}
	}
        /* Create destination file on the drive 0 */
        fr = f_open(&fdst, fatn, FA_WRITE | FA_CREATE_ALWAYS);
	if (fr) {
		MessageBox(0, fatn, "Cant create FATfs file", MB_OK+MB_ICONERROR);
		return fr;
	}
	for (;;) {
		nread = fread(cbuf, 1, sizeof cbuf, fp);
		if (nread == 0)
			break;
                fr = f_write(&fdst, cbuf, nread, &bw);             /* Write it to the destination file */
                if (fr || bw < nread) break;                       /* error or disk full */
	}
	fclose(fp);
	f_close(&fdst);
	return (int)fr;
}

int xput (char *arg, char *dosn)
{
	unsigned int br;
	FILE *fp;
        FIL ffp;
        FRESULT fr;
	char buffer[512];

	fr = f_open(&ffp, arg, FA_READ);
	if (fr) {
                MessageBox(0, arg, "Can't open FATfs file", MB_OK+MB_ICONERROR);
		return fr;
	}
	if (*dosn == 0) {
		dosn = arg+strlen(arg);
		while (--dosn > arg) {
			if (*dosn == '\\' || *dosn == ':') {
				++dosn;
				break;
			}
		}
	}
	fp = fopen(dosn, "wb");
	if (fp == NULL) {
		MessageBox(0, dosn, "Can't open destination file",MB_OK+MB_ICONERROR);
		return (-1);
	}
	for (;;) {
                fr = f_read(&ffp, buffer, sizeof buffer, &br);  /* Read a chunk of source file */
                if (fr || br == 0) break;                       /* error or eof */
		if (fwrite(buffer, br, 1, fp) != 1) {
			MessageBox(0,dosn,"fwrite error to file",MB_OK+MB_ICONERROR);
                        break;
		}
	}
	fclose(fp);
	f_close(&ffp);
	return (int)fr;
}

/* This makes a filesystem */
BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
int FdiCreateArchive(char* fname, uint ftype, uint fclust, uint fpart)
{
    /* Create FAT volume */
    sprintf(FPath2, "%d:", fpart);
    return f_mkfs(FPath2, ftype, fclust, work, sizeof work);
}

PFileRec PPrevFRec;     // no stack recursion for this variables
FILINFO statbuf;

int lsdir(char* dir, int depth, PFileRec PPrevDir)
{
    FRESULT res;
    PFileRec PFRec=NULL;
    int i, isRoot=(dir)&&(*dir=='/')&&(!dir[1]);
    DIR dp;
    FUDT udt;
    if(f_opendir(&dp, dir)) {
        MessageBox(0, dir, "Cannot open directory", MB_OK+MB_ICONSTOP);
        return ERR_FILE_STRU;
    }
// add dir record
    if (! isRoot) {                  // if not a root dir (root not needed)
      LastItem = PFRec = malloc(sizeof(FileRec_t));                       /* alloc current dirrectory record */
      strncpy(PFRec->FileName, statbuf.fname, sizeof(PFRec->FileName)-1);
      PFRec->FileName[sizeof(PFRec->FileName)-1]=0;
      PFRec->FileAttr=faDirectory;
      udt.dt.btime=statbuf.ftime;
      udt.dt.bdate=statbuf.fdate;
      PFRec->FileTime=udt.wdt;                                                           // statbuf is actual from previous iteration (UZIXstat on this dirrectory)
      PFRec->FileSize=(DWORD)statbuf.fsize;                                              //
      PFRec->FileAttr=statbuf.fattrib;
      PFRec->ParentDir= PPrevDir;
      PFRec->NextItem = NULL;
      PFRec->PrevItem = PPrevFRec;
      if (PPrevFRec)
        PPrevFRec->NextItem = PFRec;
      if (! FileList)
        FileList = PFRec;
      FileListCount++;
     }
    PPrevDir=PPrevFRec=PFRec;
    while(f_readdir(&dp, &statbuf)==FR_OK) {
      if (statbuf.fname[0] == 0) break;
      if (statbuf.fattrib & AM_DIR) {            /* Found a directory, but ignore . and .. */
        if(strcmp(".",statbuf.fname) == 0 ||
           strcmp("..",statbuf.fname) == 0)
          continue;
        if (PPrevDir)
          PPrevDir->FileAttr |= faContentD;
        i = strlen(dir);
        sprintf(&dir[i], "/%s", statbuf.fname);
        res=lsdir(dir, depth+1, PPrevDir);  /* Recurse at a new indent level */
        if (res != FR_OK) break;
        dir[i] = 0;
      } else {
// add file record
        LastItem = PFRec = malloc(sizeof(FileRec_t));
        PFRec->FileAttr=statbuf.fattrib;
        strncpy(PFRec->FileName, statbuf.fname, sizeof(PFRec->FileName)-1);             // TODO: LFN support  (statbuf.altname)
        PFRec->FileName[sizeof(PFRec->FileName)-1]=0;
        PFRec->FileSize=(DWORD)statbuf.fsize;                                                   //
        udt.dt.btime=statbuf.ftime;
        udt.dt.bdate=statbuf.fdate;
        PFRec->FileTime=udt.wdt;
        PFRec->ParentDir= PPrevDir;
        PFRec->NextItem = NULL;
        PFRec->PrevItem = PPrevFRec;
        if (PPrevFRec)
          PPrevFRec->NextItem = PFRec;
        if (! FileList)
          FileList = PFRec;
        FileListCount++;
        PPrevFRec=PFRec;
      }
      if (PPrevDir)
        PPrevDir->FileAttr |= faContentF;
    }
    res=f_closedir(&dp);
    return (int)res;
}

int FdiGetCatalog(char* fname)
{
  char topdir[MAX_PATH]="/";
  DisposeFileList(FileList);
  DriveImage[rdev][MAX_PATH-1]=0;
  PPrevFRec=NULL;                               // starting from root
  return lsdir(topdir, 0, NULL);
}

int FdiFileExtract(char* OdiArchiveName, PFileRec PFRec, char* OutName)
{
  int res;
  HANDLE fh;
  FILETIME ft, lft;  /* NT filetime */
  FUDT uft;          /* FAT filetime*/
  cursize = 0;
  res=xput(GetFullPath(FPath, PFRec, sizeof(FPath)-1, TRUE, '/'), OutName);
  if (!res) {  /* if succesfully extracted */
    uft.wdt=PFRec->FileTime;
    DosDateTimeToFileTime(uft.dt.bdate, uft.dt.btime, &lft);
    LocalFileTimeToFileTime(&lft, &ft);
    fh = CreateFile(OutName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetFileTime(fh, (LPFILETIME)NULL, (LPFILETIME)NULL, &ft);
    CloseHandle(fh);
    SetFileAttributes(OutName, PFRec->FileAttr & 255);
  }
  return res;
}

char* StrSlashCat(char* buf, char* path, char* name, int AddRoot)
{
  if (AddRoot)
    strcpy(buf, "\\");
  else
    *buf=0;
  if (path) {   /* path=NULL if destination is root */
    strncat(buf, path, MAX_PATH-strlen(buf));
    buf[sizeof(*buf)-2]=0;
    AddSlash(buf);
  }
  strncat(buf, name, MAX_PATH-strlen(buf));
  buf[MAX_PATH-1]=0;
  return buf;
}

char* ConvSlash(char* path) {
  int i;
  for (i=0; path[i]; i++)
    if (path[i]=='\\') path[i]='/';
  return path;
}

int FdiFilePack(char* OdiArchiveName, char* SrcFileName, char* ArchFileName)
{
  int res=-1;
  HANDLE fh;
  DWORD fa;
  int mode;
  FILETIME cft, aft, mft;      /* NT filetime */
  FILINFO fno;                 /* FAT file attributes */

  if ((SrcFileName[strlen(SrcFileName)-1]=='\\')&&(ArchFileName[strlen(ArchFileName)-1]=='\\')) {
    mode = faDirectory;
    SrcFileName[strlen(SrcFileName)-1]=0;
    ArchFileName[strlen(ArchFileName)-1]=0;
    res=f_mkdir(ConvSlash(ArchFileName));
  }
  else {
    mode = 0;
    cursize = 0;
    res=xget(SrcFileName, ConvSlash(ArchFileName));
  }
  if (!res) {  /* if succesfully packed - set time and attrs */
// TODO - save creation time, owner and attributes (except X=system) for allready existed files
// chown(destname, statbuf1.st_uid, statbuf1.st_gid);
    fa=GetFileAttributes(SrcFileName);
    if (fa!=0xFFFFFFFF) {  /* set file attr */
       mode|=fa;
       f_chmod(ConvSlash(ArchFileName), mode, 0xff);
    }
    fh = CreateFile(SrcFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, NULL);
    GetFileTime(fh, &cft, &aft, &mft);
    CloseHandle(fh);
    FileTimeToLocalFileTime(&mft, &cft);                      // modify time
    FileTimeToDosDateTime(&cft, &fno.fdate, &fno.ftime);      // use cft as a buffer (UZIXutime not allow to change creation time)
    res=f_utime(ConvSlash(ArchFileName), &fno);
  }
  return res;
}

int FdiFileDelete(char* OdiArchiveName, char* FileToDelete)
{
  int res;
  char* st;
  if (st=strstr(FileToDelete, "*.*")) {  // if deleting dirrectory
    *st=0; st--; *st=0;
    res=f_unlink(ConvSlash(StrSlashCat(FPath2, "", FileToDelete, TRUE)));
  }
  else {                                 // else delete file
    cursize = 0;
    res=f_unlink(ConvSlash(StrSlashCat(FPath2, "", FileToDelete, TRUE)));
  }
  return res;
}

char* CheckFN(char* name)
{
  int shft=0, partN=-1;                                           // MBR partition N = 0..3
  if ( (strlen(name)>2) && (name[1]==':') && isdigit(*name) ) {   // filespec: partN:disk:\path\fname.ext
    shft=2;
    partN=*name-'0';
    if (partN>3) return NULL;
  }
  return &name[shft];
}

char* FdiGetInfo(char* ArcNm)
{
  int res;
  FATFS *fs;
  DWORD fre_clust, fre_sect, tot_sect;
  if ((ArcNm)&&(strlen(ArcNm)>2)&&(ArcNm[1]==':')&&(*ArcNm>='0')&&(*ArcNm<'4')) {
    PartitionN=*ArcNm-'0';
    strncpy(ArcFileName,ArcNm,sizeof(ArcFileName)-1);
    ArcFileName[sizeof(ArcFileName)-1]=0;
    ArcNm++;
    ArcNm++;
    rdev=0;
  }
  if (! xfs_init(0, ArcNm)) {                           /* setts FPath2 */
    res = f_getfree(FPath2, &fre_clust, &fs);
    if (res) return "Error: f_getfree";
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    /* Get volume label of the default drive */
    f_getlabel("", FPath2, 0);
    /* Print the free space (assuming 512 bytes/sector) */
    sprintf(TmpBuf,"VolumeLabel: `%s`\n%10lu KiB total space.\n%10lu KiB available.\n%10lu sector(s) per cluster",
     	         FPath2, tot_sect / 2, fre_sect / 2, fs->csize);
    xfs_end(0);
  }
  return TmpBuf;
}

/*
        int shft=0, partN=-1;                                           // MBR partition N = 0..3
        if ( (strlen(name)>2) && (name[1]==':') && isdigit(*name) ) {   // filespec: partN:disk:\path\fname.ext
          shft=2;
          partN=*name-'0';
          if (partN>3) return -1;
        }
*/

/////////////////////////////// exported plugin functions ///////////////////////

HANDLE __export WINAPI OpenArchivePart(char *ArcName, DWORD PartOffset, DWORD PartN)    // PartOffset in sectors (not a byte)
{
  int res=-1;
  PartitionOffset=PartOffset*512;
  FileListPos = 0;
  FileListItem=PrevFileListItem=NULL;
  ArcFileName[0]=0;
  PartitionN = PartN;
  if (PartN<4) {
    ArcFileName[0]=((char)PartN)+'0'; ArcFileName[1]=':'; ArcFileName[2]=0;
  }
  strncat(ArcFileName, ArcName, sizeof(ArcFileName)-strlen(ArcName)-1);
  ArcFileName[sizeof(ArcFileName)-1]=0;
  if (! xfs_init(rdev, ArcName)) {                      // mount
    if (! file_exists(ArcName))
      FdiCreateArchive(ArcName, FM_ANY, 0, PartN);      //
    res=FdiGetCatalog(ArcName /*ArcFileName*/);
  }
  xfs_end(rdev);                                        // umount
  if (res>=0) {
    FileListItem=PrevFileListItem=FileList;             // points to first item
    return (void*)1;
  }
  else
    return 0;
}

HANDLE __export WINAPI OpenArchive(tOpenArchiveData *ArchiveData)
{
  HANDLE Result=OpenArchivePart(ArchiveData->ArcName, 0, 127);
  if (Result==0)
    ArchiveData->OpenResult = E_UNKNOWN_FORMAT;
  return Result;
}

int __export WINAPI ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{
  PFileRec FileRec;
  int FullPathLen;
/*
  MessageBox(0, "read header", "Information", MB_OK+MB_ICONINFORMATION);
*/
  FileListPos++;
  if (FileListPos > FileListCount)  /* +1 */ 
  {
    FileListPos  = 0;
    FileListItem = PrevFileListItem = FileList;
    return E_END_ARCHIVE;
  }
  else
  {
    if (!(FileRec = FileListItem))
      return E_END_ARCHIVE;
    PrevFileListItem = FileListItem;
    FileListItem = FileListItem->NextItem;    /*???*/
    *HeaderData->FileName = 0;
    cursize = 0;
    GetFullPath(HeaderData->FileName, FileRec, sizeof(HeaderData->FileName)-1, FALSE, '\\');
    HeaderData->FileName[sizeof(HeaderData->FileName)-1]=0;
    HeaderData->FileAttr                        = FileRec->FileAttr;
    HeaderData->PackSize = HeaderData->UnpSize  = FileRec->FileSize;
    HeaderData->FileTime                        = FileRec->FileTime;
    return 0;
  }
}

int __export WINAPI ProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{
  int res;
  PFileRec FileRec;
  char OutName[MAX_PATH];
/*
  MessageBox(0, "process file", "Information", MB_OK+MB_ICONINFORMATION);
*/
  if (FileListPos == FileListCount+1)
    return E_END_ARCHIVE;
  else {
    if ((Operation == PK_SKIP) || (Operation == PK_TEST))
      return 0;
    else {
      if (DestPath) {
        StrSlashCat(FPath2, OutName, DestName, FALSE);
      }
      else {
        strncpy(OutName, DestName, sizeof(OutName)-1);
        OutName[sizeof(OutName)-1]=0;
      }
#ifdef USE_VCL
      FileRec = (PFileRec)(FileList->Items[FileListPos-1]));
#else
      FileRec = PrevFileListItem;
#endif
      if (! (res=xfs_init(rdev, DriveImage[rdev])))  // mount
        res=FdiFileExtract(ArcFileName, FileRec, OutName);
      xfs_end(rdev);                                           // umount
      switch (res) {
        case             0: return 0;
        case ERR_FILE_STRU: return E_BAD_DATA;
        case ERR_FILE_SIZE: return E_EREAD;
        case ERR_FILE_SEEK: return E_BAD_ARCHIVE;
        default:            return E_EOPEN;
      }
    }
  }
  return E_NOT_SUPPORTED;
}

int __export WINAPI CloseArchive(HANDLE hArcData)
{
  return 0;
}

int __export WINAPI PackFiles(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags)
{
    char  src_path[MAX_PATH], sub_path[MAX_PATH];
    int Result =xfs_init(rdev, PackedFile);   // mount
    if (! Result) {
      if (! file_exists(PackedFile))
        FdiCreateArchive(PackedFile, FM_ANY, 0, PartitionN);
      if ((FdiGetCatalog(PackedFile)<0) || (! AddList))
      {
        xfs_end(rdev);                                    // unmount
        return E_UNKNOWN_FORMAT;
      }
      while (*AddList)
      {
        Result=FdiFilePack(PackedFile,
                           StrSlashCat(FPath, SrcPath, AddList, FALSE),
                           StrSlashCat(FPath2, SubPath, AddList, TRUE));
        if (Result<0)
          break;
        else
          if ((Flags & PK_PACK_MOVE_FILES) != 0)
            DeleteFile(strcat(AddSlash(SrcPath), AddList));
        AddList += strlen(AddList) + 1;
      }
    }
    xfs_end(rdev);                                         // unmount
    return Result;
}

int __export WINAPI DeleteFiles(char *PackedFile, char *DeleteList)
{
  int Result = E_UNKNOWN_FORMAT;
  if (file_exists(PackedFile))
  {
    if (! (Result=xfs_init(rdev, PackedFile))) {    // mount
      if (FdiGetCatalog(PackedFile)<0)
      {
        xfs_end(rdev);                                        // unmount
        return Result;
      }
      Result = 0;
      while (*DeleteList)
      {
        Result=FdiFileDelete(PackedFile, DeleteList);
        if (Result<0)
          break;
        DeleteList += strlen(DeleteList) + 1;
      }
    }
    xfs_end(rdev);                                // unmount
  }
  return Result;
}

int __export WINAPI GetPackerCaps()
{
  return PK_CAPS_NEW | PK_CAPS_MODIFY | PK_CAPS_MULTIPLE | PK_CAPS_DELETE | PK_CAPS_OPTIONS | PK_CAPS_BY_CONTENT;
}

char* __export WINAPI GetPartInfo(char *OdiArchiveName)
{
  return FdiGetInfo(ArcFileName);
}


int __export WINAPI CanYouHandleThisFile(char *FileName)
{
  int Res=-1;
  Panic=0;
  if (! xfs_init(rdev, FileName))      // mount
    Res=FdiGetCatalog(FileName);
  xfs_end(rdev);                                 // umount
  Panic=1;
  return (Res>=0);
}

void __export WINAPI SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
//  DebugInfo('SetChangeVolProc');
}

void __export WINAPI SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc)
{
//  DebugInfo('SetProcessDataProc');
}

void __export WINAPI ConfigurePacker(HANDLE Parent, DWORD DllInstance)
{
  sprintf(FPath, "Double/TotalCommander archiver (WCX) plugin for serving UDI\nfiles (UZIX disk Image files). Allow copy/extract UZIX files\n"\
                  "to/from UDI \"disk image\" such simple as processing any\narchives with TotalCommander interface.\n\nFREEWARE Version 1.01,\n"\
                  "distributed \"AS IS\" WITHOUT ANY WARRANTY\n\nCopyright (C)2008-2019 Sergey A.\n\nArchive: `%s`\n\n%s",
                  ArcFileName, FdiGetInfo(ArcFileName));
  MessageBox(Parent, FPath, "Information", MB_OK+MB_ICONINFORMATION);
}

HANDLE __export WINAPI CreateArchivePart(char *ArcName, DWORD PartOffset, DWORD PartN, DWORD PartSize)   // PartOffset,PartSize in sectors (not a byte)
{
  int res;
  PartitionOffset=PartOffset*512;
  FileListPos = 0;
  FileListItem=PrevFileListItem=NULL;
  ArcFileName[0]=0;
  PartitionN = PartN;
  if (PartN<4) {
    ArcFileName[0]=((char)PartN)+'0'; ArcFileName[1]=':'; ArcFileName[2]=0;
  }
  strncat(ArcFileName, ArcName, sizeof(ArcFileName)-strlen(ArcName)-1);
  ArcFileName[sizeof(ArcFileName)-1]=0;
//  xfs_init(rdev, ArcName);                            // mkfs does disk_initialize() iself
   rdev=0;
   strncpy(DriveImage[rdev],ArcName,MAX_PATH-1);        // instead of xfs_init
   DriveImage[rdev][MAX_PATH-1]=0;
  res=FdiCreateArchive(ArcName, FM_ANY, 0, PartN);      //
//  xfs_end(rdev);                                      // umount
  if (fHandle) {
    fclose(fHandle);                                    // instead of xfs_end (no umount)
    fHandle=NULL;
  }
  return (HANDLE)res;
}

