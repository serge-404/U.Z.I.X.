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

#include "uPackUdi.h"

//#define EXT external

#include "xfs.h"
#include "fcntl.h"
#include "dirent.h"

//udata_t udata;
//time_t tod; 	/* Time of day */

#define PluginTitle "UZIX disk images (UDI) serving plugin. (p)2019 Serge"

char DriveImage[MAXDRIV][MAX_PATH];

int rdev = 0;
//uchar *syserror = &udata.u_error;

int xumask __P((char *masks));
int xmkdir __P((char *path));
int xget __P((char *arg, char *unixn, int binflag));
int xput __P((char *arg, char *dosn, int binflag));
int xunlink __P((char *path));
int xrmdir __P((char *path));
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
        PartitionOffset=0;                   // 0=for ODI, 1..x for OHI
        TmpBuf[sizeof(TmpBuf)-1]=0;
        FileList=NULL;
        LastItem=NULL;
        FileListItem=PrevFileListItem=NULL;
        FileListCount=0;
        if (GetModuleFileName(hinst, IniFileName, sizeof(IniFileName)-1)>0) {
          if (pt=strrchr(IniFileName, '.')) strcpy(pt, ".INI");
        }
        else
          strcpy(IniFileName,"udi.ini");
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

int UdiCreateArchive(char* fname)
{
  return 0;
}


struct dirent *readdir(DIR *dir)
{
	direct_t direntry;
	struct dirent *buf;

	if (dir == NULL || dir->dd_buf == NULL /* || dir->dd_fd == 0 */ ) {
		errno = EFAULT;
		return NULL;
	}
	direntry.d_name[0] = 0;
	while (direntry.d_name[0] == 0)
		if (UZIXread(dir->dd_fd, (char *)&direntry, sizeof(direntry)) != sizeof(direntry))
			return NULL;
	buf = dir->dd_buf;
	buf->d_ino = direntry.d_ino;
	buf->d_off = dir->dd_loc++;
	strncpy(buf->d_name, (char *)direntry.d_name, DIRNAMELEN);
	buf->d_name[DIRNAMELEN] = 0;
	buf->d_reclen = strlen(buf->d_name);
	return buf;
}

DIR *opendir(char* path)
{
	struct stat statbuf;
	DIR *dir;

	if (UZIXstat(path, &statbuf) != 0)
		goto Err;
	if ((statbuf.st_mode & S_IFDIR) == 0) {
		errno = ENOTDIR;
		goto Err;
	}
	if ((dir = (DIR *)calloc(1,sizeof(DIR))) == NULL) {
		errno = ENOMEM;
		goto Err;
	}
	if ((dir->dd_buf = (struct dirent *)calloc(1,sizeof(struct dirent))) == NULL) {
		free(dir);
		errno = ENOMEM;
		goto Err;
	}
	if ((dir->dd_fd = UZIXopen(path, 0 /* O_BINARY */ )) < 0) {
		free(dir->dd_buf);
		free(dir);
Err:		return NULL;
	}
	return dir;
}

int closedir(DIR *dir)
{
	if (dir == NULL || dir->dd_buf == NULL || dir->dd_fd == 0) {
		errno = EFAULT;
		return -1;
	}
	UZIXclose(dir->dd_fd);
	free(dir->dd_buf);
	dir->dd_fd = 0;
	dir->dd_buf = NULL;
	free(dir);
	return 0;
}

PFileRec PPrevFRec;     // no stack recursion for this variables
struct stat statbuf;    //
/*
void AddFListItem(PFileRec* pPFRec, PFileRec* pPPrevFRec, PFileRec* pPPrevDir)
{
  (*pPFRec)->FileSize=statbuf.st_size;                                                   //
  (*pPFRec)->FileTime=*((DWORD*)(void*)(&statbuf.st_mtime));                             // statbuf is actual from previous iteration (UZIXstat on this dirrectory)
  if (!(statbuf.st_mode & (S_IRWXG|S_IRWXO))) (*pPFRec)->FileAttr|=faHidden;             //  -rwx------
  if (!(statbuf.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH))) (*pPFRec)->FileAttr|=faReadOnly;   //  -?-??-??-?
  (*pPFRec)->ParentDir= *pPPrevDir;
  (*pPFRec)->NextItem = NULL;
  (*pPFRec)->PrevItem = *pPPrevFRec;
  if ((*pPFRec)->PrevItem)
    (*pPPrevFRec)->NextItem = *pPFRec;
  if (! FileList)
    FileList = *pPFRec;
  FileListCount++;
}
*/
int lsdir(char *dir, int depth, PFileRec PPrevDir)
{
    PFileRec PFRec=NULL;
    DIR *dp;
    int fd;
    struct dirent *entry;
    int stat, ii;
//    FILETIME ft, lft;  /* NT filetime */
//    time_t uft;   /* UZIX filetime */

    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return ERR_FILE_STRU;
    }
    UZIXchdir(dir);
