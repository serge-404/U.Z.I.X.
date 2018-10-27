/*

Screen management functions
(c) 2010 Serge

*/

/*				VT52 compliant codes
				====================

			Orion Monitor-2		Orion CPM (ACPM)	MYZ80 CPM.EXE
			---------------		----------------	-------------

  01h			------------		------------		HOME
  04h			------------		------------		CLS+HOME

  07h			BELL			BELL			BELL
+ 08h			cursor LEFT		cursor LEFT		BACKSPACE
+ 09h			TAB			TAB			TAB
+ 0Ah			LF 			LF			LF
  0Bh			------------		------------		cursor UP
  0Ch			HOME			------------		CLS
+ 0Dh			CR			CR			CR
  
  18h			cursor RIGHT		cursor RIGHT		CLREOLN
  19h			cursor UP		cursor UP		-----------
  1Ah			cursor DOWN		cursor DOWN		CLS+HOME

  1Eh			------------		------------ 		HOME
+ 1Fh			CLS+HOME		CLS+HOME		CR+LF

+ 7Fh			INVERSE			------------		-----------

+ 1Bh+34h='4' 		------------		LineWrap ON		-----------
+ 1Bh+35h='5' 		------------ 		LineWrap OFF		-----------
+ 1Bh+36h='6' 		------------		Inverse ON		-----------
+ 1Bh+37h='7' 		------------ 		Inverse OFF		-----------

  1Bh+3Ah=':' 		------------		cursor ON		CLS
  1Bh+3Bh=';' 		------------ 		cursor OFF		CLS

+ 1Bh+41h='A'		cursor UP					cursor UP
+ 1Bh+42h='B'		cursor DOWN					cursor DOWN
+ 1Bh+43h='C'		cursor RIGHT					cursor RIGHT
+ 1Bh+44h='D'		cursor LEFT					cursor LEFT
+ 1Bh+45h='E'		CLS			CLS			CLS+HOME

+ 1Bh+48h='H'		HOME			HOME			HOME
  1Bh+49h='I'		------------		set/get ActiveScreen	reverse LineFeed
+ 1Bh+4Ah='J'		CLRREOSCR		CLRREOSCR		CLRREOSCR
+ 1Bh+4Bh='K'		CLREOLN			CLREOLN			CLREOLN
  1Bh+4Ch='L'		------------		INSROW			INSROW
  1Bh+4Dh='M'		------------		DELROW			DELROW

  1Bh+52h='R'		------------		set cursor height	DELROW

+ 1Bh+59h='Y'+Y+X	cursor position		cursor position		cursor position

  1Bh+64h='d'		------------		-----------		CLRBOSCR
  1Bh+65h='e' [		------------		-----------		cursor ON
  1Bh+66h='f'		------------		-----------		cursor OFF

  1Bh+6Ah='j'		------------		-----------		save cursor
  1Bh+6Bh='k'		------------		-----------		restore cursor position
  1Bh+6Ch='l'		------------		-----------		clear line

  1Bh+6Fh='o'		------------		-----------		CLRBOLN
+ 1Bh+70h='p'		------------		-----------		Inverse ON
+ 1Bh+71h='q'		------------		-----------		Inverse OFF

+ 1Bh+76h='v'		------------		-----------		line wrap on
+ 1Bh+77h='w'		------------		-----------		line wrap off

*/

#ifdef ORI_UZIX
#include	<unistd.h>
#else
/*
#include	<stdlib.h> */	/* malloc */
#endif
#include	<string.h>
#include	"screen.h"

/* VT52 control codes */

#define CUR_LEFT	0x08
#define TAB		0x09
#define LF		0x0a
#define CR		0x0d
#define CUR_RIGHT	0x18
#define CUR_UP		0x19
#define CUR_DOWN	0x1a
#define CLS_HOME	0x1f
#define TOGGLE_INVERSE	0x7f
#define E_ESCAPE	0x1b
#define E_WRAP_ON	0x34
#define E_WRAP_OFF	0x35
#define E_INV_ON	0x36
#define E_INV_OFF	0x37
#define E_CUR_UP	0x41		
#define E_CUR_DOWN	0x42
#define E_CUR_RIGHT	0x43
#define E_CUR_LEFT	0x44
#define E_CLS		0x45
#define E_HOME		0x48
#define E_CLRREOSCR	0x4A
#define E_CLREOLN	0x4B
#define E_CUR_POS	0x59
#define E_INV__ON	0x70
#define E_INV__OFF	0x71
#define E_WRAP__ON	0x76
#define E_WRAP__OFF	0x77

static	char	*kbufptr, EscChar=0;
	short	LineWrap=1, ScreenCol=0, ScreenRow=0, SCol, SRow;
	short	WndRow=0, WndCol=0, WndWidth=ScreenWidth, WndHeight=ScreenHeight;
