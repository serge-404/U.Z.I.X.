/*---------------------------------------------------------------*/
/* FAT file system module test program R0.0. (c)2007 Serge       */
/*---------------------------------------------------------------*/

#ifdef ORI_UZIX
#include <types.h>
#else
#include <cpm.h>
#endif
#include "filemgr.h"
#include "stringz.h"

extern CFCB yfcb;
extern char buffer[MAX_BUFF+1];
extern FRESULT res; 
extern FILINFO finfo;

#ifndef ORI_UZIX

FRESULT scanCPM(char* path, dir_callback OnFile)
{
  register BYTE idx;
  int uid=OS_getuid();

  if ((! path) || (! *path)) return FR_INVALID_OBJECT;

  OS_setuid(PathToFcb(path, &yfcb));

  bdos(CPMSDMA, &buffer[0]);                                         
  idx=bdos(CPMFFST, &yfcb);                                     
  while (idx<4) {
    res=OnFile(FcbToFInfo((void*)&buffer[idx*32], &finfo));
    if (res!=FR_OK) break;
    idx=bdos(CPMFNXT, &yfcb);                                   
  }
  OS_setuid(uid);
  return FR_OK; 
}

#endif
