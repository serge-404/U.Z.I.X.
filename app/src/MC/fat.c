/*---------------------------------------------------------------*/
/* FAT file system module test program R0.0. (c)2007 Serge       */
/*---------------------------------------------------------------*/

#include "screen.h"
#include <stdlib.h>
#include <string.h>

#include "filemgr.h"
#include "stringz.h"

#ifdef ORI_UZIX
#include <errno.h>
#include <syscalls.h>
extern int execvp(char *pathname, char *argv[]);          /* exec.h */
extern int waitpid(int pid, int *stat_loc, int options);  /* wait.h */
extern int kprintfile;
#endif

#define __MENU

#define CODE_KOI8	0
#define CODE_ALT	1
#define CODE_KOI7	2

OSFILE fsrc, fdst;

FRESULT res;         /* FatFs function common result code */
FATFS   fatfs[2];    /* File system object for each logical drive */

DWORD DriveSize[2];
int  TotalDrives=0;
BYTE DriveFAT[2]={255, 255};

char buffer[MAX_BUFF+1];
char CmdLine[150];

BOOL MenuMode;
WORD br, bw; 
uchar ch, prevch;
int cnt;

#ifdef ORI_UZIX
char HelpStr[]=
  "\nSyntax: mc [<command>] [<filemask> [<filemask>]]"
  "\n\twhere <command>: dir  <filemask> - show files list\n\t\t\ttype  <filemask> - type file content, KOI8r"
  "\n\t\t\tatype, ktype <fl>- type Alt=cp1251 or KOI7 file"
  "\n\t\t\tdel   <filemask> - delete file or catalog\n\t\t\tcopy  <inpmask> <outmask> - copy from inp to out"
  "\n\t\t\tren   <inpmask> <outmask> - rename inp to out\n\t\t\tmkdir <dirname>  - create catalog by full path"
  "\n\t\t\tinfo  <drive>    - show filesystem information\n\t\t\tstat  <filemask> - show file attributes"
  "\n\t\t\tchmod <filemask> - change file attributes"
  "\n <filemask> - file specification with full dirrectory path and(or) wildcards ?*"
  "\n\t\tUZIX filemask: /path/filename - file with name under path"
  "\n\t\tFAT filemask: 0:/dir/dir2/file1.* - all files with `file1` name"
  "\n\t\t\t\t\tin `dir/dir2` catalog of master drive"
  "\n\nExample: FAT copy 1:/test/aaa.bbb /home/cc.ddd";  

char UsageStr[]="\nUse `mc HELP` to read about command systax, `mc MENU` for panels commander.\n";
char TitleStr[]="\nMiniCommander v1.4. Serving files on FAT and generic volumes. (c)2018 Serge";

#else
char HelpStr[]=
  "\nSyntax: FAT <command> [<filemask> [<filemask>]]"
  "\n\twhere <command>: dir  <filemask> - show files list\n\t\t\ttype  <filemask> - type file content, KOI8r"
  "\n\t\t\tatype, ktype <fl>- type Alt=cp1251 or KOI7 file\n\t\t\tfmtord <ord_dsk> - format ORDOS disk @D:"
  "\n\t\t\tdel   <filemask> - delete file or catalog\n\t\t\tcopy  <inpmask> <outmask> - copy from inp to out"
  "\n\t\t\tren   <inpmask> <outmask> - rename inp to out\n\t\t\tmkdir <dirname>  - create catalog by full path"
  "\n\t\t\tinfo  <drive>    - show filesystem information\n\t\t\tstat  <filemask> - show file attributes"
  "\n\t\t\tchmod <filemask> - change file attributes"
  "\n <filemask> - file specification with full dirrectory path and(or) wildcards ?*"
  "\n\t\tCP/M filemask: A0:*.ext - files with any names and extension"
  "\n\t\t\t\t`ext` in user 0 of drive A. `0`-optional" 
  "\n\t\tFAT filemask: 0:/dir/dir2/file1.* - all files with `file1` name"
  "\n\t\t\t\t\tin `dir/dir2` catalog of master drive\n\t\t\t\t\tcatalog and drive names are optional"
  "\n\t\tORDOS filemask: @D:* - any files. Path allways starts with @"
  "\nExample: FAT copy 1:/test/aaa.bbb B3:cc.ddd";  

