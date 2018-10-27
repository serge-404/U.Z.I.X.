#ifdef ORI_UZIX
#include <types.h>
#include <ioctl.h>
#else
#include <cpm.h>
#endif
#include "windows.h"
#include "filemgr.h"

#define ScreenWidth	80
#define ScreenHeight	24
#define PanelCount	2
#define PanelWidth	ScreenWidth/PanelCount
#define PanelHeight	(ScreenHeight-2)
#define	PromptLen	5		
#define MAX_PATH	100

#define KEYCTRLA	1
#define KEYCTRLB	2
#define KEYCTRLN	14
#define KEYCTRLP	16
#define KEYCTRLR	18
#define KEYCTRLT	20
#define KEYCTRLY	25

#define CMD_LINE	0
#define CMD_COPY	1
#define CMD_TYPE	2
#define CMD_KTYPE	3
#define CMD_ATYPE	4
#define CMD_DEL		5
#define CMD_REN		6
#define CMD_MKDIR	7

extern	char ScreenBuf[];
extern	short	wRow, wCol;
extern	void cmdexec(BOOL ClrScr);
extern	char* PanelPath(char* buf, ushort index);
extern	void PanelChdir(char* dir, short PIndex);
extern	BYTE GetOSDrive(char drive);
extern BOOL MountFAT(char* path, int Index);
extern void UnMountFAT(int Index);
extern void PrintKeyBar();
extern void promptxy();

extern void ProcessParams(int __argc, char* __argv[]);
extern void ExecCmd(uchar cmd, ushort pnl);
extern void FullScreen();
extern void WndScreen();

extern char	CmdLine[150];
extern void*	topwnd;			/* currently operating window (top window) */
extern BOOL	AltairDos;

extern char StrFMount[];
extern char NoIDEBDOS[];
extern int  TotalDrives;

TWINDOW* Panel[PanelCount], *wnd1;
TLISTBOX* FileList[PanelCount], *lb1;
char	Path[PanelCount][MAX_PATH];
int	UID[PanelCount];
uchar	DRV[PanelCount];
short	CmdLinePos=0;
char	buf15[15];
ushort	sublvl;

TSTRINGS* lbItems;

char StrError[]=" Error ";

char SelDiskStrings[]=
#ifdef ORI_UZIX
  "/ \x00" "UZIX /\x00"
  "0 \x00" "FAT 0:\x00"
  "1 \x00" "FAT 1:\x00"
  "2 \x00" "FAT 2:\x00"
  "3 \x00" "FAT 3:\x00"
  "4 \x00" "FAT 4:\x00"
  "5 \x00" "FAT 5:\x00"
  "6 \x00" "FAT 6:\x00"
  "7 \x00" "FAT 7:\x00";
#else
  "A \x00" "CPM A:\x00"
  "B \x00" "CPM B:\x00"
  "C \x00" "CPM C:\x00"
  "D \x00" "CPM D:\x00"
  "E \x00" "CPM E:\x00"
  "F \x00" "CPM F:\x00"
  "G \x00" "CPM G:\x00"
  "H \x00" "CPM H:\x00"
  "I \x00" "CPM I:\x00"
  "J \x00" "CPM J:\x00"
  "K \x00" "CPM K:\x00"
  "L \x00" "CPM L:\x00"
  "M \x00" "CPM M:\x00"
  "N \x00" "CPM N:\x00"
  "O \x00" "CPM O:\x00"
  "P \x00" "CPM P:\x00"
  "0 \x00" "FAT 0:\x00"
  "1 \x00" "FAT 1:\x00"
  "2 \x00" "FAT 2:\x00"
  "3 \x00" "FAT 3:\x00"
  "4 \x00" "FAT 4:\x00"
  "5 \x00" "FAT 5:\x00"
  "6 \x00" "FAT 6:\x00"
  "7 \x00" "FAT 7:\x00"
  "a \x00" "ORD a:\x00"
  "d \x00" "ORD d:\x00"
  "e \x00" "ORD e:\x00"
  "f \x00" "ORD f:\x00"
  "g \x00" "ORD g:\x00"
  "h \x00" "ORD h:\x00";
