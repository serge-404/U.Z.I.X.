/*---------------------------------------------------------------*/
/* OS-depended file systems module. Block operations. (c) Serge  */
/*---------------------------------------------------------------*/

#ifdef ORI_UZIX
#include <types.h>
#include <syscalls.h>
#else
#include <cpm.h>
#endif
/* #include <ctype.h>    isdigit isalpha */
#include <string.h>
#include "filemgr.h"
#include "stringz.h"

extern char buffer[MAX_BUFF+1];

FILINFO finfo;
#ifndef ORI_UZIX
OFIL* FileOrd;
CFCB yfcb;
#endif
char *pos1, *pos2;
static BOOL res;
#ifndef ORI_UZIX
static int prevuid;
static ushort ii;
static char bd;
#endif
extern ushort Min(ushort a, ushort b);
extern int rmdir (char *path);
extern int rename(char *oldname, char *newname);

BYTE xff=0xff;

BOOL kalpha(ch)
  register char ch;
{
  return (ch>'@')&&(ch<'z');
}

BOOL kdigit(ch)
  register char ch;
{
  return (ch>='0')&&(ch<'9');
}

BOOL IsCPMpath(char* path)   /*  A0:filename.ext | A:filename.ext  */
{
  register char* pos;
  int   len;
  return (path) && (strchr(path, '/')==0) &&
         ( ( ((len=strlen(path))>1) && kalpha(*path) && ((pos=strrchr(path,':'))==path+1) ) ||
           ( (len>2) && ((pos==path+2) || (pos==path+3)) && kdigit(path[1]) ) 
         );  
}

#ifdef ORI_UZIX

/*  /path  */

BOOL IsUZIXpath(char* path)  
{
  return (path) && (! IsCPMpath(path)) && 
         (path[0]=='/' );
}

#else

/*  @A:filename.ext , @B:filename.ext ... @H:filename.ext  */

BOOL IsORDpath(path)   
  register char* path;
{
  return (path) && (! IsCPMpath(path)) && (strlen(path)>2) &&
         (strrchr(path, ':')==path+2) && (*path=='@') && kalpha(path[1]);
}

#endif

/*  0:/path/fname.ext  */
BOOL IsFATpath(char* path)  
{
  register char* pos;
  return (path) && (! IsCPMpath(path)) && 
         ((pos=strchr(path, ':'))==strrchr(path, ':')) &&
         ( kdigit(*path) && (pos==path+1) );
}

BYTE GetOSType(path)
  register char* path;
{
#ifdef ORI_UZIX
  if (IsUZIXpath(path)) {		/*  /path  */ 
    return FTYPEUZIX;
  }
#else
  if (IsCPMpath(path)) {		/*  A0:filename.ext | A:filename.ext  */ 
    return FTYPECPM;
  }
  else if (IsORDpath(path)) {		/*  @A:filename.ext , @B:filename.ext ... @H:filename.ext  */
    return FTYPEORD;
  }
#endif
  else if (IsFATpath(path)) {
    return FTYPEFAT;
  }
  return FTYPEUNK;
}

int OS_getuid()
{
#ifdef ORI_UZIX
  return 0;
#else
  return bdos(CPMSUID,255);
#endif
}

void OS_setuid(int uid)
{
#ifdef ORI_UZIX
  return;
#else
  bdos(CPMSUID, uid);
#endif
}

#ifndef ORI_UZIX

FILINFO* FcbToFInfo(CFCB* pfcb, FILINFO* finfo)
{
  register int ii;

  finfo->fsize=0;        
  finfo->fdate=0;
  finfo->ftime=(WORD)((void*)pfcb);
  finfo->fattrib=0;
  if (pfcb->ft[0] & 128) {
    finfo->fattrib|=AM_RDO;
    pfcb->ft[0]&=127;
  }
  if (pfcb->ft[1] & 128) {
    finfo->fattrib|=AM_HID;
    pfcb->ft[1]&=127;
  }
  pos1=finfo->fname; 
  pos2=pfcb->name;
  for (ii=0; ii<11; ii++) {
    if (*pos2==' ') {
      if (ii>7) break;
      ii=8; 
      pos2=pfcb->ft;
    }
    if (ii==8)
      *pos1++='.';
    *pos1++=*pos2++;
  }
  *pos1=0;
  return finfo;
}

/* PathToFcb init FCB and returns user ID from filespec:  dU:filename.ext - example A0:file.dat */