char UsageStr[]="\nUse `FAT HELP` to read about command systax, `FAT MENU` for panels commander.\n";
char TitleStr[]="\nFAT v1.4 - utility for serving files on FAT volumes. (c) 2007-2010 Serge A.\n";
char NoIDEBDOS[]="No IDE drives or IDEBDOS driver not installed\n";
#endif

char StrFMount[]="FAT mount";
char StrType[]="TYPE";
char StrAType[]="ATYPE";
char StrKType[]="KTYPE";
char StrCopy[]="COPY";
char StrMkdir[]="MKDIR";
char StrDel[]="DEL";
char StrRen[]="REN";

extern BOOL kdigit(char ch);

void perror(char* txt, FRESULT res)
{
  kprintf("\nError: %s (%d)\n\n", txt, res);
}

#ifdef __MENU
void do_usage()
{
  bputs(UsageStr);
}
#endif

BOOL do_help()
{
#ifdef __MENU
  do_usage();
#endif
  bputs(HelpStr);
  return FALSE;           
}

BOOL do_info(path)
  register char* path;
{
  DWORD p2;
  FATFS *fs;
  switch (GetOSType(path)) {
#ifdef ORI_UZIX
    case FTYPEUZIX:
	kprintf("\nUZIX filesystem\n");
	return TRUE;
#else
    case FTYPECPM:
	kprintf("\nCPM filesystem\n");
	return TRUE;
    case FTYPEORD:
	kprintf("\nORDOS filesystem\n");
	return TRUE;
#endif
    case FTYPEFAT:
	kprintf("\nGetInfo. Please wait...\n");
	res=f_getfree(path, &p2, &fs);
	if (res) 
	  perror("f_getfree", res);
	else {
	  if (fs->fs_type==FS_FAT12) strcpy(buffer, "FAT12");
	  else if (fs->fs_type==FS_FAT16) strcpy(buffer, "FAT16");
	  else if (fs->fs_type==FS_FAT32) strcpy(buffer, "FAT32");
	  else strcpy(buffer, "UNKNOWN");

	  kprintf("\nFAT type = %s\nBytes/Cluster = %lu\nNumber of FATs = %u\n",
            buffer, (DWORD)fs->csize * 512, (WORD)fs->n_fats);
	  kprintf("Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n",
            fs->n_rootdir, fs->sects_fat, (DWORD)fs->max_clust - 2);
	  kprintf("FAT start (lba) = %lu\nDIR start (lba,cluster) = %lu\nData start (lba) = %lu",
            fs->fatbase, fs->dirbase, fs->database);
	  kprintf("\n%lu KB total.\n%lu KB available.\n",
            ((DWORD)fs->csize / 2)*(fs->max_clust -2), p2 * (fs->csize / 2) );
	  return TRUE;
	}
    default:
	return do_help();
  }
}

void TestEsc()
{
  if (kbhit() && ( ((ch=Inkey())==27) || (ch==3) ) ) exit(1);
}

#ifdef ORI_UZIX
FRESULT dirUZIXfile(FILINFO* fileinfo)
{
  bputs(fileinfo->fname);
  bputs("\n");
  TestEsc();
  return FR_OK;
}

#else
FRESULT dirCPMfile(FILINFO* fileinfo)
{
  kprintf(" %s\n", fileinfo->fname);
  TestEsc();
  return FR_OK;
}

FRESULT dirORDfile(fileinfo)
  register FILINFO* fileinfo;
{
  kprintf(" %s     \t%x\n", fileinfo->fname, fileinfo->fsize);
  TestEsc();
  return FR_OK;
}

