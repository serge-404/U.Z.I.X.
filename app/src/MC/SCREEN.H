/*

Screen management functions
(c) 2010 Serge

*/

#ifndef _SCREEN

#include	<hitech.h>

#define FALSE	0
#define TRUE	1

/* system dependend - screen and keyboard */

#ifdef ORDOS
#define ScreenWidth	64
#else   			
#define ScreenWidth	80	/* CPM */
#endif

#define ScreenHeight	24	
/*
#define ScreenHeight	25	
*/

#define ScreenSize ScreenWidth*ScreenHeight

#ifdef ORI_UZIX
#define MAXBUFFERING 133
#endif

/* BIOS entries */

#define NWARM	0
#define NCONSTS	1
#define NCONIN	2
#define NCONOUT	3

#ifndef PIOBYTE
#define PIOBYTE  ((BYTE *)0x0003)	/* IOBYTE: TTY=D1D0=00, CRT=D1D0=01 */
#endif

extern uchar	bios(int, ...);

/* Function forwards */

typedef char (*pr_callback)(char ch);

extern char kputch(char ch);
extern char kgetch(void);
extern char kgetche(void);
extern BOOL kbhit(void);	/* returns 255 if a key has been pressed, 0 otherwise. */
/* extern char *itoa(int value, char* strP, int radix);  */
/* extern char *ltoa(long value, char* strP, int radix); */
extern void kprintf(char* fmt, ...);
extern void ksprintf(char* buf, char* fmt, ...);
extern void bputs(char* st);
extern void bgotoxy(uchar x, uchar y);
extern void GotoXY(uchar x /* COL */, uchar y /* ROW */);
extern void ClearScreen();
extern void ClrEoln();
extern void ClrEoscr();
extern char PeekChar();

#define Inkey	(int)kgetch

/* from stdio.h */

#ifndef	NULL
#define	NULL	((void *)0)
#endif	NULL
 
#define _SCREEN
#endif