static	short	InverseOn=0, ii, jj;
	char	ScreenBuf[ScreenSize+1];		/* ASCII copy of current screen */
	void	*topwnd=NULL;			/* currently operating window (top window) */
#ifdef ORI_UZIX
	BOOL	AltairDos=TRUE;
        uchar bcount=0, FlushImmed=0;
        char bbuffer[MAXBUFFERING];
#else
	BOOL	AltairDos=FALSE;
#endif

/* console output without screen buffering but with output buffering for UZIX where by-char output is slow */

#ifdef ORI_UZIX
void bflush() {
   if (bcount) {
     write(STDOUT_FILENO, bbuffer, bcount);
     bcount=0;
   }
}
#endif

void bputch(char ch) {
#ifdef ORI_UZIX
   bbuffer[bcount]=ch;
   bcount++;
   if (FlushImmed || (bcount>MAXBUFFERING-5)) {
     FlushImmed=0;
     bflush();
   }
#else
   bios(NCONOUT, ch);
#endif
}

void bputs(char* st) {
#ifdef ORI_UZIX
        bflush();
        write(STDOUT_FILENO, st, strlen(st));
#else
	while (*st) bios(NCONOUT, *(st++));
#endif
}

char VTstr[]="\x1b\x5b\x3f\x32\x6c\x1a";   /* Esc[?2l = Set VT52 (versus ANSI); 26=1a clear & home for some displays */
char CEstr[]="\x1b\x4a";                   /* VT52 displays - clear to the end of screen = ESC+J */
char XYstr[]="\x1b\x59\x20\x20";           /* VT52 screen positioning = ESC+Y+y20+x20 */

void bgotoxy(uchar x, uchar y)
{
    if (topwnd) {
/*	bios(NCONOUT, 27);
	bios(NCONOUT, 'Y');
	bios(NCONOUT, y+32);
	bios(NCONOUT, x+32);*/
       XYstr[2]=y+32;
       XYstr[3]=x+32;
       bputs(XYstr);
    }
}


/* Screen functions, Escape-codes processing for screen char buffering */

char	kstorech(char ch)
{
	*kbufptr=ch;
	*(++kbufptr)='\0';
	return ch;
}

void StoreXY()
{
  SCol=ScreenCol; SRow=ScreenRow;
}

void RestoreXY()
{
  ScreenCol=SCol; ScreenRow=SRow;
  bgotoxy(ScreenCol, ScreenRow);
}

void lscrollup()
{
  register char cc;
  if (! topwnd)
    memcpy(ScreenBuf, &ScreenBuf[ScreenWidth], ScreenWidth*(ScreenHeight-1));
  else {
    for (ii=WndRow; ii<WndRow+WndHeight-1; ii++) { 
      bgotoxy(WndCol, ii);
      for (jj=WndCol; jj<WndCol+WndWidth; jj++) { 
	ScreenBuf[ii*ScreenWidth+jj]=cc=ScreenBuf[(ii+1)*ScreenWidth+jj];
	bputch(cc);                                                      /* bios(NCONOUT, cc); */
      }
    }
    bgotoxy(WndCol, ii);
    for (jj=WndCol; jj<WndCol+WndWidth; jj++) {
      ScreenBuf[ii*ScreenWidth+jj]=' ';
      bputch(' ');                                                       /* bios(NCONOUT, ' '); */
    }
#ifdef ORI_UZIX
    bflush();
#endif
  }
}

void	lcurup()
{
	if (ScreenRow>WndRow) ScreenRow--; 
	else {
	    if (! LineWrap) ScreenRow=WndRow+WndHeight-1;
	}
	bgotoxy(ScreenCol, ScreenRow);
}

void	lcurdn()
{
	if (ScreenRow<WndRow+WndHeight-1) ScreenRow++; 
	else  {
	  if (LineWrap)
	    lscrollup();
          else
	    ScreenRow=WndRow;
	}
	bgotoxy(ScreenCol, ScreenRow);
}

void	lcurleft()
{
	if (ScreenCol>WndCol) ScreenCol--;
	else {
          ScreenCol=WndCol+WndWidth-1;
          if (LineWrap) lcurup();
        }
	bgotoxy(ScreenCol, ScreenRow);
}

void	lcurright()
{
	if (ScreenCol<WndCol+WndWidth-1) ScreenCol++;
	else {
          ScreenCol=WndCol;
          if (LineWrap)
	    lcurdn();
	  else
	    bgotoxy(ScreenCol, ScreenRow);
        }
}

char	lcls(char ch)
{
    if (! topwnd) {
	memset(ScreenBuf, 32, ScreenSize);
	return ch; 
    }
    else 
    for (ii=WndRow; ii<WndRow+WndHeight; ii++) {      
      bgotoxy(WndCol, ii);
      for (jj=WndCol; jj<WndCol+WndWidth; jj++) { 
	ScreenBuf[ii*ScreenWidth+jj]=' ';
	bputch(' ');
      }
    }
#ifdef ORI_UZIX
    bflush();
#endif
    return 0;
}