BOOL do_fmtord(path)
  register char* path;
{
  if (IsORDpath(path)) {
    *buffer=0xff;
    MemBlockPut((*(path+1) | 32)-'a', 0, 1, buffer);
    return TRUE;
  }
  return do_help();
}
#endif

FRESULT dirFATfile(fileinfo)
	register FILINFO* fileinfo;
{
  if (! fileinfo) return FALSE;
  kprintf("%02d\.%02d\.%04d  %02d:%02d   ",
          fileinfo->fdate & 31, (fileinfo->fdate >>5) & 15,
          (WORD)1980 + (WORD)((fileinfo->fdate >>9) & 127) ,
          (fileinfo->ftime >>11) & 31, (fileinfo->ftime >>5) & 63);
  if (fileinfo->fattrib & AM_DIR) 
    kprintf("<DIR>       ");
  else
    kprintf("%12lu", fileinfo->fsize);
  kprintf("  %s\n", alt2koi(fileinfo->fname));
  TestEsc();
  return FR_OK;
}

BOOL do_dir(path)
	register char* path;
{
  switch (GetOSType(path)) {
#ifdef ORI_UZIX
    case FTYPEUZIX:
	return scanUZIX(path, dirUZIXfile);
#else
    case FTYPECPM:
	return scanCPM(path, dirCPMfile);
    case FTYPEORD:
	return scanORD(path, dirORDfile, FIND_ENUM);
#endif
    case FTYPEFAT:
	return scanFAT(koi2alt(path), dirFATfile); 
    default:
	return do_help();
  }
}

BOOL do_mkdir(path)
  register char* path;
{
  if (IsFATpath(path))
    return (f_mkdir(koi2alt(path))==FR_OK); 
  return do_help();
}

BOOL do_del(char* path)
{
  if (OS_delete(path)) return TRUE;
  return do_help();
}

BOOL do_ren(char* src, char* dst)
{
  if (OS_rename(src, dst)) return TRUE;
  return do_help();
}

