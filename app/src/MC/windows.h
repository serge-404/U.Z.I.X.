/*

Windows management functions
(c) 2010 Serge

*/

#ifndef _WINDOWS

#include	<hitech.h>

/* #define DBG */

#ifdef DBG
#define DBGPRT(st) bputs(st);kgetch();
#else
#define DBGPRT(st) ;
#endif

/* conditional compile defines */
#define	__CTRLCBREAK		/* if defined, windows driver interrupted when CTRL+C pressed        */

#ifndef	NULL
#define	NULL	((void *)0)
#endif	NULL
 
#define FALSE	0
#define TRUE	1

/* Window definitions */

#define E_ESCAPE	0x1b
#define E_CUR_RIGHT	0x43
#define TAB		0x09

#define KEYCTRLC	0x03
#define KEYCR		0x0d
#define KEYINSERT	0x1a	/* ^Z */
#define KEYESCAPE	0x1b
#define KEYSPACE	0x20

/* "pseudographic" chars */

#define RSHADOW		'@'	
#define DSHADOW		'@'
#define LFOCUS		'>'
#define RFOCUS		'<'
#define SELECTED	'*'

/* system dependend - screen and keyboard */

#ifdef ORDOS

#define KEYBACKSPACE	0x7f
#define KEYLEFT1	0x08
#define KEYLEFT2	0x08
#define KEYRIGHT	0x18
#define KEYUP		0x19
#define KEYDOWN		0x1a

#else   /* Orion CPM */

#define KEYBACKSPACE	0x11	/* ^Q */
#define KEYLEFT1	0x08	
#define KEYLEFT2	0x13	/* ^S for CPM.EXE */
#define KEYRIGHT	0x04	/* ^D for CPM.EXE */
#define KEYUP		0x05	/* ^E for CPM.EXE */
#define KEYDOWN		0x18	/* ^X for CPM.EXE */

#endif

/* Window & control messages */

#define WE_NULL		0
#define WE_IDLE		0
#define WE_CREATE	1
#define WE_PAINT	2
#define WE_ACTIVATE	3
#define WE_DEACTIVATE	4
#define WE_KEYPRESSED	5
#define WE_CLOSE	6
#define WE_DESTROY	7
#define WE_SETFOCUS	8
#define WE_LOSTFOCUS	9
#define WE_MOVEFOCUS	10

/* Window properties */

#define	WP_ACTIVE	1
#define WP_CAPTION	2
#define WP_TABSTOP	3
#define	WP_ATTR		4

/* Window attributes - WP_ATTR bitwise */

#define	WA_NOBORDER	0
#define	WA_SINGLE	1
#define	WA_DOUBLE	2
#define	WA_BORDER	3
#define	WA_TRANSP	4
#define WA_SHADOW	8
#define WA_NOSTORE	16	/* do not store underwindow screen area */
#define WA_CANCEL	32	/* close window with ESC key */

#define	AL_LEFT		0
#define	AL_CENTER	1
#define	AL_RIGHT	2

/* Box chars */

#define LUCORNER	0
#define RUCORNER	1 
#define LDCORNER	2
#define RDCORNER	3
#define ULINE		4
#define LLINE		5
#define RLINE		6
#define DLINE		7

/* Controls (CtrlType) */

#define CBUTTON		1
#define CLISTBOX	2

/* ModalResults, ModalButtons

#define	MB_OK			0
#define	MB_OKCANCEL 		1
#define	MB_ABORTRETRYIGNORE	2
#define	MB_YESNOCANCEL		3
#define	MB_YESNO		4
#define	MB_RETRYCANCEL		5

#define	IDOK			1
#define	ID_OK 			IDOK
#define	MR_OK			IDOK
#define	IDCANCEL		2
#define	ID_CANCEL		IDCANCEL
#define	MR_CANCEL		IDCANCEL
#define	IDABORT			3
#define	ID_ABORT		IDABORT
#define	MR_ABORT		IDABORT
#define	IDRETRY			4
#define	ID_RETRY		IDRETRY
#define	MR_RETRY		IDRETRY
#define	IDIGNORE		5
#define	ID_IGNORE		IDIGNORE
#define	MR_IGNORE		IDIGNORE
#define	IDYES			6
#define	ID_YES			IDYES
#define	MR_YES			IDYES
#define	IDNO			7
#define	ID_NO			IDNO
#define	MR_NO			IDNO

*/

typedef unsigned char	MRESULT;
typedef char*		LPSTR;

typedef struct {
  ushort	Col;		/* left-up corner X coord */
  ushort	Row;		/* left-up corner Y coord */
  ushort	Width;	
  ushort	Height;	
} TRECT;

