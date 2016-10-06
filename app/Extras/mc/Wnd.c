/*

Windows screen-related functions
(c) 2010 Serge

*/

#include	"stdlib.h"	/* malloc */
#include	"string.h"
#include	"screen.h"
#include	"windows.h"

extern	short	LineWrap, ScreenCol, ScreenRow, SCol, SRow;
extern	short	WndRow, WndCol, WndWidth, WndHeight;
extern	char	ScreenBuf[ScreenSize];		/* ASCII copy of current screen */
extern	TWINDOW	*topwnd;			/* currently operating window (top window) */
static	short	WSCol, WSRow, ii, jj;

static	char	borders[]="        ++++-!!-####=$$=    #   ";

void WStoreXY(Window, rect)
  TWINDOW* Window;
  register TRECT* rect;
{
  rect->Row=WndRow;
  rect->Col=WndCol;
  rect->Width=WndWidth;
  rect->Height=WndHeight;
  WndCol=Window->Col;
  WndRow=Window->Row;
  WndWidth=Window->Width;
  WndHeight=Window->Height;
  WSCol=ScreenCol; WSRow=ScreenRow;
}

void WRestoreXY(rect)
  register TRECT* rect;
{
  WndRow=rect->Row;
  WndCol=rect->Col;
  WndWidth=rect->Width;
  WndHeight=rect->Height;
  ScreenCol=WSCol; ScreenRow=WSRow;
  __wgotoxy(ScreenCol, ScreenRow);
}

TWINDOW* WindowExists(TWINDOW* www)
{
  register TWINDOW* Wnd;
  for (Wnd=topwnd; (Wnd)&&(Wnd!=www); Wnd=Wnd->PrevWindow); 
  return Wnd;
}

TWINDOW* ActiveWindow()
{
  register TWINDOW* Wnd;
  for (Wnd=topwnd; (Wnd)&&(! Wnd->Active); Wnd=Wnd->PrevWindow); 
  return Wnd;
}

/* Close window & restore underwindow screen area                           */
/* Input: TWINDOW* to close or NULL to close top window                     */
/* Return: next topwindow (last in stack) or NULL if all windows are closed */

TWINDOW* WClose(TWINDOW* Window)
{
  int kk;
  register char *ptr;
  char* SBuf;
  TWINDOW* Wnd;
  if (! Window) Window=topwnd;
  if (! Window) return NULL;
  if (ptr=Window->wndBUF)
    for (ii=Window->Row; ii<Window->Row+Window->Height; ii++) {
      __wgotoxy(Window->Col, ii); 
      kk=Window->Col+Window->Width;
      SBuf=&ScreenBuf[ii*ScreenWidth];
      for (jj=Window->Col; jj<kk; jj++, ptr++) {
        SBuf[jj]=*ptr;
        bios(NCONOUT, *ptr);
      }
    }
  ScreenCol=Window->ScrCol;
  ScreenRow=Window->ScrRow;
  __wgotoxy(ScreenCol, ScreenRow);
  if (ptr) free(Window->wndBUF);
  if (Window==topwnd)
    topwnd=Window->PrevWindow;
  else {
    for (Wnd=topwnd; (Wnd)&&(Wnd->PrevWindow!=Window); Wnd=Wnd->PrevWindow); 
    if (Wnd) Wnd->PrevWindow=Window->PrevWindow;
  }
  if (Wnd=Window->PrevWindow) {
    WndRow=Wnd->Row;
    WndCol=Wnd->Col; 
    WndWidth=Wnd->Width;
    WndHeight=Wnd->Height;
  }
  else {
    WndRow=WndCol=0; WndWidth=ScreenWidth; WndHeight=ScreenHeight;
  }
  free(Window);
  return topwnd;
}

/* User Interface functions */

void PrintAligned(char* st, short Width, uchar Align, char pad)
{
  int cb=0, ii;				/* AL_LEFT */
  register int jj;
  if (! st) ii=0; else ii=strlen(st);
  if (ii>Width) ii=Width; 
  switch (Align) {
    case AL_CENTER:
	cb=(Width-ii)/2;
	break;
    case AL_RIGHT:
	cb=Width-ii;
	break;
  }
  for (jj=0; jj<Width; jj++) {
    if ((jj<cb)||(jj>cb+ii)) {
      if (pad) kputch(pad); 
      else { kputch(E_ESCAPE); kputch(E_CUR_RIGHT); }	/* just move cursor right */
    }
    else kputch(*st++);
  }
}

int atr;

void BoxChar(int offs)
{
  kputch(borders[atr*8 + offs]);
}