BOOL do_type(char* path, int codeset)
{
#ifndef ORI_UZIX
  register BOOL IsORD=IsORDpath(path);
#endif
  cnt=0;
  if (IsFATpath(path)) koi2alt(path);
  memset(&fsrc, 0, sizeof(fsrc));
  res = OS_open(&fsrc, path, FA_OPEN_EXISTING | FA_READ);
  if (res) perror("f_open", res);
#ifndef ORI_UZIX
  if (IsORD)
    OS_read(&fsrc, buffer, 16, &br);		/* skip ORDOS header */
#endif
  while (cnt!=27) {
    res = OS_read(&fsrc, buffer, MAX_BUFF, &br);
    if (res || (! br)) break;
    buffer[MAX_BUFF]=0; 
    if (codeset==CODE_ALT)
      alt2koi(buffer); 
    for (bw=0; bw<br; bw++) {
      ch=buffer[bw];
      if ((codeset==CODE_KOI7) && (ch>0x5f) && (ch<0x80)) ch|=0x80;
      if (ch==0x1A) break; 
      bputch(ch);
#ifdef ORI_UZIX
      if (ch=='\x0a') {
#else
      if (ch=='\x0d') {
        if (IsORD) bios(NCONOUT, '\x0a');
#endif
	if ((++cnt)==ScreenHeight) {
	  if ((cnt=Inkey())==27) break;	/* ESC */
	  cnt=0; 
	}
      }
#ifndef ORI_UZIX
      if (prevch=='\x0d') {
	if (ch=='\x0a')
	  IsORD=FALSE;
        else {
	  if ((! IsORD)&&(ch!='\x0d')) bios(NCONOUT, '\x0a');
	  IsORD=ch!='\x0d';
	}
      }
#endif
      prevch=ch;
    }
  }
#ifdef __MENU
  if ((MenuMode)&&(cnt!=27)) Inkey();
#endif
  OS_close(&fsrc);
  return TRUE;
}

BOOL do_copy(char* src, char* dst)
{
  register char *pos;

  strcpy(&buffer[256], dst);
  dst=&buffer[256];

  if (IsFATpath(src)) {
    koi2alt(src);
    prevch='/';
  }
  else
    prevch=':';
  if (IsFATpath(dst)) { 
    koi2alt(dst);
    ch='/';
  }
  else
    ch=':';
  if (dst[strlen(dst)-1]==ch) {
    if (pos=strrchr(src, prevch)) {
       pos++;
       strcat(dst, pos);
    }
  }
#ifndef ORI_UZIX
  if (IsORDpath(src) && (! strchr(dst, '.'))) strcat(dst, ".ORD");
#endif
  strcpy(buffer, "file open: "); 
  if (OS_open(&fsrc, src, FA_OPEN_EXISTING | FA_READ))
    { perror(strcat(buffer, src), 1); return FALSE; }
  if (OS_open(&fdst, dst, FA_CREATE_ALWAYS | FA_WRITE))
    { perror(strcat(buffer, dst), 2); return FALSE; }
  
  for (;;) {                                             
      res = OS_read(&fsrc, buffer, MAX_BUFF, &br);
      if (res || (! br)) break;                        
      res = OS_write(&fdst, buffer, br, &bw);
      if (res || (bw < br)) break;                         
  }
  OS_close(&fsrc);
  OS_close(&fdst);
  return TRUE;
}

#define GET_SECTOR_COUNT	1
#define RES_OK			0

extern BYTE disk_ioctl(BYTE, BYTE, void*);

#ifdef __MENU
extern void do_menu();
#endif

#ifndef ORI_UZIX
void ExitNoFat()
{ 
  bputs(NoIDEBDOS);
  exit (-1); 
}

void CheckIDE()
{
  DriveSize[0]=DriveSize[1]=0;
  if ((disk_ioctl(0, GET_SECTOR_COUNT, &DriveSize[0]) == RES_OK) && 
      (DriveSize[0]>1)) 
    TotalDrives++;
  if ((disk_ioctl(1, GET_SECTOR_COUNT, &DriveSize[1]) == RES_OK) &&
      (DriveSize[1]>1)) 
    TotalDrives++;
}
#endif

BOOL eq(char* st1, char* st2, BOOL param_ok)
{
  return (strcmp(st1, st2)==0) && (param_ok);
}

BOOL MountFAT(path, Index)
  char* path;
  register int Index;
{
/*
  FATFS *fs=(void*)HelpStr;
  if (Index) fs=fs+sizeof(FATFS);
*/
  DriveFAT[Index] = (kdigit(*path) ? (*path - '0') : 0);
  return f_mount(DriveFAT[Index], /* fs */ &fatfs[Index])==FR_OK;
}

void UnMountFAT(Index)
  register int Index;
{
  if (DriveFAT[Index] != 255) {
    f_mount(DriveFAT[Index], NULL);
    DriveFAT[Index]=255;
  }
}

#ifdef ORI_UZIX
#define ARGC_MAX 10
void do_exec(int bargc, char* bargv[])
{
	int pid1 = 0, pid2 = 0, bgp = 0, status;
        register int i;
	char *xargv[ARGC_MAX];

kprintf("\n EXEC: %s \n", bargv[0]);

        xargv[ARGC_MAX-1] = 0;
	bgp = (strcmp(bargv[bargc-1],"&") == 0);
	if (bgp) bargv[--bargc] = 0;
	if ((pid1 = fork()) < 0) {
		kprintf(" fork failed\n");
		return;
	}
	if (pid1) {
		status = 0;
		if (bgp)
			kprintf("[%d]: %s\n", pid1, bargv[0]);
		else
			while ((((pid2 = waitpid(-1 /*WAIT_ANY*/, &status, 0) ) < 0) &&
				(errno == EINTR)) || (pid2 != pid1));
		ioctl(0 /*STDIN_FILENO*/, 0 /*TTY_COOKED*/);
		if (status & 0x00ff) {
                        kprintfile=2; /*STDERR_FILENO*/
			kprintf("pid %d: %s (signal %d)\n", pid1,
				(status & 0x80) ? "core dumped" : "killed",
				(status & 0x7f));
                        kprintfile=1; /*STDOUT_FILENO*/
		}
		return;
	}
	/* We are the child, so run the program. */
	execvp(bargv[0], bargv);
	if (errno == ESHELL) {
		for (i = ARGC_MAX-2; i > 0; --i)
			xargv[i] = bargv[i-1];
		xargv[0] = "/bin/sh";
		execvp(xargv[0], xargv);
	}
	kprintf(" %s: error %d\n",bargv[0], errno);
	exit(1);
}
#endif

void ProcessParams(__argc, __argv[])
	int __argc;
	register char* __argv[];
{
    UpperCase(CmdLine);
    if (eq(CmdLine, "INFO", __argc==3))
      do_info(__argv[2]);
    else if (eq(CmdLine, "DIR", __argc==3))
      do_dir(__argv[2]);
    else if (eq(CmdLine, StrType, __argc==3))
      do_type(__argv[2], CODE_KOI8);
    else if (eq(CmdLine, StrAType, __argc==3))
      do_type(__argv[2], CODE_ALT);
    else if (eq(CmdLine, StrKType, __argc==3))
      do_type(__argv[2], CODE_KOI7);
    else if (eq(CmdLine, StrCopy, __argc==4))
      do_copy(__argv[2], __argv[3]);
    else if (eq(CmdLine, StrMkdir, __argc==3))
      do_mkdir(__argv[2]);
    else if (eq(CmdLine, StrDel, __argc==3))
      do_del(__argv[2]);
    else if (eq(CmdLine, StrRen, __argc==4))
      do_ren(__argv[2], __argv[3]);
#ifndef ORI_UZIX
    else if (eq(CmdLine, "FMTORD", __argc==3))
      do_fmtord(__argv[2]);
#endif
    else {
#ifdef ORI_UZIX
      if (MenuMode) {
        if (*CmdLine) do_exec(__argc, __argv);
      }
      else
#endif
      do_help();
    }
}

int main (argc, argv)
  register int argc;
  char* argv[];
{
  if (argc<2) {
    bputs(TitleStr);
#ifdef __MENU
#ifdef ORI_UZIX
    for (ch=0; ch<ScreenHeight; ch++) bputs("\0x13\0x1b\0x4c");   /* Insert line = ESC+L */
    MenuMode=TRUE;
    do_menu();
#else
    do_usage();
#endif
#else
    do_help();
#endif
    return 1;
  }
  strcpy(CmdLine, argv[1]);
  UpperCase(CmdLine);
  if (eq(CmdLine, "HELP", 1)) 
    do_help();
  else {
#ifndef ORI_UZIX
    CheckIDE();
#endif
    if ((argc>=3) && IsFATpath(argv[2])) {
#ifndef ORI_UZIX
      if (! TotalDrives) ExitNoFat();
#endif
      if (! MountFAT(argv[2],0)) {
 	perror(StrFMount, 1);
	return 1;
      }
    } 
    if ((argc>=4) && IsFATpath(argv[3])) {
#ifndef ORI_UZIX
      if (! TotalDrives) ExitNoFat();
#endif
      if (! MountFAT(argv[3],1)) {
	perror(StrFMount, 2);
	return 1;
      }
    } 
#ifdef __MENU
    if (MenuMode=eq(CmdLine, "MENU", argc==2))
      do_menu();
    else
#endif
      ProcessParams(argc, argv);

    /* Unregister a work areas before discard it */
    UnMountFAT(0);
    UnMountFAT(1);
  }
  return 0;
}

