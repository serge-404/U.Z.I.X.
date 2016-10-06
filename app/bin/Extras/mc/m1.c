#include "string.h"
#include "screen.h"
#include "windows.h"

#define MAX_PATH	100
#define PanelCount	2
#define	PromptLen	5		

#define FTYPECPM	1
#define FTYPEFAT	2
#define FTYPEORD	3
#define MAX_BUFF	512

#define CMD_LINE	0
#define CMD_COPY	1
#define CMD_TYPE	2
#define CMD_KTYPE	3
#define CMD_ATYPE	4
#define CMD_DEL		5
#define CMD_REN		6
#define CMD_MKDIR	7

#define MAX_ARGV	4

extern	short	WndRow, WndCol, WndWidth, WndHeight;
extern TWINDOW* Panel[PanelCount], *wnd1;
extern TLISTBOX* FileList[PanelCount], *lb1;
extern char	Path[PanelCount][MAX_PATH];
extern ushort	UID[PanelCount];
extern BYTE	DRV[PanelCount];
extern char	buffer[MAX_BUFF+1];
extern char	buf15[15];
extern BYTE DriveFAT[2];
extern short	CmdLinePos;
extern char	CmdLine[150];
extern char 	*pos1, *pos2;
extern void*	topwnd;			/* currently operating window (top window) */


extern char StrType[];
extern char StrAType[];
extern char StrKType[];
extern char StrCopy[];
extern char StrMkdir[];
extern char StrDel[];
extern char StrRen[];

extern int __atoi(char *st);
extern BYTE GetOSType(char* path);
extern	void UpdatePanel(int index);

short wRow, wCol, wWidth, wHeight;
BOOL twopar;
char *cmdptr;
char* kargv[MAX_ARGV+1];

char StrSpace[]=" ";

extern char StrError[];

void FullScreen()
{
  wRow=WndRow; wCol=WndCol; wWidth=WndWidth; wHeight=WndHeight;
  WndRow=WndCol=0; WndWidth=ScreenWidth; WndHeight=ScreenHeight;
}

void WndScreen()
{
  WndRow=wRow; WndCol=wCol; WndWidth=wWidth; WndHeight=wHeight;
}

void promptxy()
{
  FullScreen();
  GotoXY(PromptLen+CmdLinePos, 22);
  WndScreen();
}

void PrintKeyBar()
{
  FullScreen();
  GotoXY(0, 22); 
  kprintf("FAT> ");
  PrintAligned(CmdLine, ScreenWidth-5, AL_LEFT, ' ');
  GotoXY(0, 23); 
  kprintf("^EXSD=Cur ^Z=Ins ^Q=Del ^T=Typ ^I=Pnl ^M=Cpy ^N=Mnu ^R=Dsk ^A=Mkd ^B=Ren ^C=Ext");
  WndScreen();
}

void cmdexec(BOOL ClrScr)	/* TYPE A:AAA.AAA */
{
  register int ii;
  cmdptr=CmdLine;
  for (ii=1; ii<=MAX_ARGV; ii++) {
    while (*cmdptr==' ') cmdptr++;
    if (! *cmdptr) break;
    kargv[ii]=cmdptr;
    while (*cmdptr>' ') cmdptr++;
    if (! *cmdptr) break;
    *cmdptr++=0;
  }
  if (ii>1) ProcessParams(ii+1, kargv);
  *CmdLine=0;
  if (ClrScr) ClearScreen();
  PrintKeyBar();
}

BYTE GetOSDrive(drv)
	register char drv;
{
  if ((drv>='A')&&(drv<='P'))		/*  A0:filename.ext | A:filename.ext  */ 
    return FTYPECPM;
  else if ((drv>='0')&&(drv<'8'))
    return FTYPEFAT;
  else if ((drv>='a')&&(drv<='h'))	/*  @A:filename.ext , @B:filename.ext ... @H:filename.ext  */
    return FTYPEORD;
  return 0;
}

char* PanelPath(char* buf, ushort index)
{
  register ushort drv=DRV[index];
  switch (GetOSDrive(drv)) {				/* FAT drv = {'0'...'7'} */
    case FTYPEFAT:
      *buffer=buffer[1]=0;
      strcpy(buffer,strrchr(buf,':'));
      ksprintf(buf, "%c:%s", drv, buffer+1);
      break;
    case FTYPECPM:					/* CPM drv = {'A'...'P'} */
      ksprintf(buf, "%c%d:", drv, UID[index]);
      break;
    case FTYPEORD:					/* ORD drv = {'a'...'h'} */
      ksprintf(buf, "@%c:", drv);
      break;
    default: return NULL;
  }
  return buf;
}