typedef short	(*w_callback)(void* object, short msg, short Param);
typedef short	(*w_notify)(void* object);
typedef short	(*w_keypres)(void* object, short Key);
typedef BOOL	(*w_drawitm)(void* object, short Index, TRECT* Rect);
typedef int	(*w_itemcmp)(char* item1, char* item2);


typedef struct {
  w_callback	OnMessage;	/* allways first, do not move */
  ushort	Col;		/* left-up corner X coord */
  ushort	Row;		/* left-up corner Y coord */
  ushort	Width;	
  ushort	Height;	
  void*		Window;		/* TWINDOW* */
  short		CtrlType;		
  short		Tag;
  void*		PrevControl;	/* TCONTROL* */
  void*		NextControl;	/* TCONTROL* */
  BOOL		CatchIdle;
  w_keypres	OnKeyPressed;	/* Event firing when key pressed */
} TCONTROL;

typedef struct {
  w_callback	OnMessage;	/* allways first */
  ushort	Col;		/* left-up corner X coord */
  ushort	Row;		/* left-up corner Y coord */
  ushort	Width;	
  ushort	Height;	
  char*		wndBUF;		/* under-window content (for close) */
  ushort	Attr;		/* includes Border, BorderIcons bits */
  void*		PrevWindow;	/* TWINDOW* */
  void*		PrevFocused;	/* TWINDOW* */
  TCONTROL*	FirstControl;
  TCONTROL*	FocusedControl;
  ushort	ScrCol;		/* for storing ScreenCol */
  ushort	ScrRow;		/* for storing ScreenRow */
  BOOL		Active;
  char*		Caption;
  BOOL		TabStop;
  w_keypres	OnKeyPressed;	/* Event firing when key pressed */
} TWINDOW;

#define ALLOCSIZE	1024
#define OBJSIZE		2
#define GET_SELECTED	2

typedef	struct {
  short     Allocated;		/* actually curerently allocated Items with malloc/realloc */
  short	    Count;
  char*     Last;		/* pointer to next new element (FreeSpace) */
  char*     Data;		/* short_tàg,nul-term-String,..,short_tàg,nul-term-String */
  w_itemcmp ItemCmp;		/* subroutine for compare Items while sort. No sorting if NULL */
} TSTRINGS;

typedef struct {
  TCONTROL  Control;		/* allways first */
  uchar	    Mode;		/* 0=MR_NONE(no shortcut), CR=MR_OK(Enter), E_ESCAPE=MR_CANCEL(Esc) */
  char*	    Caption;		/* just pointer, not array! */
  w_notify  OnClick;
} TBUTTON;

typedef struct {
  TCONTROL  Control;		/* allways first */
  short	    Columns;		/* display items in several columns */
/* private properties - do not change from the outside */
  short     DrawCnt;		
  short     DrawItemNum;
/* Public properties */
  short     Selected;		/* negative value disables MultiSelect mode */
  uchar     ItemAlign;
  short     TopIndex;		/* item that appears at the top of the list box */
  short     ItemIndex;		/* current focused item */
  TSTRINGS* Items;		
  w_notify  OnClick;
  w_drawitm OnDrawItem;
} TLISTBOX;

/* Function forwards */

extern ushort Max(ushort a, ushort b);
extern ushort Min(ushort a, ushort b);

#ifndef ORI_UZIX
extern void sleep(ushort ms);
#endif

extern void PrintAligned(char* st, short Width, uchar Align, char pad);

extern void WBox(short col, ushort row, ushort width, short height, ushort attr);
extern TWINDOW* WindowOpen(ushort col, ushort row, ushort width, ushort height, ushort attr);
extern short SetWindow(TWINDOW* Window, int property);   
extern TWINDOW* WindowClose(TWINDOW* Window);
extern TCONTROL* ControlRegister(TWINDOW* Window, void* control, BOOL DefaultDraw);
extern void ApplicationLoop();
extern short SendMessage(void* obj, short msg, short param);

extern MRESULT MessageBox(LPSTR lpText, LPSTR lpCaption, ushort uType);

extern TSTRINGS* CreateStrings(short Alloc);
extern void* DestroyStrings(TSTRINGS* Str);
extern void StringsClear(TSTRINGS* Str);
extern BOOL StringsAdd(TSTRINGS* Strings, char* str);
extern char* StringsItem(TSTRINGS* Strings, short Item);
extern BOOL StringsItemSelected(char* Item, BOOL Action);

extern void ControlUpdate(void* ctrl);

extern TBUTTON* CreateButton(TWINDOW* wnd, short col, short row, short width, char* Caption);

extern TLISTBOX* CreateListBox(TWINDOW* wnd, short col, short row, short width, short height, BOOL DefaultDraw);
extern void ListBoxDrawItem(TLISTBOX* lb, short ItemN);
extern void ListBoxClear(TLISTBOX* lb, BOOL redraw);

#define _WINDOWS
#endif
