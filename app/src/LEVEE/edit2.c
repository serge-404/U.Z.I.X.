/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-1997 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by David L Parsons (orc@pell.chi.il.us).  My name may not be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.  THIS SOFTWARE IS PROVIDED
 * AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "levee.h"
#include "extern.h"

#include <string.h>
#include <ctype.h>
/* do some undoable modification */

/* These variables make docommand nonrecursive */

extern bool	ok;
extern int	newend,		/* end position after change */
	disp,		/* start redisplay here */
	newc,		/* new cursor position for wierd cmds */
	endY;		/* final yp for endp */


/* Initialize && execute a macro */

exec_type PROC
editcore(void)
{
    register cmdtype cmd;
    extern bool s_wrapped;

    /* rcb[0] = 0; rcp = rcb; */

    if (diddled) {
	setpos(skipws(curr));		/* set cursor x position.. */
	yp = settop(LINES / 2);		/* Y position */
    }
    if (diddled || zotscreen)		/* redisplay? */
	redisplay(FALSE);
    mvcur(yp, xp);			/* and move the cursor */

    for (;;) {
	s_wrapped = 0;
	ch = 0x7f & readchar();			/* get 7-bits of a char */
	gcount();			/* ... a possible count */
	cmd = movemap[ch];
        if (cmd==FILE_C) {
	    wr_stat();			/* write file stats */
	    mvcur(yp, xp);
	}
	else if (cmd==WINDOW_UP || cmd==WINDOW_DOWN)
	    scroll(cmd==WINDOW_UP);		/* scroll the window */
	else if (cmd==REDRAW_C) {			/* redraw the window */
	    redisplay(TRUE);
	    mvcur(yp, xp);
	}
	else if (cmd==MARKER_C) {			/* set a marker */
	    ch = tolower(readchar());
	    if (ch >= 'a' && ch <= 'z')
		contexts[ch-'`'] = curr;
	    else if (ch != ESC)
		error();
	}
	else if (cmd==REDO_C) {
	    if (rcb[0] != 0) {
		zerostack(&undo);
		insertmacro(rcb, 1);
		redoing = TRUE;
	    }
	}
	else if (cmd==REWINDOW) 
	    zdraw(readchar());		/* shift the window */
	else if (cmd==DEBUG_C) ;	/* debugging stuff -- unused */
	else if (cmd==ZZ_C) {			/* shortcut for :xit */
	    ch = readchar();
	    if (ch == 'Z')
		insertmacro(":x\r", 1);
	    else if (ch != ESC)
		error();
	}
	else if (cmd==EDIT_C)		/* drop into line mode */
	    return E_EDIT;
	else if (cmd==COLIN_C)		/* do one exec mode command */
	    return E_VISUAL;
	else if (cmd==HARDMACRO)
	    macrocommand();		/* 'hard-wired' macros */
	else if (cmd==SOFTMACRO)
	    exmacro();			/* run a macro */
	else if (cmd==INSMACRO || cmd==BAD_COMMAND)		/* macro for insert mode */
	    error();
	else {
	    if (cmd < DELETE_C)
		movearound(cmd);
	    else /*if (cmd < HARDMACRO)*/
		docommand(cmd);
	}
	lastexec = 0;
    }
    /* never exits here */
}

PROC
exmacro(void)
{
    register int mp;

    mp = lookup(ch);
    if (mp > 0) {
	if (macro<0)
	    zerostack(&undo);
	insertmacro(mbuffer[mp].m_text, count);
    }
    else
	error();
}

/* redraw the screen w.r.t. the cursor */

PROC
zdraw(char code)
{
    register int nl;
    int np = (count>0)?to_index(count):curr;

    nl = ERR;
    if (movemap[code] == CR_FWD)
	nl = 0;
    else if (movemap[code] == CR_BACK)
	nl = LINES-1;
    else if (code == '.')
	nl = LINES / 2;
    if (nl >= 0) {
	curr = np;
	yp = settop(nl);
	redisplay(TRUE);
	mvcur(yp,xp);
    }
    else
	error();
}

/* start up a built-in macro */

PROC
macrocommand(void)
{
    if (count > 1)
	numtoa(gcb,count);
    else
	gcb[0] = 0;
/* which macro? */
    if (ch=='x')			/* x out characters */
	    strcat(gcb,"dl"); 
    else if (ch=='X')			/* ... backwards */
	    strcat(gcb,"dh");
    else if (ch=='s')			/* substitute over chars */
	    strcat(gcb,"cl");
    else if (ch=='D')			/* delete to end of line */
	    strcat(gcb,"d$");
    else if (ch=='C')			/* change ... */
	    strcat(gcb,"c$");
    else if (ch=='Y')			/* yank ... */
	    strcat(gcb,"y$");
    else if (ch==0x06)			/* ^F */   /* scroll up one page */
	    strcpy(gcb,"22\x04");       /* 22^D */
    else if (ch==0x02)			/* ^B */   /* ... down one page */
	    strcpy(gcb,"22\x15");	/* 22^U */
    else if (ch==0x05)			/* ^E */   /* scroll up one line */
	    strcpy(gcb,"1\x04");	/* 1^D */
    else if (ch==0x19)			/* ^Y */   /* ... down one line */
	    strcpy(gcb,"1\x15");	/* 1^U */
    else {
            error();
            return 0;
    }
    if (macro<0)
	zerostack(&undo);
    insertmacro((void*)gcb, 1);
}

/* scroll the window up || down */

PROC
scroll(bool down)
{
    register int i;

    if (count <= 0)
	count = dofscroll;
    strput(CURoff);
    if (down) {
	curr = min(bufmax-1, nextline(TRUE, curr, count));
	i = min(bufmax-1, nextline(TRUE, pend, count));
	if (i > pend)
	    scrollforward(i);
    }
    else {
	curr = bseekeol(max(0,nextline(FALSE, curr, count)));
	i = bseekeol(max(0,nextline(FALSE, ptop, count)));
	if (i < ptop)
	    if (canUPSCROLL)
		scrollback(i);
	    else {
		ptop = i;
		setend();
		redisplay(TRUE);
	    }
    }
    strput(CURon);
    setpos(skipws(curr));	/* initialize new position - first nonwhite */
    yp = setY(curr);
    mvcur(yp, xp);		/* go there */
}

