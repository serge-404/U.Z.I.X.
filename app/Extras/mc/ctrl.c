/*

Windows screen-related functions - Controls

(c) 2010 Serge

*/

#include	"stdlib.h"	/* malloc */
#include	"string.h"
#include	"screen.h"
#include	"windows.h"

#define NOSELECTDIR

extern	short	ScreenCol, ScreenRow, SCol, SRow;
extern	short	WndRow, WndCol, WndWidth, WndHeight;
extern	char*	ScreenBuf;			/* ASCII copy of current screen */
extern	TWINDOW	*topwnd;			/* currently operating window (top window) */
extern	int atr;
static	short	ii;

ushort Max(a, b)
  register ushort a;
  ushort b;
{
  return ((a>b) ? a : b);
}

ushort Min(a, b)
  register ushort a;
  ushort b;
{
  return ((a<b) ? a : b);
}

void sleep(ushort ms)
{
  for (ii=0; ii<ms; ii++);
}

extern void WStoreXY(TWINDOW* Window, TRECT* rect);
extern void WRestoreXY(TRECT* rect);
extern void BoxChar(int offs);

MRESULT MessageBox(LPSTR lpText, LPSTR lpCaption, ushort uType)
{
  register TWINDOW* wnd;
  short ii=strlen(lpText);
  if (wnd=WindowOpen(Max((ScreenWidth-ii-4)/2, 1)-1, 10, Min(ScreenWidth-3, ii+5), 7, WA_SHADOW | WA_SINGLE)) {
    if ((lpCaption)&&(*lpCaption)) {
      wnd->Caption=lpCaption;
      SetWindow(wnd, WP_CAPTION);
    }
    GotoXY(2,2); kprintf(lpText);
  }
  Inkey();				/* TODO: create & register buttons */
  WindowClose(wnd);
  return 0;				/* TODO: return ModalResult based on button pressed */
}

/* TSTRINGS */

void StringsClear(Str)
  register TSTRINGS* Str;
{
  if (Str) {
    Str->Last=Str->Data;
    Str->Count=0;
  }
}

TSTRINGS* CreateStrings(short Alloc)
{
  register TSTRINGS* Str;
  if (Str=malloc(sizeof(TSTRINGS))) {
    memset(Str, 0, sizeof(TSTRINGS));
    if ((Alloc>0)&&(Str->Last=Str->Data=malloc(Alloc)))
      Str->Allocated=Alloc;
  }
  return Str;
}

void* DestroyStrings(Str)
  register TSTRINGS* Str;
{
  if (Str) {
    if ((Str->Data) && (Str->Allocated))
      free(Str->Data);
    free(Str);
  }
  return NULL; 
}

BOOL StringsAdd(Strings, str)	/* String[i]=(OBJSIZE)Object(BOOL)Selected(LPSTR)Str*/
  register TSTRINGS* Strings;
  char* str;
{
  char* dest;
  int i, k;
  if ((! Strings)||(! str)) return FALSE;
  i=strlen(str);
  k=(short)((void*)Strings->Last)-(short)(void*)Strings->Data;
  if ((! Strings->Allocated) && (! Strings->Data))
    if (Strings->Last=Strings->Data=malloc(ALLOCSIZE))
      Strings->Allocated=ALLOCSIZE;
  if (i+OBJSIZE+2>(Strings->Allocated-k)) {
    if (! (Strings->Data=realloc(Strings->Data, Strings->Allocated=Max(Strings->Allocated+i+OBJSIZE+2, Strings->Allocated+ALLOCSIZE))))
      k=Strings->Count=Strings->Allocated=0; 
    Strings->Last=Strings->Data+k;
  }
  if (! Strings->Last) return FALSE;
  k=OBJSIZE+1; 			
  dest=Strings->Last;			 
  while (k--) *dest++=0; 		/* reserve & zeroing `Selected` and `Object` fields */  
  while (*str) *dest++=*str++;		/* strcpy(Strings->Last, str); */
  *dest++=0;				/* terminating 0 */
  Strings->Last=dest;
  Strings->Count++;
  return (Strings->Allocated!=0);
}