/* Paint Box with passed x,y,dx,dy (x,y,dx,dy>=0) or
             with topwnd x,y,dx,dy (x<0)          or  
   Paint top row only with x,y,dx  (dy<=0)             
*/
void WBox(short col, ushort row, ushort width, short height, ushort attr)
{
  short LW=LineWrap;
  register short dd;
  if (col<0) {
    if (! topwnd) return;
    col=row=0;
    width=topwnd->Width;
    height=topwnd->Height;
  }
  if (attr & WA_SHADOW) dd=2; else dd=1;
  StoreXY();
  GotoXY(col, row);		
  atr=attr & WA_BORDER;
  LineWrap=0;
  BoxChar(LUCORNER);
  for (ii=1; ii<width-dd; ii++)		
    BoxChar(ULINE);
  BoxChar(RUCORNER);
  if (height>0) {
    for (ii=row+1; ii<row+height-dd; ii++) {
      GotoXY(col, ii); BoxChar(LLINE);	
      if (! (attr & WA_TRANSP))
        for (jj=1; jj<width-dd; jj++) kputch(' ');
      else
        GotoXY(col+width-dd, ii);
      BoxChar(RLINE);
      if (attr & WA_SHADOW) kputch(RSHADOW);	
    }
    GotoXY(col, row+height-dd);	
    BoxChar(LDCORNER);
    for (ii=1; ii<width-dd; ii++)
      BoxChar(DLINE);
    BoxChar(RDCORNER);
    if (attr & WA_SHADOW) {
      kputch(RSHADOW);	
      GotoXY(col+1, row+height-1);	
      for (ii=1; ii<width; ii++) kputch(DSHADOW);
    }
  }
  LineWrap=LW;
  RestoreXY();
}

short SendMessage(obj, msg, param)
  register void* obj;
  short msg; 
  short param;
{
  if ((obj) && (((TWINDOW*)obj)->OnMessage))
    return (((TWINDOW*)obj)->OnMessage)(obj,msg,param);
  return WE_NULL;
}

uchar ButtonMode(ctrl)
  register TCONTROL* ctrl;
{
  if ((! ctrl)||(ctrl->CtrlType!=CBUTTON)) return 0;
  return ((TBUTTON*)ctrl)->Mode;
}

short WndProc(Wnd, msg, Param)
  TWINDOW* Wnd;
  short msg;
  short Param;
{
  short result=0;
  TRECT WRect;
  TWINDOW* NextWnd;
  register TCONTROL *ctrl=Wnd->FocusedControl;
  TCONTROL *ctl=Wnd->FirstControl;
  switch (msg) {
    case WE_IDLE: while (ctl) {
		    if (ctl->CatchIdle)
			result=SendMessage(ctl, WE_IDLE, 0);
		    ctl=ctl->NextControl; 
		  }
		  break;
    case WE_PAINT: WStoreXY(Wnd, &WRect);
		   WBox(0, 0, Wnd->Width, Wnd->Height, Wnd->Attr);
		   while ((ctl)&&(! Param)) {
			SendMessage(ctl, WE_PAINT, 0);
			ctl=ctl->NextControl; 
		   }
		   WRestoreXY(&WRect);
		   break;
    case WE_ACTIVATE: Wnd->Active=TRUE;
 		      SetWindow(Wnd, WP_CAPTION);
		      SendMessage(ctrl, WE_SETFOCUS, 0);
	              break;
    case WE_DEACTIVATE: Wnd->Active=FALSE;
		        SetWindow(Wnd, WP_CAPTION);
		        SendMessage(ctrl, WE_LOSTFOCUS, 0);
	                break;
    case WE_MOVEFOCUS: if ((ctrl)&&(ctrl->NextControl)) {
		    	 SendMessage(ctrl, WE_LOSTFOCUS, Param);
			 Wnd->FocusedControl=ctrl->NextControl;
			 result=SendMessage(ctrl->NextControl, WE_SETFOCUS, Param);
                       }
		       else if (! Wnd->TabStop) {
			 if (! (NextWnd=Wnd->PrevWindow)) NextWnd=topwnd;
			 if (NextWnd==Wnd) Wnd->FocusedControl=ctl;  /* Wnd->FirstControl; */
			 else {
				SendMessage(Wnd, WE_DEACTIVATE, 0);
				NextWnd->FocusedControl=NextWnd->FirstControl;
				result=SendMessage(NextWnd, WE_ACTIVATE, 0);
			 }	
		       }
		       break;
    case WE_KEYPRESSED:
	if ((Wnd->OnKeyPressed)&&((Wnd->OnKeyPressed)(Wnd, Param))) break;
	if (Param==KEYESCAPE) {
	  for (ctrl=ctl;  /* Wnd->FirstControl; */
               (ctrl)&&(ctrl->NextControl)&&(ButtonMode(ctrl)!=Param); 
               ctrl=ctrl->NextControl); 
	  if ((ctrl)&&(ButtonMode(ctrl)==Param))
	    Param=KEYSPACE;
	  else
	    if (Wnd->Attr & WA_CANCEL) {
		ctrl=NULL;
		WindowClose(Wnd);
	    }					
	}
	if (ctrl) result=SendMessage(ctrl, WE_KEYPRESSED, Param);
	break;
    default: ;
  }
  return result;
}

