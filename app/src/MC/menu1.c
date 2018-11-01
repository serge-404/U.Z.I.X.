#include <string.h>
#include "screen.h"
#include "windows.h"

#define MAX_PATH	100
#define PanelCount	2
#define	PromptLen	5		

#define FTYPECPM	1
#define FTYPEFAT	2
#define FTYPEUZIX	4
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
extern int	UID[PanelCount];
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
#ifdef ORI_UZIX
  kprintf("MC> ");
#else
  kprintf("FAT> ");
#endif
  PrintAligned(CmdLine, ScreenWidth-5, AL_LEFT, ' ');
  GotoXY(0, 23); 
  kprintf("^EXSD=Cur ^R=Dsk ^N=Mnu ^I=Pnl ^Z=Ins ^Q=Del ^T=Typ ^M=Cpy ^A=Mkd ^B=Ren ^C=Ext");
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
  if ((drv>='0')&&(drv<'8'))
    return FTYPEFAT;
#ifdef ORI_UZIX
  else if (drv=='/')
    return FTYPEUZIX;
#else
  else if ((drv>='A')&&(drv<='P'))	/*  A0:filename.ext | A:filename.ext  */
    return FTYPECPM;
  else if ((drv>='a')&&(drv<='h'))	/*  @A:filename.ext , @B:filename.ext ... @H:filename.ext  */
    return FTYPEORD;
#endif
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
#ifdef ORI_UZIX
    case FTYPEUZIX:					/* UZIX drv = {'/'} */
      if (! *buf) ksprintf(buf, "%c", '/');
      break;
#else
    case FTYPECPM:					/* CPM drv = {'A'...'P'} */
      ksprintf(buf, "%c%d:", drv, UID[index]);
      break;
    case FTYPEORD:					/* ORD drv = {'a'...'h'} */
      ksprintf(buf, "@%c:", drv);
      break;
#endif
    default: return NULL;
  }
  return buf;
}

void PanelChdir(char* dir, short PIndex)
{
  BYTE OsType;
  register char* path=&(Path[PIndex][0]);
  OsType=GetOSType(path);
  if ( (OsType==FTYPEFAT)
#ifdef ORI_UZIX
       || (OsType==FTYPEUZIX)
#endif
        ) {
    if (strcmp(dir,"/..")) {
      if ((path[strlen(path)-1]=='/')&&(dir[0]=='/')) dir++;
      strcat(path, dir);
    }
    else {
      if (path=strrchr(path, '/')) {
        *(path)=path[1]=0;
      }
    }
  }
#ifndef ORI_UZIX
  else if (OsType==FTYPECPM) {
    UID[PIndex]=__atoi(dir+2);
  }
#endif
  UpdatePanel(PIndex);
}

void ExecCmd(uchar cmd, ushort pnl)	/* lb1 = active listbox */
{
  register char *Item=StringsItem(lb1->Items, lb1->ItemIndex)+OBJSIZE+1;
  BYTE OsType;
  int nextpnl=(pnl) ? 0 : 1;		/* inverse panel path - for another panel operations */

  if ((*Item=='/') && (cmd!=CMD_DEL) && (cmd!=CMD_REN) && (cmd!=CMD_LINE)) return;
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
    if ( (((OsType=GetOSType(pos1))==FTYPEFAT)&&(*Item!='/')) ||
         ((OsType==FTYPEUZIX)&& pos1[1]) )
      strcat(CmdLine, "/");
    strcat(CmdLine, Item);
    if (twopar) {
      strcat(CmdLine, StrSpace);
      strcat(CmdLine, (cmd==CMD_REN) ? pos1 : pos2);
      if ( (((OsType=GetOSType(pos2))==FTYPEFAT)&&(*Item!='/')) ||
           ((OsType==FTYPEUZIX)&& pos2[1]) )
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
    bputs("\n");                                /* also does bflush() */
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