int PathToFcb(char* path, CFCB  *xfcb)  
{ 
  register char ch;
  int   i;
  int uID=255;
  xfcb->dr=xfcb->ex=xfcb->fil[0]=xfcb->fil[1]=xfcb->fil[2]=xfcb->rc=0;
  UpperCase(path);
  if (pos1=strchr(path, ':')) {
    xfcb->dr=(BYTE)(*path-'A'+1);
    if (pos1-path>1) {
      ch=*pos1;
      *pos1=0;
      uID=__atoi(path+1);
      *pos1=ch;
    }
    path=pos1+1;
  }
  memset(xfcb->name, '?', 11);
  if (*path) {
    ch=0;
    for(i=0; i<8; i++) {
      if ((! *path) || (*path=='.')) {
        if (! ch) ch=' ';
      }
      else {
        if (*path=='*') ch='?';
        path++;
      }
      xfcb->name[i]=(ch ? ch : *(path-1)); 
    }
    if (*path) {
      if (*path=='.') path++;
      ch=0;
      for(i=0; i<3; i++) {
        if (! *path) {
          if (! ch) ch=' ';
        }
        else {
          if (*path=='*') ch='?';
          path++;
        }
        xfcb->ft[i]=(ch ? ch : *(path-1)); 
      }
    }
  }
  return uID;
}

FRESULT openORDfile(fileinfo)
  register FILINFO* fileinfo;
{
  memcpy(FileOrd->name, fileinfo->fname, 9); 
  FileOrd->fileEND=(FileOrd->filePTR=FileOrd->fileBEG=fileinfo->ftime)+fileinfo->fsize;   
  return FR_OK;
}

#endif  /* ifndef ORI_UZIX */

FRESULT OS_open(OSFILE* FileObject, char* path, BYTE Flags)
{
  register BYTE res=FR_NO_FILE;
#ifdef ORI_UZIX
  struct stat *statbf;
#endif
  switch (FileObject->OSType=GetOSType(path)) {
#ifdef ORI_UZIX
    case FTYPEUZIX:
        statbf = &(((UFIL*)(void*)FileObject)->statbuf);
        if ( (((UFIL*)(void*)FileObject)->fhandle = open(path, 0)) <= 0)  /* *((int*)FileObject) = fhandle */
          break;                                                        /* TODO: call create() if Flags have FA_CREATE_ALWAYS bit */
	if ((stat(path, statbf) < 0) || S_ISDIR(statbf->st_mode) || S_ISDEV(statbf->st_mode)) 
	  break;
	return FR_OK;
#else
    case FTYPECPM:
	memset(FileObject, 0 , sizeof(CFIL));
	((CFIL*)FileObject)->prevuid=OS_getuid();  
	OS_setuid(((CFIL*)FileObject)->uid=PathToFcb(path, (CFCB*)FileObject));
	bd=bdos(CPMOPN, FileObject);
	if ((bd>=0)&&(bd<4)) res=FR_OK; 
	if (Flags & FA_CREATE_ALWAYS) {
	  if (res==FR_OK)
	    res=FR_EXIST;
	  else {
	    PathToFcb(path, (CFCB*)FileObject);
	    bd=bdos(CPMMAKE, FileObject);
	    if ((bd>=0)&&(bd<4)) res=FR_OK; 
	  }
	}
	else
	if (res!=FR_OK) OS_setuid(((CFIL*)FileObject)->prevuid);
	return res;
    case FTYPEORD:
	memset(FileObject, 0, sizeof(OFIL));
	FileOrd=(void*)FileObject;  
	FileOrd->fileDSK=(*(path+1) | 32)-'a';
        if (Flags & FA_CREATE_ALWAYS) {
	  if ((FileOrd->fileBEG=FileOrd->filePTR=scanORD(path, NULL, FIND_FREE))==FR_NO_FILE)
	    return FR_NO_FILE;
	  path+=3;
          for (ii=0; ii<8; ii++)
	    if (FileOrd->name[ii]=*path) path++; else FileOrd->name[ii]=' ';
	  return FR_OK;
	}
	else return scanORD(path, openORDfile, FIND_FIRST);
#endif
    case FTYPEFAT:
	res=f_open((void*)FileObject, path, Flags);
	break;
    default:
	return FR_INVALID_NAME;
  }
  return res;
}