/*
   Attr= [ D0..D1 = { 0=no border, 1=single border,  2=double border, 3=reserved }
           D2     = { 0=not transparent (clear wnd), 1=transparent (no cls)      }
*/

TWINDOW* WindowOpen(ushort col, ushort row, ushort width, ushort height, ushort attr)
{
  register char* ptr;
  TWINDOW *Window, *Wnd;
  if (Window=malloc(sizeof(TWINDOW))) {
    memset(Window, 0, sizeof(TWINDOW));
    if (Wnd=ActiveWindow()) {
      Window->PrevFocused=Wnd;
      SendMessage(Wnd, WE_DEACTIVATE, 0);
    }
    Window->PrevWindow=topwnd;
    topwnd=Window;
    topwnd->Col=ScreenCol=WndCol=col; 
    topwnd->Row=ScreenRow=WndRow=row;
    topwnd->Width=WndWidth=width;
    topwnd->Height=WndHeight=height;
    topwnd->ScrCol=ScreenCol;
    topwnd->ScrRow=ScreenRow;
    topwnd->TabStop=TRUE;		/* Modal window */
    topwnd->Attr=attr;
    topwnd->OnMessage=WndProc;
    SendMessage(topwnd, WE_CREATE, 0);
    if (! (attr & WA_NOSTORE)) {
      topwnd->wndBUF=ptr=malloc(width*height);
      for (ii=0; (ptr) && (ii<height); ii++)
        for (jj=col; jj<width+col; jj++, ptr++)
          *ptr=ScreenBuf[(ii+row)*ScreenWidth+jj];
    }
    __wgotoxy(col, row);
    SendMessage(topwnd, WE_PAINT, 0);
    SendMessage(topwnd, WE_ACTIVATE, 0);
  }
  return topwnd;
}

/* refreshes window after TWINDOW (property) changed */

short SetWindow(Window, property)   
  register TWINDOW* Window;
  int property;
{
  TRECT wrect;
  if (! Window) return FALSE;
  atr=Window->Attr & WA_BORDER;
  WStoreXY(Window, &wrect);
  switch (property) {
    case WP_ACTIVE: if (Window->Active)
			SendMessage(Window, WE_ACTIVATE, 0);
                    else
			SendMessage(Window, WE_DEACTIVATE, 0);
		    break;
    case WP_ATTR: WBox(Window->Col, Window->Row, Window->Width, Window->Height, WA_TRANSP | atr);
    case WP_CAPTION: GotoXY(1,0); 
                     if (Window->Caption) 
                       PrintAligned(Window->Caption, Window->Width-1, AL_CENTER,
                                    (Window->Active) ? borders[24+ULINE] : borders[atr*8 + ULINE]);
                     break;
    default: ;
  }
  WRestoreXY(&wrect);
  return TRUE;
}

/* Closes specified window or top-window if window=NULL, all controls are destroyed */

TWINDOW* WindowClose(TWINDOW* Window)
{
  TWINDOW* Wnd;
  register TCONTROL* nextctrl;
  if (! Window) Window=topwnd;
  if (! Window) return NULL;
  SendMessage(Window, WE_DEACTIVATE, 0);
  if (! (Wnd=WindowExists(Window->PrevFocused))) Wnd=Window->PrevWindow;
  SendMessage(Window, WE_CLOSE, 0);
  while (Window->FirstControl) {
    nextctrl=(Window->FirstControl)->NextControl;
    SendMessage(Window->FirstControl, WE_DESTROY, 0);
    (Window->FirstControl)->Window=NULL;
    free(Window->FirstControl);				/* Free controls */
    Window->FirstControl=nextctrl;
  }
  SendMessage(Window, WE_DESTROY, 0);
  WClose(Window);
  WRestoreXY((TRECT*)(&(Wnd->Col)));
  SendMessage(Wnd, WE_ACTIVATE, 0);
  return NULL;
}

BOOL GetMessage(msg, param)
  short* msg; 
  register short* param;
{
  *msg=WE_IDLE;
  *param=0;
  if (! kbhit()) return TRUE;
  else {
    switch (*param=Inkey()) {
      case TAB:	*msg=WE_MOVEFOCUS;
		break;
#ifdef __CTRLCBREAK
      case KEYCTRLC: return FALSE;			/* stop ApplicationLoop */
#endif __CTRLCBREAK
      default:	*msg=WE_KEYPRESSED;
		if (*param==0x7f) *param=KEYBACKSPACE;	/* for crazy Orion`s BS code */
		break;
    } /*switch*/
  }
  return TRUE;
}

void ApplicationLoop()
{
  short Msg;
  short Param;
  while (GetMessage(&Msg, &Param)) {
    SendMessage(ActiveWindow(), Msg, Param);    /* DispatchMessage(&Msg); */
  }
}