#endif

int ItemsCompare(char* item1, char* item2)		/* TODO: sort modes - by_ext, by_name */
{
  return 0;
}

uchar ScanCPMfile(fileinfo)
	register FILINFO* fileinfo;
{
  if (fileinfo->fattrib & AM_DIR) {
    *buf15='/';
    strcpy(buf15+1, fileinfo->fname);
    StringsAdd(lbItems, buf15);
  }
  else
    StringsAdd(lbItems, fileinfo->fname);
  return 0;
}

void UpdatePanel(int index)
{
#ifndef ORI_UZIX
  register int ii;
#endif
  BYTE OsType;
  char* path=&(Path[index][0]);
  lb1=FileList[index];

DBGPRT("UP1 ");

  PanelPath(path, index);
  OsType=GetOSType(path);
  Panel[index]->Caption=path;
  ListBoxClear(lb1, FALSE);
  lbItems=lb1->Items;

DBGPRT("UP2 ");

  switch (OsType) {
   case FTYPEFAT:
     if (! *path) break;
     if (path[2]) StringsAdd(lbItems, "/..");
     scanFAT(path, ScanCPMfile);
     break;
#ifdef ORI_UZIX
   case FTYPEUZIX:
     if (*path)
       scanUZIX(path, ScanCPMfile);
     break;
#else
   case FTYPECPM:
     for (ii=0; ii<16; ii++) {
	ksprintf(buf15, "/U%d", ii);
	StringsAdd(lbItems, buf15);
     }
     scanCPM(path, ScanCPMfile); 
     break;
   case FTYPEORD:
     scanORD(path, ScanCPMfile, FIND_ENUM); 
     break;
#endif
   default: ;
  }
  if (lbItems->Count) lb1->ItemIndex=0;

DBGPRT("UP3 ");

  SetWindow(Panel[index], WP_CAPTION);

DBGPRT("UP4 ");

  ControlUpdate(lb1);

DBGPRT("UP5 ");

}

void cmdputch(char ch)
{
  promptxy();
  kputch(ch);
  CmdLine[CmdLinePos++]=ch;
  CmdLine[CmdLinePos]=0;
}

short SelDiskClick(lb)
  TLISTBOX* lb;
{
  register int index=lb->Control.Tag;
  char drv=*StringsItem(lb->Items, lb->ItemIndex);
  WindowClose(lb->Control.Window);
  switch(GetOSDrive(drv)) {
#ifdef ORI_UZIX
    case FTYPEUZIX:
      drv='/';
      break;
#else
    case FTYPECPM:
      bdos(CPMRDS);
      bdos(CPMLGIN, drv-'A');
      break;
#endif
    case FTYPEFAT:
#ifndef ORI_UZIX
      if (! TotalDrives) {
	MessageBox(NoIDEBDOS, StrError, 0);
	return 0;
      }
#endif
      UnMountFAT(index);
      if (! MountFAT(&drv, index)) {
        MessageBox(StrFMount, StrError, 0);
	return 0;
      }
      break;
    default: ;
  }
  DRV[index]=drv;
  UpdatePanel(index);
  return 0;
}

