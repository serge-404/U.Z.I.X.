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
#include <ctype.h>
#include <string.h>
/*
#include <stdlib.h>
*/
extern int atoi __P((char *str));

/*
 * do a newline and set flags.
 */
#define exprintln()	(zotscreen=YES),println()

extern char *execstr;		/* if someone calls getarg in the	*/
				/* wrong place, death will come...	*/ 
extern int  high,low;		/* low && high end of command range */
extern bool affirm;		/* cmd! */ 

/* execute lines from a :sourced || .lvrc file */

bool PROC
do_file(char *fname, exec_type *mode, bool *noquit)
{
    char line[120];
    register FILE *fp;
    
    if ((fp = fopen(fname,"r")) != NULL) {
	indirect = YES;
	while (fgets(line,120,fp) && indirect) {
	    strtok(line, "\n");
	    if (*line != 0)
		exec(line,mode,noquit);
	}
	indirect = YES;
	fclose(fp);
	return YES;
    }
    return NO;
}


PROC doins(bool flag)
{
    int i;
    curr = low;
    exprintln();
    low = insertion(1,setstep[flag],&i,&i,NO)-1;
    if (low >= 0)
	curr = low;
    diddled = YES;
}


/* figure out a address range for a command */
char * PROC findbounds(char *ip)
{
    ip = findparse(ip, &low, curr);	/* get the low address */
    if (low >= 0) {
	low = bseekeol(low);		/* at start of line */
	if (*ip == ',') {		/* high address? */
	    ip++;
	    count = 0;
	    ip = findparse(ip, &high, curr);
	    if (high >= 0) {
		high = fseekeol(high);
		return(ip);
	    }
	}
	else
	    return(ip);
    }
    return(0);
}


/* parse the command line for lineranges && a command */
PROC parse(char *inp)
{
    int j,k;
    char cmd[80];
    low = high = ERR;
    affirm = 0;
    if (*inp == '%') {
	moveright(inp, 2+inp, 1+strlen(inp));
	inp[0]='1';
	inp[1]=',';
	inp[2]='$';
    }
    while (isspace(*inp))
	++inp;
    if (strchr(".$-+0123456789?/`'", *inp))
	if (!(inp=findbounds(inp))) {
	    errmsg("bad address");
	    return ERR;
	}
    while (isspace(*inp))
	++inp;
    j = 0;
    while (isalpha(*inp))
	cmd[j++] = *inp++;
    if (*inp == '!') {
	if (j == 0)
	    cmd[j++] = '!';
	else
	    affirm++;
	inp++;
    }
    else if (*inp == '=' && j == 0)
	cmd[j++] = '=';
    while (isspace(*inp))
	++inp;
    execstr = inp;
    if (j==0)
	return EX_CR;
    for (k=0; excmds[k]; k++)
	if (strncmp(cmd, excmds[k], j) == 0)
	    return k;
    return ERR;
}