// add dir record
    if (!((*dir=='/')&&(!dir[1]))) {                  // if not a root dir (root not needed)
      LastItem = PFRec = malloc(sizeof(FileRec_t));                       /* alloc current dirrectory record */
      strncpy(PFRec->FileName, dir, sizeof(PFRec->FileName)-1);
      PFRec->FileName[sizeof(PFRec->FileName)-1]=0;
      PFRec->FileAttr=faDirectory;

//      uft=statbuf.st_mtime;
//      DosDateTimeToFileTime(uft.t_date, uft.t_time, &ft);
//    FileTimeToLocalFileTime(&ft, &lft);

      PFRec->FileTime=*((DWORD*)(void*)(&statbuf.st_mtime));                             // statbuf is actual from previous iteration (UZIXstat on this dirrectory)
      PFRec->FileSize=statbuf.st_size;                                                   //
      if (!(statbuf.st_mode & (S_IRWXG|S_IRWXO))) PFRec->FileAttr|=faHidden;             //  -rwx------
      if (!(statbuf.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH))) PFRec->FileAttr|=faReadOnly;   //  -?-??-??-?
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
    while((entry = readdir(dp)) != NULL) {
      fd = UZIXopen(entry->d_name, O_SYMLINK);
      stat = (fd >= 0) ? UZIXfstat(fd, &statbuf) : UZIXstat(entry->d_name, &statbuf);
      if (stat) {
	if (fd >= 0)
 	  UZIXclose(fd);
        continue;
      }
      stat = statbuf.st_mode & S_IFMT;
      if (stat == S_IFDIR) {   /* Found a directory, but ignore . and .. */
        if(strcmp(".",entry->d_name) == 0 ||
           strcmp("..",entry->d_name) == 0)
          continue;
        if (PPrevDir)
          PPrevDir->FileAttr |= faContentD;
        lsdir(entry->d_name,depth+1, PPrevDir);  /* Recurse at a new indent level */
      } else {
// add file record
        LastItem = PFRec = malloc(sizeof(FileRec_t));
        PFRec->FileAttr=0;
        switch (stat) {
          case S_IFBLK:	break;
          case S_IFPIPE:PFRec->FileAttr|=faPipe; break;
          case S_IFCHR:	PFRec->FileAttr|=faDevChar; break;
          case S_IFLNK:	PFRec->FileAttr|=faLink; break;
	}
        if (statbuf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) PFRec->FileAttr|=faSysFile;       //  -??x??x??x
        strncpy(PFRec->FileName, entry->d_name, sizeof(PFRec->FileName)-1);
        PFRec->FileName[sizeof(PFRec->FileName)-1]=0;
        PFRec->FileSize=statbuf.st_size;                                                   //
        PFRec->FileTime=*((DWORD*)(void*)(&statbuf.st_mtime));                             // statbuf is actual from previous iteration (UZIXstat on this dirrectory)
        if (!(statbuf.st_mode & (S_IRWXG|S_IRWXO))) PFRec->FileAttr|=faHidden;             //  -rwx------
        if (!(statbuf.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH))) PFRec->FileAttr|=faReadOnly;   //  -?-??-??-?
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
    UZIXchdir("..");
    closedir(dp);
    return 0;
}