FRESULT OS_close(FileObject)
  register OSFILE* FileObject;
{
  switch (FileObject->OSType) {
    case FTYPEFAT:
      return f_close((void*)FileObject);
#ifdef ORI_UZIX
    case FTYPEUZIX:
        close(((UFIL*)(void*)FileObject)->fhandle);
	return FR_OK;
#else
    case FTYPECPM:
	bd=bdos(CPMCLS, FileObject);
	if ((bd>=0)&&(bd<4)) {
	  OS_setuid(((CFIL*)FileObject)->prevuid);
	  return FR_OK; 
	}
	return FR_RW_ERROR;
    case FTYPEORD:
	if (! ((OFIL*)FileObject)->fileEND) {					/* if opened for write */
	  MemBlockGet(((OFIL*)FileObject)->fileDSK, ((OFIL*)FileObject)->fileBEG, 16, buffer);
	  ii = ((((OFIL*)FileObject)->filePTR -1) | 15) +1;
	  MemBlockPut(((OFIL*)FileObject)->fileDSK, ii, 1, (void*)&xff);	/* end of files */
	  *((WORD*)(buffer+10)) = ii - ((OFIL*)FileObject)->fileBEG - 16;	/* file size */
	  buffer[12]=0;	
	  MemBlockPut(((OFIL*)FileObject)->fileDSK, ((OFIL*)FileObject)->fileBEG, 16, buffer);
	}
	return FR_OK;
#endif
    default:
	return FR_INVALID_OBJECT;
  }
}

FRESULT OS_read(OSFILE* FileObject, void* buf, WORD cnt, WORD* readed)
{  
#ifndef ORI_UZIX
  register int blocksize=CPM_BLOCKSIZE;
#endif
  *readed=0;
  switch (FileObject->OSType) {
    case FTYPEFAT:
      return f_read((void*)FileObject, buf, cnt, readed);
#ifdef ORI_UZIX
    case FTYPEUZIX:
        if ((*readed=read(((UFIL*)(void*)FileObject)->fhandle, buf, cnt)) <= 0) {
	   *readed=0;
           return FR_RW_ERROR;
        }
        break;
#else
    case FTYPECPM:
      while (cnt) {
	if (cnt<CPM_BLOCKSIZE) blocksize=cnt;
	bdos(CPMSDMA, buf);
	if (bdos(CPMREAD, FileObject))
	  cnt=0;
	else {
	  cnt-=blocksize;
	  buf=(char*)buf+blocksize;
	  *readed+=blocksize;
	}
      }
      break;
    case FTYPEORD:
      if ( ((OFIL*)FileObject)->filePTR < ((OFIL*)FileObject)->fileEND ) {
	*readed=Min(cnt, ((OFIL*)FileObject)->fileEND - ((OFIL*)FileObject)->filePTR);
	MemBlockGet(((OFIL*)FileObject)->fileDSK, ((OFIL*)FileObject)->filePTR, *readed, buf);
	FileOrd->filePTR+=*readed;
      }
      break;
#endif
    default: ;
  }
  if (*readed>0) return FR_OK; else return FR_INVALID_OBJECT;
}

FRESULT OS_write(OSFILE* FileObject, void* buf, WORD cnt, WORD* written)
{
#ifndef ORI_UZIX
  register int blocksize=CPM_BLOCKSIZE;
#endif
  *written=0;
  switch (FileObject->OSType) {
    case FTYPEFAT:
      return f_write((void*)FileObject, buf, cnt, written);
#ifdef ORI_UZIX
    case FTYPEUZIX:
	if (*written=write(((UFIL*)(void*)FileObject)->fhandle, buf, cnt) < 0) {
           *written=0;
           return FR_RW_ERROR;
        }
        break;
#else
    case FTYPECPM:
      while (cnt) {
	if (cnt<CPM_BLOCKSIZE) blocksize=cnt;
	bdos(CPMSDMA, buf);
	bd=bdos(CPMWRIT, FileObject);
	if (bd)
	  cnt=0;
	else {
	  cnt-=blocksize;
	  buf=(char*)buf+blocksize;
	  *written+=blocksize;
	}
      }
      break;
    case FTYPEORD:
      if (! ((OFIL*)FileObject)->fileEND) {					/* if opened for write */
	if (((OFIL*)FileObject)->filePTR == ((OFIL*)FileObject)->fileBEG ) {	/* write header */
	  if ((buffer[12]) || (*buffer<0x41) || (*buffer>0x7f)) {		/* header not in file */
	    MemBlockPut(((OFIL*)FileObject)->fileDSK, ((OFIL*)FileObject)->fileBEG, 16, ((OFIL*)FileObject)->name);
	    ((OFIL*)FileObject)->filePTR += 16;
	  }
	}
	if ((*written=Min(cnt, RAMTOP-((OFIL*)FileObject)->filePTR))>0) {
	  MemBlockPut(((OFIL*)FileObject)->fileDSK, ((OFIL*)FileObject)->filePTR, *written, buf);
	  ((OFIL*)FileObject)->filePTR += *written;
	}
      }
      return FR_OK;
#endif
    default: ;
  }
  if (*written>0) return FR_OK; else return FR_RW_ERROR;
}