char* StringsItem(TSTRINGS* Strings, short Item)
{
  register char* result=NULL;
  if ((! Strings)||(Item<0)) return NULL;
  result=Strings->Data;
  while (Item--) {
    result+=OBJSIZE+1;			/* skip Selected and Object fields */
    while (*result) result++;			
    result++;				/* skip string terminator (goto next element) */
  }
  return result;
}

/* Action={0=Set FALSE (not selected), 1=Set TRUE (selected), else get selected info }
*/

BOOL StringsItemSelected(Item, Action)
  register char* Item;
  BOOL Action;
{
  Item+=OBJSIZE;
  if (Action==GET_SELECTED) return *Item != 0;
  if (Item) *Item=Action;
  return (Item!=NULL);
}

/* Common control procedures */

void ControlUpdate(void* ctrl)
{
  SendMessage(ctrl, WE_PAINT, 0);
}

TCONTROL* ControlRegister(TWINDOW* Window, void* control, BOOL DefaultDraw) 
{
  register TCONTROL* ctrl=NULL;
  TCONTROL* nextctrl;
  if ((! Window)||(! control)) return NULL;
  nextctrl=Window->FirstControl;
  while (nextctrl) {
    ctrl=nextctrl;
    nextctrl=nextctrl->NextControl;
  }
  ((TCONTROL*)control)->NextControl=NULL;
  ((TCONTROL*)control)->PrevControl=ctrl;
  ((TCONTROL*)control)->Window=Window;
  if (DefaultDraw) ControlUpdate(control);
  if (ctrl)
    ctrl->NextControl=control;
  else {
    Window->FirstControl=Window->FocusedControl=control;	/* first registered = focused */
    SendMessage(control, WE_SETFOCUS, 0);
  }
  return control;
}

/* TBUTTON */

void BtnDraw(ctrl, Capt)
  register TCONTROL* ctrl;
  char* Capt;
{ 
  WBox(ctrl->Col, ctrl->Row, ctrl->Width, 3, WA_SINGLE);
  GotoXY(ctrl->Col+1, ctrl->Row+1);
  PrintAligned(Capt, ctrl->Width-2, AL_CENTER, ' ');
}

short CtrlPreEventKeyPressed(obj,param)
	register TCONTROL* obj;
	short param;
{
  if (obj->OnKeyPressed) return (obj->OnKeyPressed)(obj,param);
  return FALSE;
}

short ButtonProc(void* obj, short msg, short Param)
{
  short result=0;
  TRECT crect;
  TBUTTON* btn=obj;
  register TCONTROL* ctrl=obj; 
  if ((! btn)||(! ctrl->Window)) return 0;
  WStoreXY(ctrl->Window, &crect);
  switch (msg) {
    case WE_PAINT: BtnDraw(ctrl, btn->Caption);
		   break;
    case WE_KEYPRESSED:
	if (CtrlPreEventKeyPressed(ctrl,Param)) break;
	if ((Param==KEYCR)||(Param==KEYSPACE)) {
	  WBox(ctrl->Col, ctrl->Row, ctrl->Width, 3, WA_DOUBLE | WA_TRANSP);
	  sleep(1000);
	  BtnDraw(ctrl, btn->Caption);
	  if (btn->OnClick)
            result=(btn->OnClick)(obj);
	}
	else if ((Param==KEYRIGHT)||(Param==KEYDOWN)) {
          SendMessage(ctrl->Window, WE_MOVEFOCUS, TAB);
	  break;
        } 
    case WE_SETFOCUS:
	GotoXY(ctrl->Col, ctrl->Row+1); kputch(LFOCUS);
	GotoXY(ctrl->Col+ctrl->Width-1, ctrl->Row+1); kputch(RFOCUS);
	break;
    case WE_LOSTFOCUS:
        atr=WA_SINGLE;
	GotoXY(ctrl->Col, ctrl->Row+1); BoxChar(LLINE);
	GotoXY(ctrl->Col+ctrl->Width-1, ctrl->Row+1); BoxChar(RLINE);
	break;
    default: ;
  }
  WRestoreXY(&crect);
  return result;
}