/* inner loop of execmode */
PROC
exec(char *cmd, exec_type *mode, bool *noquit)
{
    register int  what;
    bool ok;
    
    what = parse(cmd);
    ok = YES;
    if (diddled) {
	lstart = bseekeol(curr);
	lend = fseekeol(curr);
    }
    if (what==EX_QUIT) {				/* :quit */
	    if (affirm || what == lastexec || !modified)
		*noquit = NO;
	    else
		errmsg(fismod);
    }
    else if (what==EX_READ) {				/* :read */
	    clrmsg();
	    readfile();
    }
    else if (what==EX_EDIT) {				/* :read, :edit */
	    clrmsg();
	    editfile();
    }
    else if (what==EX_WRITE || what==EX_WQ) {		/* :write, :wq */
	    clrmsg();
	    if (readonly && !affirm)
		prints(fisro);
	    else if (writefile() && what==EX_WQ)
		*noquit = NO;
    }
    else if (what==EX_PREV || what==EX_NEXT) {		/* :next */
	    clrmsg();
	    nextfile(what==EX_PREV);
    }
    else if (what==EX_SUBS) 				/* :substitute */
	    cutandpaste();
    else if (what==EX_SOURCE) {				/* :source */
	    if ((cmd = getarg()) && !do_file(cmd, mode, noquit)) {
		errmsg("cannot open ");
		prints(cmd);
	    }
    }
    else if (what==EX_XIT) {
	    clrmsg();
  	    if (modified && readonly) 
		    prints(fisro);
	    else if ((modified && writefile())|| !modified) {
		    if (!affirm && (argc-pc > 1)) {	/* any more files to edit? */
			printch('(');
			plural(argc-pc-1," more file");
			prints(" to edit)");
		    }
		    else
			*noquit = NO;
	        }
    }
    else if (what==EX_MAP)
	    map(affirm);
    else if (what==EX_UNMAP)
	    ok = unmap();
    else if (what==EX_FILE) {			/* :file */
	    if (cmd=getarg()) {			/* :file name */
		strcpy(altnm, filenm);
		strcpy(filenm, cmd);
		pc = addarg(filenm);
	    }
	    wr_stat();
    }
    else if (what==EX_SET)			/* :set */
	    setcmd();
    else if (what==EX_CR || what==EX_PR) {	/* :print */
	    fixupline(bseekeol(curr));
	    if (what == EX_PR)
		print();
    }
    else if (what==EX_LINE)			/* := */
	    whatline();
    else if (what==EX_DELETE || what==EX_YANK) {	/* :delete, :yank */
	    yank.lines = YES;
	    fixupline(lstart);
	    zerostack(&undo);
	    if (what == EX_DELETE)
		ok = deletion(low,high);
	    else
		ok = doyank(low,high);
	    diddled = YES;
    }
    else if (what==EX_PUT) {			/* :put */
	    fixupline(lstart);
	    zerostack(&undo);
	    ok = putback(low, &high);
	    diddled = YES;
    }
    else if (what==EX_VI) {			/* :visual */
	    *mode = E_VISUAL;
	    if (*execstr) {
		clrmsg();
		nextfile(NO);
	    }
    }
    else if (what==EX_EX)
	    *mode = E_EDIT;			/* :execmode */
    else if (what==EX_INSERT || what==EX_OPEN){	/* :insert, :open */
	    if (indirect)
		ok = NO;		/* kludge, kludge, kludge!!!!!!!!!! */
	    else {
		zerostack(&undo);
		fixupline(lstart);
		doins(what == EX_OPEN);
	    }
    }
    else if (what==EX_CHANGE) {			/* :change */
	    if (indirect)
		ok = NO;		/* kludge, kludge, kludge!!!!!!!!!! */
	    else {
		zerostack(&undo);
		yank.lines = YES;
		fixupline(lstart);
		if (deletion(low,high))
		    doins(YES);
		else
		    ok = NO;
	    }
    }
    else if (what==EX_UNDO) {			/* :undo */
	    low = fixcore(&high);
	    if (low >= 0) {
		diddled = YES;
		curr = low;
	    }
	    else ok = NO;
    }
    else if (what==EX_ARGS)			/* :args */
	    args();
    else if (what==EX_VERSION)			/* version */
	    version();
    else if (what==EX_ESCAPE) {			/* shell escape hack */
	    zotscreen = YES;
	    exprintln();
	    if (*execstr) {
#if ZTERM
		zclose();
#endif
#if FLEXOS|UNIX
		fixcon();
#else
		allowintr();
#endif
/* //FIXME!!		system(execstr); */
#if FLEXOS|UNIX
		initcon();
#else
		nointr();
#endif
	    }
	    else
		prints("incomplete shell escape.");
    }
    else if (what==EX_REWIND) {
	    clrmsg();
	    if (argc > 0 && oktoedit(autowrite)) {
		pc = 0;
		doinput(argv[0]);
	    }
    }
    else
	prints(":not an editor command.");

    lastexec = what;
    if (!ok) {
	errmsg(excmds[what]);
	prints(" error");
    }
}