short WndKeyPressed(wnd, key)
  register TWINDOW* wnd;
  short	   key;
{
  lb1=(void*)wnd->FocusedControl;
  if (lb1->ItemIndex<0) return FALSE;
  switch (key) {
    case KEYCTRLT:
        ExecCmd(CMD_TYPE, wnd->Col); 
	break;
    case KEYCTRLP:
        ExecCmd(CMD_KTYPE, wnd->Col); 
	break;
    case KEYCTRLY:
        ExecCmd(CMD_ATYPE, wnd->Col); 
	break;
    case KEYCTRLR:
        wCol=wnd->Col+(PanelWidth/2-11);
	wRow=wnd->Row+3;
	if (wnd1=WindowOpen(wCol, wRow,
#ifdef ORI_UZIX
                            17, 13,
#else
	                    21, 20,
#endif
                            WA_DOUBLE | WA_SHADOW | WA_CANCEL)) {
	  wnd1->Caption=" Select disk ";
	  SetWindow(wnd1, WP_CAPTION);
#ifdef ORI_UZIX
          if (lb1=CreateListBox(wnd1, 2, 2, 12, 9, FALSE)) {
	    lb1->Columns=1;
	    lb1->Items->Count=9;
#else
          if (lb1=CreateListBox(wnd1, 1, 2, 18, 16, FALSE)) {
	    lb1->Columns=2;
	    if (AltairDos)
		lb1->Items->Count=30;
	    else
		lb1->Items->Count=24;
#endif
	    if (wnd->Col) lb1->Control.Tag=1;
	    lb1->OnClick=SelDiskClick;
	    lb1->Items->Data=&SelDiskStrings[0];
	    lb1->ItemIndex=0;
	    ControlUpdate(lb1);
	  }
	}
	break;
    case KEYBACKSPACE:		/* ^Q */
	if (CmdLinePos) {
	  CmdLinePos--;
	  cmdputch(' ');
	  CmdLine[--CmdLinePos]=0;
	  promptxy();
	}
	break;
    default:
	if ((key<32)||((key==32)&&(! *CmdLine))) return FALSE;		/* continue with default key processing */
	if (CmdLinePos<ScreenWidth-PromptLen-1)
	  cmdputch(key);
	break;
  }
  return TRUE;								/* do not default key processing */
}

short FileListClick(lb)
  TLISTBOX* lb;
{
  register char* Item;
  lb1=lb;
  if (*CmdLine) 
    ExecCmd(CMD_LINE, lb->Control.Tag);
  else {
    if (lb->ItemIndex<0) return FALSE;
    Item=StringsItem(lb->Items, lb->ItemIndex)+OBJSIZE+1;
    if (*Item=='/') 						/* chdir */
      PanelChdir(Item, lb->Control.Tag);
    else 							/* copy */
      ExecCmd(CMD_COPY, lb->Control.Tag); 
    
  }
  return 0;
}

void do_menu()
{
/* Initialization    */
  *CmdLine=0;
  memset(Path, 0, sizeof(Path));
  UID[0]=UID[1]=OS_getuid();
#ifdef ORI_UZIX
  ioctl(STDIN_FILENO, TTY_RAWCHAR);
  DRV[0]=DRV[1]='/';
  memset(ScreenBuf,20,ScreenWidth*ScreenHeight);
#else
  OsLevel(&sublvl);
  AltairDos=(sublvl>0xab30)&&(sublvl<0xabff);
  DRV[0]=DRV[1]=bdos(CPMIDRV)+0x41;
  ClearScreen();
#endif

DBGPRT("$> ");

  PrintKeyBar();

DBGPRT("#> ");

  for (wCol=0; wCol<PanelCount; wCol++) {

DBGPRT("@> ");

    if (Panel[wCol]=WindowOpen(wCol*PanelWidth, 0, PanelWidth,
				PanelHeight, WA_SINGLE | WA_NOSTORE)) {		/* Create window */
      Panel[wCol]->TabStop=FALSE;						/* Allow to move focus to next window */
      Panel[wCol]->OnKeyPressed=WndKeyPressed;

DBGPRT("A> ");

      if (FileList[wCol]=CreateListBox(Panel[wCol], 1, 2, 38, PanelHeight-3, FALSE)) {	/* Create control */

DBGPRT("B> ");

	FileList[wCol]->Control.Tag=wCol;
	FileList[wCol]->OnClick=FileListClick;
	FileList[wCol]->Control.CatchIdle=TRUE;					/* advanced repaint on IDLE ticks */
	FileList[wCol]->Columns=2;
	FileList[wCol]->Selected=0;						/* enable MultiSelect */

DBGPRT("C> ");

	UpdatePanel(wCol);

DBGPRT("D> ");

      }
    }
  }
  promptxy();

/**/

  ApplicationLoop();

/* Finalization      */
 
/*  WindowClose(Panel[0]);
    WindowClose(Panel[1]); */
  topwnd=NULL;
  ClearScreen();
#ifdef ORI_UZIX
  ioctl(STDIN_FILENO, TTY_COOKED);
#endif
}