char	kputch(char ch)
{
    register BOOL curright=FALSE;
    if (! EscChar) {
      if (curright=(ch>31)) {
         if (ch==TOGGLE_INVERSE) {        /* TOGGLE_INVERSE may be >31 */
	    InverseOn=!InverseOn;
            curright=0;
         }
         else
            ScreenBuf[ScreenRow*ScreenWidth+ScreenCol]=ch;
      }
      else {
       switch (ch) {
	case TAB: ScreenCol |= 7;
		  lcurright();
		  break;
        case CUR_LEFT:	lcurleft();
			break;
	case CUR_RIGHT:	lcurright();
			break;
	case CUR_UP:	lcurup();
			break;
	case LF:
	case CUR_DOWN:	lcurdn();
			break;
	case CLS_HOME: 	/*	ScreenCol=WndCol;		*/
				ScreenRow=WndRow;
				ch=lcls(ch);
/*				break;				*/
	case CR:		ScreenCol=WndCol;
				break;  
	case TOGGLE_INVERSE:	InverseOn=!InverseOn;
				break;
	case E_ESCAPE:		EscChar=1;
				break;
        default: 
	  break;
       }
      }
    } 
    else 
    {
      if (EscChar=='Y') {
	  ScreenRow=ch-32;
	  if (topwnd) { ScreenRow+=WndRow; ch+=(char)WndRow; }
          EscChar='y';   
      } 
      else if (EscChar=='y') {
          ScreenCol=ch-32;
	  if (topwnd) { ScreenCol+=WndCol; ch+=(char)WndCol; }
          EscChar=0;
#ifdef ORI_UZIX
          FlushImmed=1;
#endif
      }
      else {
        EscChar=0;
	switch (ch) {
          case E_CUR_UP:  lcurup();
			  break;
          case E_CUR_DOWN:lcurdn();
			  break;
          case E_CUR_RIGHT:lcurright();
			  break;
          case E_CUR_LEFT:lcurleft();
			  break;
          case E_CLS:	  ch=lcls(ch);
			  break;
          case E_HOME:    ScreenCol=WndCol;
                          ScreenRow=WndRow;
			  break;
	  case E_CLRREOSCR:memset(&ScreenBuf[ScreenRow*ScreenWidth+ScreenCol], 32,
                                  ScreenSize-(ScreenRow*ScreenWidth+ScreenCol));
			   break;
	  case E_CLREOLN: memset(&ScreenBuf[ScreenRow*ScreenWidth+ScreenCol], 32,
				 ScreenWidth-1-ScreenCol); 
			  break;
          case E_CUR_POS: EscChar='Y';
			  break;
          case E_WRAP_ON: 
          case E_WRAP__ON:LineWrap=1;
			  break;
          case E_WRAP_OFF:
          case E_WRAP__OFF:LineWrap=0;
			  break;
	  case E_INV_ON:
	  case E_INV__ON: InverseOn=1;
			  break;
	  case E_INV_OFF:
	  case E_INV__OFF:InverseOn=0;
			  break;
	  default: ;
        }
      }
    }
    bputch(ch);
    if (curright) lcurright();
    return ch;
}

char	PeekChar()
{
  return ScreenBuf[ScreenRow*ScreenWidth+ScreenCol];
}

char	kgetch(void)
{
	return bios(NCONIN);
}

char	kgetche(void)
{
	return kputch(kgetch());
}

BOOL	kbhit(void)	/* returns 255 if a key has been pressed, 0 otherwise. */
{
#ifdef ORI_UZIX
        bflush();
#endif
	return bios(NCONSTS);
}

/* number-to-string coversion routines */

void __itoa (unsigned value, char *strP, uchar radix) {
	char buf[34];
	register char *_di = strP, *_si = buf;
	uchar len;

	do {
		*_si++ = (char)(value % radix);
		value /= radix;
	} while (value != 0);
	/* The value has now been reduced to zero and the
	 * digits are in the buffer.
	 */
	/* The digits in the buffer must now be copied in reverse order into
	 *  the target string, translating to ASCII as they are moved.
	 */
	len = (uchar)(_si-buf);
	while (len != 0) {
		--len;
		radix = (uchar)*--_si;
		*_di++ = (char)((uchar)radix + (((uchar)radix < 10) ? '0' : 'A'-10));
	}
	/* terminate the output string with a zero. */
	*_di ='\0';
}