TBUTTON* CreateButton(TWINDOW* wnd, short col, short row, short width, char* Caption)
{
  TBUTTON* btn=malloc(sizeof(TBUTTON));
  register TCONTROL* ctrl=(void*)btn;
  if (btn) {
    memset(btn, 0, sizeof(TBUTTON));
    ctrl->OnMessage=ButtonProc;
    ctrl->CtrlType=CBUTTON;		
    ctrl->Col=col;
    ctrl->Row=row;
    ctrl->Width=width;
    btn->Caption=Caption;		/* not strcpy, so Caption must never be freed */
    ControlRegister(wnd, btn, TRUE);	/* Register control on window */
  }
  return btn;
}

/* TLISTBOX */

void ListBoxDrawItem(TLISTBOX* lb, short ItemN)
{
  TRECT rect;
  char* str;
  BOOL focused;
  register TCONTROL* ctrl=(void*)lb; 
  short itemCol=ctrl->Col;
  short itemRow=ctrl->Row;
  short itemWidth=ctrl->Width/lb->Columns;
  short ii=ItemN - lb->TopIndex;
  if ((ItemN<0)||(! ctrl->Window)) return;
  while (ii>=ctrl->Height) {
    itemCol+=itemWidth;
    ii-=ctrl->Height;
  }
  GotoXY(itemCol, itemRow+ii);
  if (ItemN<lb->Items->Count) {
/* PreEvent */
    rect.Col=itemCol;
    rect.Row=itemRow;
    rect.Width=itemWidth;
    rect.Height=1;
    if ((lb->OnDrawItem) && ((lb->OnDrawItem)(lb, ItemN, &rect))) return;
/**/
    str=StringsItem(lb->Items, ItemN)+OBJSIZE;	/* Skip Object field */
    focused=(((TWINDOW*)(lb->Control.Window))->FocusedControl==(TCONTROL*)lb)&&(ItemN==lb->ItemIndex);
    if (focused) 
      kputch(LFOCUS);
    else {
      kputch(' ');
    }
    if (*str) *str=SELECTED; else str++;	/* check for 'selected' attribute */
    PrintAligned(str, itemWidth-1, lb->ItemAlign, ' ');
    if (focused)
      kputch(RFOCUS); 
    else {
      kputch(' ');
    }
  }
  else while (itemWidth--) kputch(' ');
}

void ListBoxDraw(lb)
  register TLISTBOX* lb;
{
  short clmns=lb->Columns;
  short itemNum=lb->TopIndex;
  short height;
  while (clmns--) {
    height=lb->Control.Height;
    while (height--)
      ListBoxDrawItem(lb, itemNum++);
  }
}

