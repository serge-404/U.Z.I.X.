#include "screen.h"
#include "stdlib.h"
#include "string.h"
#include "stringz.h"
#include "windows.h"
#include "filemgr.h" 

#define __MENU

#define CODE_KOI8	0
#define CODE_ALT	1
#define CODE_KOI7	2

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

extern	short	wRow, wCol;
extern	void 	__wgotoxy(uchar x, uchar y);
extern	void cmdexec(BOOL ClrScr);
extern	char* PanelPath(char* buf, ushort index);
extern	void PanelChdir(char* dir, short PIndex);
extern	BYTE GetOSDrive(char drive);
extern BOOL MountFAT(char* path, int Index);
extern void UnMountFAT(int Index);
extern void PrintKeyBar();
extern void promptxy();

extern void ExecCmd(uchar cmd, ushort pnl);
extern void FullScreen();
extern void WndScreen();

extern char	CmdLine[150];
extern void*	topwnd;			/* currently operating window (top window) */

extern char StrFMount[];

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

TWINDOW* Panel[PanelCount], *wnd1;
TLISTBOX* FileList[PanelCount], *lb1;
char	Path[PanelCount][MAX_PATH];
ushort	UID[PanelCount];
uchar	DRV[PanelCount];
short	CmdLinePos=0;
char	buf15[15];
ushort	sublvl;

TSTRINGS* lbItems;

char StrError[]=" Error ";

char SelDiskStrings[]=
  "A \x00" "UZIX /\x00"
  "0 \x00" "FAT 0:\x00"
  "1 \x00" "FAT 1:\x00"
  "2 \x00" "FAT 2:\x00"
  "3 \x00" "FAT 3:\x00"
  "4 \x00" "FAT 4:\x00"
  "5 \x00" "FAT 5:\x00"
  "6 \x00" "FAT 6:\x00"
  "7 \x00" "FAT 7:\x00";

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
  register int ii;
  BYTE OsType;
  char* path=&(Path[index][0]);
  lb1=FileList[index];
  PanelPath(path, index);
  OsType=GetOSType(path);
  Panel[index]->Caption=path;
  ListBoxClear(lb1, FALSE);
  lbItems=lb1->Items;
  switch (OsType) {
   case FTYPECPM:
     for (ii=0; ii<16; ii++) {
	ksprintf(buf15, "/U%d", ii);
	StringsAdd(lbItems, buf15);
     }
     scanCPM(path, ScanCPMfile); 
     break;
   case FTYPEFAT:
     if (! *path) break;
     if (path[2]) StringsAdd(lbItems, "/..");
     scanFAT(path, ScanCPMfile); 
     break;
   case FTYPEORD:
     scanORD(path, ScanCPMfile, FIND_ENUM); 
     break;
   default: ;
  }
  if (lbItems->Count) lb1->ItemIndex=0;
  SetWindow(Panel[index], WP_CAPTION);
  ControlUpdate(lb1);
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
    case FTYPECPM:
      bdos(CPMRDS);
      bdos(CPMLGIN, drv-'A');
      break;
    case FTYPEFAT:
      if (! TotalDrives) {
	MessageBox(NoIDEBDOS, StrError, 0);
	return 0;
      }
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
	if (wnd1=WindowOpen(wCol, wRow, 21, 20, WA_DOUBLE | WA_SHADOW | WA_CANCEL)) {
	  wnd1->Caption=" Select disk ";
	  SetWindow(wnd1, WP_CAPTION);
          if (lb1=CreateListBox(wnd1, 1, 2, 18, 16, FALSE)) {	
	    if (wnd->Col) lb1->Control.Tag=1;
	    lb1->OnClick=SelDiskClick;
	    lb1->Columns=2;
	    lb1->Items->Data=&SelDiskStrings[0];
	    if (AltairDos)
		lb1->Items->Count=30;
	    else
		lb1->Items->Count=24;
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

int main()
{
/* Initialization    */
  *CmdLine=0;
  ClearScreen();
  PrintKeyBar();

  UID[0]=UID[1]=OS_getuid();   
  DRV[0]=DRV[1]=bdos(CPMIDRV)+0x41;

  for (wCol=0; wCol<PanelCount; wCol++) {
    if (Panel[wCol]=WindowOpen(wCol*PanelWidth, 0, PanelWidth,
				PanelHeight, WA_SINGLE | WA_NOSTORE)) {		/* Create window */
      Panel[wCol]->TabStop=FALSE;						/* Allow to move focus to next window */
      Panel[wCol]->OnKeyPressed=WndKeyPressed;
      if (FileList[wCol]=CreateListBox(Panel[wCol], 1, 2, 38, PanelHeight-3, FALSE)) {	/* Create control */
	FileList[wCol]->Control.Tag=wCol;
	FileList[wCol]->OnClick=FileListClick;
	FileList[wCol]->Control.CatchIdle=TRUE;					/* advanced repaint on IDLE ticks */
	FileList[wCol]->Columns=2;
	FileList[wCol]->Selected=0;						/* enable MultiSelect */
	UpdatePanel(wCol);
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
  return 0;
}