void __ltoa (long value, char *strP, uchar radix) {
	char buf[34];
	register char *_di = strP, *_si = buf;
	uchar len;

	do {
		*_si++ = (char)(value % radix);
		value /= radix;
	} while (value != 0);
	/* The value has now been reduced to zero and the
	 * digits are in the buffer.
	 */
	/* The digits in the buffer must now be copied in reverse order into
	 *  the target string, translating to ASCII as they are moved.
	 */
	len = (uchar)(_si-buf);
	while (len != 0) {
		--len;
		radix = (uchar)*--_si;
		*_di++ = (char)((uchar)radix + (((uchar)radix < 10) ? '0' : 'A'-10));
	}
	/* terminate the output string with a zero. */
	*_di ='\0';
}
 
/*
 */
char *itoa(value, strP, radix)
	int value;
	char *strP;
	int radix;
{
	register char *p = strP;

	if (radix == 0) {
		if (value < 0) {
			*p++ = '-';
			value = -value;
		}
		radix = 10;
	}
	__itoa((unsigned)value,p,(unsigned)radix);
	return strP;
}

/*
 */
char *ltoa(value, strP, radix)
	long value;
	char *strP;
	int radix;
{
	register char *p = strP;

	if (radix == 0) {
		if (value < 0) {
			*p++ = '-';
			value = -value;
		}
		radix = 10;
	}
	__ltoa(value,p,(unsigned)radix);
	return strP;
}

/* Short version of (s)printf */

void kxprintf(pr_callback OnChar, char* fmt, uchar **arg)
{
	register int base;
	uchar l, w, c, pad, s[12], *p, *q;
	int len = 0;

	while ((w = (uchar)*fmt++) != 0) {
		if (w != '%') {
		   	if (w=='\n') OnChar(CR);
			OnChar(w);
			continue;
		}
		pad = (uchar)(*fmt == '0' ? '0' : ' ');
		w = 0;
		while ((*fmt >= '0') && (*fmt <= '9')) {
			w = w * 10;
			w += (uchar)(*fmt - '0');
			++fmt;
		}
		s[1] = 0;
		p = s;
		len = 0x7FFF;
		switch (c = (uchar)*fmt++) {
		case 'c':
			s[0] = *(uchar *)arg++;
			break;
		case 'l': base = 0; 
                          if ((uchar)*fmt=='u') {
				base=10; fmt++;
			  }
			  ltoa(*(long *)arg++, (char *)s, base);
                          arg++;
                          break;
		case 'd': base = 0;	goto prt;
		case 'o': base = 8;	goto prt;
		case 'b': base = 2;	goto prt;
		case 'u': base = 10;	goto prt;
		case 'p':
			w = 4;
			pad = '0';
		case 'x':
			base = 16;
prt:			itoa(*(int *)arg++, (char *)s, base);
			break;
		case 'n':
			p = *arg++;
			len = *(int *)arg++;
			break;
		case 's':
			p = *arg++;
			break;
		default:
			s[0] = c;
			break;
		}
		if (w) {
			l = 0;
			q = p;
			while (--len != 0 && *q++ != 0)
				++l;
			w -= l;
			while ((int)w-- > 0)
				OnChar(pad);
		}
		while (*p) OnChar(*(p++));
	}
}

void kprintf(fmt)
	char* fmt;
{
	kxprintf(kputch, fmt, (uchar **)&fmt + 1);
}

void ksprintf(buf, fmt)
	register char* buf;
	char* fmt;
{
	if (buf) {
          kbufptr=buf;
	  kxprintf(kstorech, fmt, (uchar **)&fmt + 1);
        }
}

void GotoXY(uchar x /* COL */, uchar y /* ROW */)		/* VT52 display */
{
  kputch(27); kputch('Y'); kputch(y+32); kputch(x+32);
}

void ClearScreen()			/* VT52 display */
{
  if (! AltairDos) {
/*    bios(NCONOUT, 27);			
    bios(NCONOUT, '[');
    bios(NCONOUT, '?');
    bios(NCONOUT, '2');
    bios(NCONOUT, 'l');
    bios(NCONOUT, 26); */
    bputs(VTstr);
  }
  kputch(27); kputch('E');			/* Only clear screen for Amstrad & ACPM */  
  kputch(27); kputch('H'); 			/* home cursor */  
  if ((AltairDos) && (!(*PIOBYTE & 3))) {	/* if console device = TTY: */
    bputs(CEstr);
/*    bios(NCONOUT, 27);
    bios(NCONOUT, 'J');	*/		/* other VT52 displays - clear to the end of screen */ 
  }
}

void ClrEoln()				/* VT52 display */
{
  kputch(27); kputch('K');   
}

void ClrEoscr()				/* VT52 display */
{
  kputch(27); kputch('J');   
}

/* BackSpace for both VT52 terminals - ACPM & CPM.EXE */   
/*
void BackSpace()			
{
  kputch(8); kputch(' '); kputch(8);	
}
*/