BOOL OS_delete(path)
  register char* path;
{
#ifdef ORI_UZIX
  struct stat buf;
#endif
  switch (GetOSType(path)) {
    case FTYPEFAT:
      return (f_unlink(koi2alt(path))==FR_OK); 
#ifdef ORI_UZIX
    case FTYPEUZIX:
	if(stat(path, &buf)) return FALSE;
	if ((buf.st_mode&S_IFMT) == S_IFDIR) 
	   return !rmdir(path);
	else
	   return !unlink(path);
#else
    case FTYPECPM:
	memset(&yfcb, 0 , sizeof(CFCB));
	prevuid=OS_getuid();  
	OS_setuid(PathToFcb(path, &yfcb));
	bd=bdos(CPMDEL, &yfcb);
	OS_setuid(prevuid);
	return (bd>=0)&&(bd<4);
    case FTYPEORD:
      return FALSE;
#endif
    default:
	return FALSE;
  }
}

BOOL OS_rename(char* src, char* dst)
{
  BYTE tsrc=GetOSType(src);
  register BYTE tdst=GetOSType(dst);
  if ((tsrc==FTYPEFAT) && (tdst==FTYPEFAT)) {
    if (dst[1]==':') dst=dst+2;
    if (*dst=='/') dst++;
    res=f_rename(koi2alt(src), koi2alt(dst));
    return (res==FR_OK); 
  }
#ifdef ORI_UZIX
  else if ((tsrc==FTYPEUZIX) && (tdst==FTYPEUZIX)) {
	return rename(src, dst);
  }
#else
  else if ((tsrc==FTYPECPM) && (tdst==FTYPECPM)) {
	prevuid=OS_getuid();  
        PathToFcb(dst, &yfcb);
        memcpy(&yfcb.dm[1], &yfcb.name[0], 11);
	OS_setuid(PathToFcb(src, &yfcb));
	bd=(BYTE)bdos(CPMREN, &yfcb);
	OS_setuid(prevuid);
	return (bd>=0)&&(bd<4);
  }
  else if ((tsrc==FTYPEORD) && (tdst==FTYPEORD)) {
    return FALSE;
  }
#endif
  return FALSE;
}

#ifndef ORI_UZIX

ushort scanORD(char* path, dir_callback OnFile, BYTE FindMode)
{
  uchar  page;
  char* pos;
  register ushort addr=0;
  if ((! path) || (! *path)) return FR_INVALID_OBJECT;
  page=(*(++path) | 32)-'a';
  path+=2;
  *buffer=finfo.fattrib=0;
  if (! page) {
#asm
    ld	A, 144
    ld	(62723), A
#endasm
  }
  while ((addr<RAMTOP)&&(*buffer!=-1)) {
    MemBlockGet(page, addr, 16, buffer);
    if (*buffer==-1) break;
    finfo.ftime=addr;
    buffer[8]=0;
    if (pos=strchr(buffer, ' ')) *pos=0;
    memcpy(finfo.fname, buffer, 9); 
    ii=*(WORD*)(buffer+10);
    addr+=ii+16;
    finfo.fsize=ii;
    if (ii<16)  
      return FR_NO_FILE;
    if (FileFilter(finfo.fname, (*path) ? path : "*")) {
      if ((FindMode==FIND_ENUM) && (OnFile(&finfo)!=FR_OK)) break;
      if (FindMode==FIND_FIRST) return OnFile(&finfo);
    }
  }
  if (FindMode==FIND_FIRST) return FR_NO_FILE;
  return addr; 
}

#endif

