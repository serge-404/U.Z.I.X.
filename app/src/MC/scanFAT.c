/*---------------------------------------------------------------*/
/* FAT file system module test program R0.0. (c)2007 Serge       */
/*---------------------------------------------------------------*/

#include <string.h>
#include "filemgr.h"
#include "stringz.h"
#include "screen.h"

#ifndef ORI_UZIX
extern CFCB yfcb;
#endif
extern char buffer[MAX_BUFF+1];
extern FRESULT res; 
extern FILINFO finfo;
extern char *pos1, *pos2;

FRESULT scanFAT(char* path, dir_callback OnFile)
{
  DIR dir;
  register char *mask;
  if ((! path) || (! *path)) return FR_INVALID_OBJECT;
  strcpy(buffer, path);
  mask=NULL;
  pos2=strchr(buffer, '?');
  pos1=strchr(buffer, '*');
  if ((pos1) || (pos2)) {
    mask=pos2;
    if ((! mask) || ((mask>pos1)&&(pos1))) mask=pos1;
  }
  for ( ; (mask)&&(&mask[0]>&buffer[0])&&(*mask!='/'); mask--);
  if (mask==buffer) mask=NULL;
  if (mask) {
    *mask=0;
    mask++;
  }
  if ((res = f_opendir(&dir, buffer)) == FR_OK)
    while (((res = f_readdir(&dir, &finfo)) == FR_OK) && finfo.fname[0])
      if (mask) {
        if ( FileFilter(finfo.fname, mask) && (OnFile(&finfo)!=FR_OK) ) break;
      }
      else if (OnFile(&finfo)!=FR_OK) break;
  return res;
}