void PanelChdir(char* dir, short PIndex)
{
  BYTE OsType;
  register char* path=&(Path[PIndex][0]);
  OsType=GetOSType(path);
  if (OsType==FTYPECPM) {
    UID[PIndex]=__atoi(dir+2); 
  }
  else if (OsType==FTYPEFAT) {
    if (dir[1]=='.') 
      *(strrchr(path, '/'))=0;
    else
      strcat(path, dir);   
  }
  UpdatePanel(PIndex);
}

void ExecCmd(uchar cmd, ushort pnl)	/* lb1 = active listbox */
{
  register char *Item=StringsItem(lb1->Items, lb1->ItemIndex)+OBJSIZE+1;
  int nextpnl=(pnl) ? 0 : 1;		/* inverse panel path - for another panel operations */

  if ((*Item=='/') && (cmd!=CMD_DEL) && (cmd!=CMD_REN)) return;
  if (pnl) pnl=1;
  pos2=Path[nextpnl];		/* dstpath */
  pos1=Path[pnl];		/* srcpath */

  twopar=FALSE;
  switch (cmd) {
    case CMD_COPY:
        if (! strcmp(pos1, pos2)) {
          MessageBox("Can`t copy to itself", StrError, 0);
          return;
        }
	cmdptr=StrCopy;
	twopar=TRUE;
	break;
    case CMD_TYPE:
	cmdptr=StrType;
	break;
    case CMD_KTYPE:
	cmdptr=StrKType;
	break;
    case CMD_ATYPE:
	cmdptr=StrAType;
	break;
    case CMD_DEL:
	cmdptr=StrDel;
	break;
    case CMD_REN:
	cmdptr=StrRen;
	twopar=TRUE;
	break;
    case CMD_MKDIR:
	cmdptr=StrMkdir;
	break;
    default: ;
  }
  if (cmd!=CMD_LINE) {
    strcpy(CmdLine, cmdptr);
    strcat(CmdLine, StrSpace);
    strcat(CmdLine, pos1);
    if ((GetOSType(pos1)==FTYPEFAT) && (*Item!='/')) 
      strcat(CmdLine, "/");
    strcat(CmdLine, Item);
    if (twopar) {
      strcat(CmdLine, StrSpace);
      strcat(CmdLine, (cmd==CMD_REN) ? pos1 : pos2);
      if ((GetOSType(pos2)==FTYPEFAT) && (*Item!='/')) 
        strcat(CmdLine, "/");
      if (cmd==CMD_REN)
        return;					/* TODO: GetString dialog */
      else
        strcat(CmdLine, Item);
    }
    PrintKeyBar();
  }
  twopar=(cmd==CMD_TYPE)||(cmd==CMD_ATYPE)||(cmd==CMD_KTYPE)||(cmd==CMD_LINE);
  if (twopar) {
    Item=topwnd;
    topwnd=NULL;				/* stop window buffering */
    FullScreen();
    ClearScreen();
  }
  cmdexec(twopar);
  if (twopar) topwnd=Item;			/* continue window buffering */
  SendMessage(Panel[nextpnl], WE_PAINT, 1);	/* redraw window but do not redraw controls */
  UpdatePanel(nextpnl);
  SendMessage(Panel[nextpnl], WE_DEACTIVATE, 0);
  SendMessage(Panel[pnl], WE_PAINT, 1);		/* redraw window but do not redraw controls */
  UpdatePanel(pnl);
  if (twopar) WndScreen();
}

/*=====================================*/

BOOL do_type(char* path, int codeset)
{
  register BOOL IsORD=IsORDpath(path);
  cnt=0;
  if (IsFATpath(path)) koi2alt(path);
  res = OS_open(&fsrc, path, FA_OPEN_EXISTING | FA_READ);
  if (res) perror("f_open", res);
  if (IsORD)
    OS_read(&fsrc, buffer, 16, &br);		/* skip ORDOS header */
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
      bios(NCONOUT, ch);
      if (ch=='\x0d') {
        if (IsORD) bios(NCONOUT, '\x0a');
	if ((++cnt)==ScreenHeight) {
	  cnt=Inkey();
	  if (cnt==27) break;	/* ESC */
	  cnt=0; 
	}
      }
      if (prevch=='\x0d') {
	if (ch=='\x0a')
	  IsORD=FALSE;
        else {
	  if ((! IsORD)&&(ch!='\x0d')) bios(NCONOUT, '\x0a');
	  IsORD=ch!='\x0d';
	}
      }
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
  if (IsORDpath(src) && (! strchr(dst, '.'))) strcat(dst, ".ORD");

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