short ListBoxProc(void* obj, short msg, short Param)
{
  short result=0, ItemsOnScreen, ItemsCount, DeltaItem, PrevItemIndex, ItemsMax;
  char* Item;
  TRECT crect;
  register TLISTBOX* lb=obj;
  TCONTROL* ctrl=obj; 
  if (! lb) return 0;
  if (msg==WE_IDLE) {
    if (lb->DrawCnt) { 
      WStoreXY(ctrl->Window, &crect);
      lb->DrawCnt--;
      ListBoxDrawItem(lb, lb->DrawItemNum++);
      WRestoreXY(&crect);
    }
  }
  else {
   WStoreXY(ctrl->Window, &crect);
   switch (msg) {
    case WE_PAINT: ListBoxDraw(lb);
		   break;
    case WE_KEYPRESSED:
	if (CtrlPreEventKeyPressed(ctrl,Param)) break;
	ItemsCount=lb->Items->Count;
	DeltaItem=ctrl->Height;
	PrevItemIndex=lb->ItemIndex;
	switch (Param) {
	  case KEYCR: if (lb->OnClick) result=(lb->OnClick)(obj);
		      break;
	  case KEYSPACE:	/* select item */
	  case KEYINSERT:
		Item=StringsItem(lb->Items, lb->ItemIndex);
		if ((lb->Selected<0)||(lb->ItemIndex<0)
#ifdef NOSELECTDIR
		    ||(*(Item+OBJSIZE+1)=='/')
#endif
		   ) break;
        	StringsItemSelected(Item, (ItemsMax=StringsItemSelected(Item, GET_SELECTED))==FALSE);
		if (ItemsMax) lb->Selected--; else lb->Selected++;
		ListBoxDrawItem(lb, lb->ItemIndex);
		Param=KEYDOWN;
	  case KEYUP:		/* move pointer for 1 item - Up */
	  case KEYDOWN:		/* move pointer for 1 item - Down */
		DeltaItem=1;
	  case KEYLEFT1:
	  case KEYLEFT2:	/* move pointer for N items left = PageUp */
	  case KEYRIGHT:	/* move pointer for N items right = PageDn */
		ItemsMax=ctrl->Height * lb->Columns;
		if ((Param==KEYRIGHT)||(Param==KEYDOWN)) {	
		  ItemsOnScreen=Min(ItemsMax, ItemsCount - lb->TopIndex);  
		  if ((lb->ItemIndex+=DeltaItem)>=ItemsCount) lb->ItemIndex=ItemsCount-1;
		  if (lb->ItemIndex-ItemsOnScreen>=lb->TopIndex) {
		    if ((lb->TopIndex+=DeltaItem)>=ItemsCount) lb->TopIndex=ItemsCount-ItemsOnScreen-1;
		    lb->DrawCnt=ItemsMax;
		    lb->DrawItemNum=lb->TopIndex;
		  }
		}
		else {
		  if ((lb->ItemIndex-=DeltaItem)<0) lb->ItemIndex=0;
		  if (lb->ItemIndex<lb->TopIndex) {
		    if ((lb->TopIndex-=DeltaItem)<0) lb->TopIndex=0;
		    lb->DrawCnt=ItemsMax;
		    lb->DrawItemNum=lb->TopIndex;
		  }
		}
		if ((lb->DrawCnt)&&(! ctrl->CatchIdle)) ListBoxDraw(lb);
                else {
		  if ((PrevItemIndex>=lb->TopIndex)&&(PrevItemIndex<lb->TopIndex+ItemsMax))
                    ListBoxDrawItem(lb, PrevItemIndex);   /* redraw previously focused */
		  ListBoxDrawItem(lb, lb->ItemIndex);
		}
		break;
	  default: ;
	}
    case WE_SETFOCUS:
	ListBoxDrawItem(lb, lb->ItemIndex);
	break;
    case WE_LOSTFOCUS:
	PrevItemIndex=lb->ItemIndex;
	lb->ItemIndex=-1;
	ListBoxDrawItem(lb, PrevItemIndex);
	lb->ItemIndex=PrevItemIndex;
	break;
    case WE_DESTROY:
	lb->Items=DestroyStrings(lb->Items);
	break;
    default: ;
   }
   WRestoreXY(&crect);
  }
  return result;
}

void ListBoxClear(lb, redraw)
  register TLISTBOX* lb;
  BOOL redraw;
{
  lb->ItemIndex=-1;
  lb->TopIndex=0;
  if (lb->Selected>0) lb->Selected=0;
  StringsClear(lb->Items);
  if (redraw) ControlUpdate(lb);
}

TLISTBOX* CreateListBox(TWINDOW* wnd, short col, short row, short width, short height, BOOL DefaultDraw)
{
  register TLISTBOX* lb=malloc(sizeof(TLISTBOX));		
  TCONTROL* ctrl=(void*)lb;
  if (lb) {
    memset(lb, 0, sizeof(TLISTBOX));
    ctrl->OnMessage=ListBoxProc;
    ctrl->CtrlType=CLISTBOX;		
    ctrl->Col=col;
    ctrl->Row=row;
    ctrl->Width=width;
    ctrl->Height=height;
    lb->Columns=1;
    lb->Selected=lb->ItemIndex=-1;
    lb->Items=CreateStrings(0);
    ControlRegister(wnd, lb, DefaultDraw);		
  }
  return lb;
}