int xget(char *arg, char *unixn, int binflag)
{
	int d, nread;
	FILE *fp = fopen(arg, binflag ? "rb" : "r");
	char cbuf[BUFSIZE];

	if (fp == NULL) {
		MessageBox(0, "Source file not found", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	if (*unixn == 0) {
		unixn = arg+strlen(arg);
		while (--unixn > arg) {
			if (*unixn == '\\' || *unixn == ':') {
				++unixn;
				break;
			}
		}
	}
	d = UZIXcreat(unixn, 0666);
	if (d < 0) {
		MessageBox(0, unixn, "Cant open uzix file", MB_OK+MB_ICONERROR);
		return (-1);
	}
	for (;;) {
		nread = fread(cbuf, 1, BUFSIZE, fp);
		if (nread == 0)
			break;
		if (UZIXwrite(d, cbuf, nread) != nread) {
			MessageBox(0, "_write: error", "Error", MB_OK+MB_ICONERROR);
			fclose(fp);
			UZIXclose(d);
			return (-1);
		}
	}
	fclose(fp);
	UZIXclose(d);
	return (0);
}

int xput(char *arg, char *dosn, int binflag)
{
	int d, nread;
	FILE *fp;
	char cbuf[BUFSIZE];

	d = UZIXopen(arg, 0);
	if (d < 0) {
                MessageBox(0, arg, "Can't open uzix file", MB_OK+MB_ICONERROR);
		return (-1);
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
	fp = fopen(dosn, binflag ? "wb" : "w");
	if (fp == NULL) {
		MessageBox(0, dosn, "Can't open destination file",MB_OK+MB_ICONERROR);
		return (-1);
	}
	for (;;) {
		if ((nread = UZIXread(d, cbuf, BUFSIZE)) == 0)
			break;
		if (fwrite(cbuf, 1, nread, fp) != nread) {
			MessageBox(0,"fwrite error","fwrite error",MB_OK+MB_ICONERROR);
			fclose(fp);
			UZIXclose(d);
			return (-1);
		}
	}
	fclose(fp);
	UZIXclose(d);
	return (0);
}

int xunlink(path)
	char *path;
{
	int i, fd = UZIXopen(path, O_SYMLINK);
	struct stat statbuf;

	if (fd >= 0) {
		i = UZIXfstat(fd, &statbuf);
		UZIXclose(fd);
	}
	else	i = UZIXstat(path, &statbuf);
	if (i) {
		MessageBox(0, path, "unlink: can't stat", MB_OK+MB_ICONERROR);
		return (-1);
	}
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
		MessageBox(0, path, "unlink: directory", MB_OK+MB_ICONERROR);
		return (-1);
	}
	if (UZIXunlink(path) != 0) {
		MessageBox(0, "_unlink: error", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	return (0);
}

int xmkdir(path)
	char *path;
{
	if (UZIXmknod(path, S_IFDIR | 0777, 0) != 0) {
		MessageBox(0, "mkdir: mknod error", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	return 0;
}

int xrmdir(path)
	char *path;
{
	int fd;
	struct stat statbuf;
	char newpath[100];
	direct_t dir;

	if (UZIXstat(path, &statbuf) != 0) {
		MessageBox(0, "rmdir: can't stat", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	if ((statbuf.st_mode & S_IFDIR) == 0) {
		MessageBox(0, "rmdir: not a directory", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	if ((fd = UZIXopen(path, 0)) < 0) {
		MessageBox(0, "rmdir: dirrectory unreadable", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	while (UZIXread(fd, (char *) &dir, sizeof(dir)) == sizeof(dir)) {
		if (dir.d_ino == 0 ||
		    (strstr(dir.d_name, ".")==dir.d_name) ||
		    (strstr(dir.d_name, "..")==dir.d_name))
			continue;
		MessageBox(0, "rmdir: dirrectory not empty", "Error", MB_OK+MB_ICONERROR);
		UZIXclose(fd);
		return (-1);
	}
	UZIXclose(fd);
	strcpy(newpath, path);
	strcat(newpath, "/.");
	if (UZIXunlink(newpath) != 0)
		MessageBox(0, "rmdir: can't unlink \".\"", "Error", MB_OK+MB_ICONERROR);
	strcat(newpath, ".");
	if (UZIXunlink(newpath) != 0)
		MessageBox(0, "rmdir: can't unlink \"..\"", "Error", MB_OK+MB_ICONERROR);
	if (UZIXunlink(path) != 0) {
		MessageBox(0, "rmdir: _unlink error", "Error", MB_OK+MB_ICONERROR);
		return (-1);
	}
	return (0);
}

/*========*/

int UdiGetCatalog(char* fname)
{
  char topdir[MAX_PATH]="/";
  int Result;
  DisposeFileList(FileList);
  DriveImage[rdev][MAX_PATH-1]=0;
  if (!Result) {
    PPrevFRec=NULL;                               // starting from root
    Result=lsdir(topdir,0,NULL);
  }
  return Result;
}

int UdiFileExtract(char* OdiArchiveName, PFileRec PFRec, char* OutName)
{
  int res;
  HANDLE fh;
  FILETIME ft, lft;  /* NT filetime */
  time_t uft;   /* UZIX filetime */
  cursize = 0;
  res=xput(GetFullPath(FPath, PFRec, sizeof(FPath)-1, TRUE, '/'), OutName, 1);
  if (!res) {  /* if succesfully extracted */
    *(LPDWORD)((void*)&uft)=PFRec->FileTime;
    DosDateTimeToFileTime(uft.t_date, uft.t_time, &lft);
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

int UdiFilePack(char* OdiArchiveName, char* SrcFileName, char* ArchFileName)
{
  int res;
  HANDLE fh;
  DWORD fa;
  int mode;
  FILETIME cft, aft, mft;  /* NT filetime */
  time_t uft;              /* UZIX filetime */
  struct utimbuf times;
  if ((SrcFileName[strlen(SrcFileName)-1]=='\\')&&(ArchFileName[strlen(ArchFileName)-1]=='\\')) {
    mode = S_IFDIR;
    SrcFileName[strlen(SrcFileName)-1]=0;
    ArchFileName[strlen(ArchFileName)-1]=0;
    res=xmkdir(ConvSlash(ArchFileName));
  }
  else {
    mode = 0;
    cursize = 0;
    res=xget(SrcFileName, ConvSlash(ArchFileName), 1);
  }
  if (!res) {  /* if succesfully packed - set time and attrs */
// TODO - save creation time, owner and attributes (except X=system) for allready existed files
// chown(destname, statbuf1.st_uid, statbuf1.st_gid);
    fa=GetFileAttributes(SrcFileName);
    if (fa!=0xFFFFFFFF) {  /* set file attr */
       mode|=S_IRUSR|S_IRGRP|S_IROTH;                                                                 //  -r--r--r--
       if (fa & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)) mode|= S_IXUSR|S_IXGRP|S_IXOTH;   //  -??x??x??x
       if (!(fa & FILE_ATTRIBUTE_READONLY))                         mode|= S_IWUSR|S_IWGRP|S_IWOTH;   //  -?w??w??w?
       if (fa & FILE_ATTRIBUTE_HIDDEN)                              mode&= ~(S_IRWXG|S_IRWXO);        //  -rwx------
       UZIXchmod(ConvSlash(ArchFileName), mode);
    }
    fh = CreateFile(SrcFileName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    GetFileTime(fh, &cft, &aft, &mft);
    CloseHandle(fh);
    FileTimeToLocalFileTime(&aft, &cft);                        // access time
    FileTimeToDosDateTime(&cft, &uft.t_date, &uft.t_time);      // use cft as a buffer
    times.actime = uft;
    FileTimeToLocalFileTime(&mft, &cft);                        // modify time
    FileTimeToDosDateTime(&cft, &uft.t_date, &uft.t_time);
    times.modtime = uft;
    UZIXutime(ConvSlash(ArchFileName), &times);
  }
  return res;
}

int UdiFileDelete(char* OdiArchiveName, char* FileToDelete)
{
  int res;
  char* st;
  if (st=strstr(FileToDelete, "*.*")) {  // if deleting dirrectory
    *st=0; st--; *st=0;
    res=xrmdir(ConvSlash(StrSlashCat(FPath2, "", FileToDelete, TRUE)));
  }
  else {                                 // else delete file
    cursize = 0;
    res=xunlink(ConvSlash(StrSlashCat(FPath2, "", FileToDelete, TRUE)));
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

char* UdiGetInfo(char* ArcNm)
{
  uint j;
  fsptr fsys;
  info_t info;
  int bused, bfree, iused, ifree;
  xfs_init(rdev, 0, Panic, CheckFN(ArcNm));            // mount
  for (j = 0; j < 8; ++j) {
  	if (!UZIXgetfsys(j, &info) &&
  	    (fsys = (fsptr)info.ptr)->s_mounted) {
                bused = (fsys->s_fsize - fsys->s_isize) - fsys->s_tfree - fsys->s_reserv;
                bfree = fsys->s_tfree;
                iused = (DINODESPERBLOCK * fsys->s_isize - fsys->s_tinode);
                ifree = fsys->s_tinode;
  		sprintf(TmpBuf,"   %u / %u  blocks used/free (%u Kb / %u Kb)\n"
			       "   %u / %u  inodes used/free\n"
                               "   %u  blocks reserved for system (%u Kb)\n"
			       "   %u  total blocks (%u Kb)",
			       bused, bfree, bused/2, bfree/2, iused, ifree,
                               fsys->s_reserv, (fsys->s_reserv)/2, fsys->s_fsize, (fsys->s_fsize)/2);
		}
	}
  xfs_end(rdev);                                // umount
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

HANDLE __export WINAPI OpenArchivePart(char *ArcName, DWORD PartOffset, DWORD PartN)
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
  if (! file_exists(ArcName))
    UdiCreateArchive(ArcName);
  xfs_init(rdev, 0, Panic, ArcName);            // mount
  res=UdiGetCatalog(ArcName /*ArcFileName*/);
  xfs_end(rdev);                                // umount
  if (res>=0) {
    FileListItem=PrevFileListItem=FileList;                      // points to first item
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
      xfs_init(rdev, 0, Panic, DriveImage[rdev]);          // mount
      res=UdiFileExtract(ArcFileName, FileRec, OutName);
      xfs_end(rdev);                                       // umount
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
    int Result = 0;
    char  src_path[MAX_PATH], sub_path[MAX_PATH];
    if (! file_exists(PackedFile))
      UdiCreateArchive(PackedFile);
    xfs_init(rdev, 0, Panic, PackedFile);           // mount
    if ((UdiGetCatalog(PackedFile)<0) || (! AddList))
    {
      xfs_end(rdev);                                // unmount
      return E_UNKNOWN_FORMAT;
    }
    while (*AddList)
    {
      Result=UdiFilePack(PackedFile,
                         StrSlashCat(FPath, SrcPath, AddList, FALSE),
                         StrSlashCat(FPath2, SubPath, AddList, TRUE));
      if (Result<0)
        break;
      else
        if ((Flags & PK_PACK_MOVE_FILES) != 0)
          DeleteFile(strcat(AddSlash(SrcPath), AddList));
      AddList += strlen(AddList) + 1;
    }
    xfs_end(rdev);                                // unmount
    return Result;
}

int __export WINAPI DeleteFiles(char *PackedFile, char *DeleteList)
{
  int Result = E_UNKNOWN_FORMAT;
  if (file_exists(PackedFile))
  {
    xfs_init(rdev, 0, Panic, PackedFile);          // mount
    if (UdiGetCatalog(PackedFile)<0)
    {
      xfs_end(rdev);                               // unmount
      return Result;
    }
    Result = 0;
    while (*DeleteList)
    {
      Result=UdiFileDelete(PackedFile, DeleteList);
      if (Result<0)
        break;
      DeleteList += strlen(DeleteList) + 1;
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
  return UdiGetInfo(ArcFileName);
}


int __export WINAPI CanYouHandleThisFile(char *FileName)
{
  int Res;
  Panic=0;
  xfs_init(rdev, 0, Panic, FileName);            // mount
  Res=UdiGetCatalog(FileName);
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
                  ArcFileName, UdiGetInfo(ArcFileName));
  MessageBox(Parent, FPath, "Information", MB_OK+MB_ICONINFORMATION);
}

HANDLE __export WINAPI CreateArchivePart(char *ArcName, DWORD PartOffset, DWORD PartN, DWORD PartSize)
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
  xfs_init(rdev, 0, Panic, ArcName);            // mount
  res=UdiCreateArchive(ArcName);
  xfs_end(rdev);                                // umount
  return res;
}

