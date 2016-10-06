/*---------------------------------------------------------------*/
/* OS-depended file systems module. Block operations. (c) Serge  */
/*---------------------------------------------------------------*/

#include "string.h"
#include "filemgr.h"
#include "stringz.h"

extern char buffer[MAX_BUFF+1];

FILINFO finfo;
char *pos1, *pos2;
static BOOL res;
static BYTE prevuid;
static ushort ii;
static char bd;

extern ushort Min(ushort a, ushort b);

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

/*  @A:filename.ext , @B:filename.ext ... @H:filename.ext  */

BOOL IsORDpath(path)   
  register char* path;
{
  return (path) && (! IsCPMpath(path)) && (strlen(path)>2) &&
         (strrchr(path, ':')==path+2) && (*path=='@') && kalpha(path[1]);
}

/*  0:/path/fname.ext  |  /path/fname.ext  |  fname.ext */

BOOL IsFATpath(char* path)  
{
  register char* pos;
  return (path) && (! IsCPMpath(path)) && 
         ((pos=strchr(path, ':'))==strrchr(path, ':')) &&
         ( (! pos) || ( kdigit(*path) && (pos==path+1) ) );
}

BYTE GetOSType(path)
  register char* path;
{
  if (IsUZIXpath(path)) { 
    return FTYPEUZIX;
  }
  else if (IsFATpath(path)) {
    return FTYPEFAT;
  }
  return FTYPEUNK;
}

FRESULT OS_open(OSFILE* FileObject, char* path, BYTE Flags)
{
  register BYTE res=FR_NO_FILE;
  switch (FileObject->OSType=GetOSType(path)) {
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
    case FTYPEFAT:
	res=f_open((void*)FileObject, path, Flags);
	return res;
    default:
	return FR_INVALID_NAME;
  }
}

FRESULT OS_close(FileObject)
  register OSFILE* FileObject;
{
  switch (FileObject->OSType) {
    case FTYPEFAT:
      return f_close((void*)FileObject);
    case FTYPECPM:
	bd=bdos(CPMCLS, FileObject);
	if ((bd>=0)&&(bd<4)) {
	  OS_setuid(((CFIL*)FileObject)->prevuid);
	  return FR_OK; 
	}
	return FR_RW_ERROR;
    default:
	return FR_INVALID_OBJECT;
  }
}

FRESULT OS_read(OSFILE* FileObject, void* buf, WORD cnt, WORD* readed)
{  
  register int blocksize=CPM_BLOCKSIZE;
  *readed=0;
  switch (FileObject->OSType) {
    case FTYPEFAT:
      return f_read((void*)FileObject, buf, cnt, readed);
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
    default: ;
  }
  if (*readed>0) return FR_OK; else return FR_INVALID_OBJECT;
}

FRESULT OS_write(OSFILE* FileObject, void* buf, WORD cnt, WORD* written)
{
  register int blocksize=CPM_BLOCKSIZE;
  *written=0;
  switch (FileObject->OSType) {
    case FTYPEFAT:
      return f_write((void*)FileObject, buf, cnt, written);
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
    default: ;
  }
  if (*written>0) return FR_OK; else return FR_RW_ERROR;
}

BOOL OS_delete(path)
  register char* path;
{
  switch (GetOSType(path)) {
    case FTYPEFAT:
      return (f_unlink(koi2alt(path))==FR_OK); 
    case FTYPECPM:
	memset(&yfcb, 0 , sizeof(CFCB));
	prevuid=OS_getuid();  
	OS_setuid(PathToFcb(path, &yfcb));
	bd=bdos(CPMDEL, &yfcb);
	OS_setuid(prevuid);
	return (bd>=0)&&(bd<4);
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
  else if ((tsrc==FTYPECPM) && (tdst==FTYPECPM)) {
	prevuid=OS_getuid();  
        PathToFcb(dst, &yfcb);
        memcpy(&yfcb.dm[1], &yfcb.name[0], 11);
	OS_setuid(PathToFcb(src, &yfcb));
	bd=(BYTE)bdos(CPMREN, &yfcb);
	OS_setuid(prevuid);
	return (bd>=0)&&(bd<4);
  }
  return FALSE;
}

