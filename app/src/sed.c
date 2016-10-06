/*
  Copyright (c) 1987,1997, Prentice Hall
  All rights reserved.
  
  Redistribution and use of the MINIX operating system in source and
  binary forms, with or without modification, are permitted provided
  that the following conditions are met:
  
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
  
     * Redistributions in binary form must reproduce the above
       copyright notice, this list of conditions and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution.
  
     * Neither the name of Prentice Hall nor the names of the software
       authors or contributors may be used to endorse or promote
       products derived from this software without specific prior
       written permission.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS, AUTHORS, AND
  CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL PRENTICE HALL OR ANY AUTHORS OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* sed - stream editor		Author: Eric S. Raymond */

/* This used to be three different files with the following makefile:
 * (Note the chmem).

CFLAGS=	-F -T.

OBJS=   sedcomp.s sedexec.s

sed: 	$(OBJS)
        cc -T. -o sed $(OBJS)
  @chmem =13312 sed

$(OBJS):	sed.h

 * If you want longer lines: increase MAXBUF.
 * If you want scripts with more text: increase POOLSIZE.
 * If you want more commands per script: increase MAXCMDS.
 */

#include "features.h"
#include "ctype.h"
#include "types.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "stdio.h"
#include "sed.h"

/*+++++++++++++++*/

/* Sed - stream editor		Author: Eric S. Raymond */

/*
   The stream editor compiles its command input	 (from files or -e options)
   into an internal form using compile() then executes the compiled form using
   execute(). Main() just initializes data structures, interprets command line
   options, and calls compile() and execute() in appropriate sequence.

   The data structure produced by compile() is an array of compiled-command
   structures (type sedcmd).  These contain several pointers into pool[], the
   regular-expression and text-data pool, plus a command code and g & p flags.
   In the special case that the command is a label the struct  will hold a ptr
   into the labels array labels[] during most of the compile,  until resolve()
   resolves references at the end.

   The operation of execute() is described in its source module.
*/

/* Main data areas */
char linebuf[MAXBUF + 1];	/* current-line buffer */
sedcmd cmds[MAXCMDS + 1];	/* hold compiled commands */
long linenum[MAXLINES];		/* numeric-addresses table */

/* Miscellaneous shared variables */
int nflag;			/* -n option flag */
int eargc;			/* scratch copy of argument count */
char **eargv;			/* scratch copy of argument list */
char bits[] = {1, 2, 4, 8, 16, 32, 64, 128};

/* Error messages */
static char AGMSG[] = "sed: garbled address %s\n";
static char CGMSG[] = "sed: garbled command %s\n";
static char TMTXT[] = "sed: too much text: %s\n";
static char AD1NG[] = "sed: no addresses allowed for %s\n";
static char AD2NG[] = "sed: only one address allowed for %s\n";
static char TMCDS[] = "sed: too many commands, last was %s\n";
static char COCFI[] = "sed: cannot open command-file %s\n";
static char UFLAG[] = "sed: unknown flag %c\n";
static char CCOFI[] = "sed: cannot create %s\n";
static char ULABL[] = "sed: undefined label %s\n";
static char TMLBR[] = "sed: too many {'s\n";
static char FRENL[] = "sed: first RE must be non-null\n";
static char NSCAX[] = "sed: no such command as %s\n";
static char TMRBR[] = "sed: too many }'s\n";
static char DLABL[] = "sed: duplicate label %s\n";
static char TMLAB[] = "sed: too many labels: %s\n";
static char TMWFI[] = "sed: too many w files\n";
static char REITL[] = "sed: RE too long: %s\n";
static char TMLNR[] = "sed: too many line numbers\n";
static char TRAIL[] = "sed: command \"%s\" has trailing garbage\n";

/* Label handling */
static label labels[MAXLABS];	/* here's the label table */
static label *lab = labels + 1;	/* pointer to current label */
static label *lablst = labels;	/* header for search list */

/* String pool for regular expressions, append text, etc. etc. */
static char pool[POOLSIZE];	/* the pool */
static char *fp = pool;		/* current pool pointer */
static char *poolend = pool + POOLSIZE;	/* pointer past pool end */

/* Compilation state */
static FILE *cmdf = NULL;	/* current command source */
static char *cp = linebuf;	/* compile pointer */
static sedcmd *cmdp = cmds;	/* current compiled-cmd ptr */
static char *lastre = NULL;	/* old RE pointer */
static int bdepth = 0;		/* current {}-nesting level */
static int bcount = 0;		/* # tagged patterns in current RE */

/* Compilation flags */
static int eflag;		/* -e option flag */
static int gflag;		/* -g option flag */

#define H	0x80		/* 128 bit, on if there's really code for * command */
#define LOWCMD	56		/* = '8', lowest char indexed in cmdmask */

/* Indirect through this to get command internal code, if it exists */
static char cmdmask[] =
{
 0, 0, H, 0, 0, H + EQCMD, 0, 0,
 0, 0, 0, 0, H + CDCMD, 0, 0, CGCMD,
 CHCMD, 0, 0, 0, 0, 0, CNCMD, 0,
 CPCMD, 0, 0, 0, H + CTCMD, 0, 0, H + CWCMD,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, H + ACMD, H + BCMD, H + CCMD, DCMD, 0, 0, GCMD,
 HCMD, H + ICMD, 0, 0, H + LCMD, 0, NCMD, 0,
 PCMD, H + QCMD, H + RCMD, H + SCMD, H + TCMD, 0, 0, H + WCMD,
 XCMD, H + YCMD, 0, H + BCMD, 0, H, 0, 0,
};

static void compile(void)
/* Precompile sed commands out of a file */
{
  char ccode;


  for (;;) {			/* main compilation loop */
	if (*cp == '\0') {	/* get a new command line */
		*linebuf = '\0';	/* K.H */
		if (cmdline(cp = linebuf) < 0) break;
	}
	SKIPWS(cp);
	if (*cp == '\0')	/* empty */
		continue;
	if (*cp == '#') {	/* comment */
		while (*cp) ++cp;
		continue;
	}
	if (*cp == ';') {	/* ; separates cmds */
		cp++;
		continue;
	}

	/* Compile first address */
	if (fp > poolend)
		ABORT(TMTXT);
	else if ((fp = address(cmdp->addr1 = fp)) == BAD)
		ABORT(AGMSG);

	if (fp == cmdp->addr1) {/* if empty RE was found */
		if (lastre)	/* if there was previous RE */
			cmdp->addr1 = lastre;	/* use it */
		else
			ABORT(FRENL);
	} else if (fp == NULL) {/* if fp was NULL */
		fp = cmdp->addr1;	/* use current pool location */
		cmdp->addr1 = NULL;
	} else {
		lastre = cmdp->addr1;
		if (*cp == ',' || *cp == ';') {	/* there's 2nd addr */
			cp++;
			if (fp > poolend) ABORT(TMTXT);
			fp = address(cmdp->addr2 = fp);
			if (fp == BAD || fp == NULL) ABORT(AGMSG);
			if (fp == cmdp->addr2)
				cmdp->addr2 = lastre;
			else
				lastre = cmdp->addr2;
		} else
			cmdp->addr2 = NULL;	/* no 2nd address */
	}
	if (fp > poolend) ABORT(TMTXT);

	SKIPWS(cp);		/* discard whitespace after address */
	IFEQ(cp, '!') cmdp->flags.allbut = 1;

	SKIPWS(cp);		/* get cmd char, range-check it */
	if ((*cp < LOWCMD) || (*cp > '~')
	    || ((ccode = cmdmask[*cp - LOWCMD]) == 0))
		ABORT(NSCAX);

	cmdp->command = ccode & ~H;	/* fill in command value */
	if ((ccode & H) == 0)	/* if no compile-time code */
		cp++;		/* discard command char */
	else if (cmdcomp(*cp++))/* execute it; if ret = 1 */
		continue;	/* skip next line read */

	if (++cmdp >= cmds + MAXCMDS) ABORT(TMCDS);

	SKIPWS(cp);		/* look for trailing stuff */
	if (*cp != '\0' && *cp != ';' && *cp != '#') ABORT(TRAIL);
  }
}

static int cmdcomp(char cchar)
/* Compile a single command */
{
  static sedcmd **cmpstk[MAXDEPTH];	/* current cmd stack for {} */
  static char *fname[WFILES];	/* w file name pointers */
  static FILE *fout[WFILES];	/* w file file ptrs */
  static int nwfiles = 1;	/* count of open w files */
  int i;			/* indexing dummy used in w */
  sedcmd *sp1, *sp2;		/* temps for label searches */
  label *lpt;
  char redelim;			/* current RE delimiter */

  fout[0] = stdout;
  switch (cchar) {
      case '{':			/* start command group */
	cmdp->flags.allbut = !cmdp->flags.allbut;
	cmpstk[bdepth++] = &(cmdp->u.link);
	if (++cmdp >= cmds + MAXCMDS) ABORT(TMCDS);
	return(1);

      case '}':			/* end command group */
	if (cmdp->addr1) ABORT(AD1NG);	/* no addresses allowed */
	if (--bdepth < 0) ABORT(TMRBR);	/* too many right braces */
	*cmpstk[bdepth] = cmdp;	/* set the jump address */
	return(1);

      case '=':			/* print current source line number */
      case 'q':			/* exit the stream editor */
	if (cmdp->addr2) ABORT(AD2NG);
	break;

      case ':':			/* label declaration */
	if (cmdp->addr1) ABORT(AD1NG);	/* no addresses allowed */
	fp = gettext(lab->name = fp);	/* get the label name */
	if (lpt = search(lab)) {/* does it have a double? */
		if (lpt->address) ABORT(DLABL);	/* yes, abort */
	} else {		/* check that it doesn't overflow label table */
		lab->last = NULL;
		lpt = lab;
		if (++lab >= labels + MAXLABS) ABORT(TMLAB);
	}
	lpt->address = cmdp;
	return(1);

      case 'b':			/* branch command */
      case 't':			/* branch-on-succeed command */
      case 'T':			/* branch-on-fail command */
	SKIPWS(cp);
	if (*cp == '\0') {	/* if branch is to start of cmds... */
		/* Add current command to end of label last */
		if (sp1 = lablst->last) {
			while (sp2 = sp1->u.link) sp1 = sp2;
			sp1->u.link = cmdp;
		} else		/* lablst->last == NULL */
			lablst->last = cmdp;
		break;
	}
	fp = gettext(lab->name = fp);	/* else get label into pool */
	if (lpt = search(lab)) {/* enter branch to it */
		if (lpt->address)
			cmdp->u.link = lpt->address;
		else {
			sp1 = lpt->last;
			while (sp2 = sp1->u.link) sp1 = sp2;
			sp1->u.link = cmdp;
		}
	} else {		/* matching named label not found */
		lab->last = cmdp;	/* add the new label */
		lab->address = NULL;	/* it's forward of here */
		if (++lab >= labels + MAXLABS)	/* overflow if last */
			ABORT(TMLAB);
	}
	break;

      case 'a':			/* append text */
      case 'i':			/* insert text */
      case 'r':			/* read file into stream */
	if (cmdp->addr2) ABORT(AD2NG);
      case 'c':			/* change text */
	if ((*cp == '\\') && (*++cp == '\n')) cp++;
	fp = gettext(cmdp->u.lhs = fp);
	break;

      case 'D':			/* delete current line in hold space */
	cmdp->u.link = cmds;
	break;

      case 's':			/* substitute regular expression */
	redelim = *cp++;	/* get delimiter from 1st ch */
	if ((fp = recomp(cmdp->u.lhs = fp, redelim)) == BAD) ABORT(CGMSG);
	if (fp == cmdp->u.lhs)	/* if compiled RE zero len */
		cmdp->u.lhs = lastre;	/* use the previous one */
	else			/* otherwise */
		lastre = cmdp->u.lhs;	/* save the one just found */
	if ((cmdp->rhs = fp) > poolend) ABORT(TMTXT);
	if ((fp = rhscomp(cmdp->rhs, redelim)) == BAD) ABORT(CGMSG);
	if (gflag) cmdp->flags.global ++;
	while (*cp == 'g' || *cp == 'p' || *cp == 'P') {
		IFEQ(cp, 'g') cmdp->flags.global ++;
		IFEQ(cp, 'p') cmdp->flags.print = 1;
		IFEQ(cp, 'P') cmdp->flags.print = 2;
	}

      case 'l':			/* list pattern space */
	if (*cp == 'w')
		cp++;		/* and execute a w command! */
	else
		break;		/* s or l is done */

      case 'w':			/* write-pattern-space command */
      case 'W':			/* write-first-line command */
	if (nwfiles >= WFILES) ABORT(TMWFI);
	fp = gettext(fname[nwfiles] = fp);	/* filename will be in pool */
	for (i = nwfiles - 1; i >= 0; i--)	/* match it in table */
		if ((fname[i] != NULL) &&
		    (strcmp(fname[nwfiles], fname[i]) == 0)) {
			cmdp->fout = fout[i];
			return(0);
		}

	/* If didn't find one, open new out file */
	if ((cmdp->fout = fopen(fname[nwfiles], "w")) == NULL) {
		fprintf(stderr, CCOFI, fname[nwfiles]);
		quit(2);
	}
	fout[nwfiles++] = cmdp->fout;
	break;

      case 'y':			/* transliterate text */
	fp = ycomp(cmdp->u.lhs = fp, *cp++);	/* compile translit */
	if (fp == BAD) ABORT(CGMSG);	/* fail on bad form */
	if (fp > poolend) ABORT(TMTXT);	/* fail on overflow */
	break;
  }
  return(0);			/* succeeded in interpreting one command */
}

static char *rhscomp(char *rhsp,  int delim)	/* uses bcount */
 /* Generate replacement string for substitute command right hand side */
/* rhsp: place to compile expression to */
/* delim: regular-expression end-mark to look for */
{
  register char *p = cp;	/* strictly for speed */

  for (;;)
	if ((*rhsp = *p++) == '\\') {	/* copy; if it's a \, */
		*rhsp = *p++;	/* copy escaped char */
		/* Check validity of pattern tag */
		if (*rhsp > bcount + '0' && *rhsp <= '9') return(BAD);
		*rhsp++ |= 0x80;/* mark the good ones */
		continue;
	} else if (*rhsp == delim) {	/* found RE end, hooray... */
		*rhsp++ = '\0';	/* cap the expression string */
		cp = p;
		return(rhsp);	/* pt at 1 past the RE */
	} else if (*rhsp++ == '\0')	/* last ch not RE end, help! */
		return(BAD);
}

static char *recomp(char *expbuf, int redelim)	/* uses cp, bcount */
/* Compile a regular expression to internal form */
/* expbuf: place to compile it to */
/* redelim: RE end-marker to look for */
{
  register char *ep = expbuf;	/* current-compiled-char pointer */
  register char *sp = cp;	/* source-character ptr */
  register int c;		/* current-character pointer */
  char negclass;		/* all-but flag */
  char *lastep;			/* ptr to last expr compiled */
  char *svclass;		/* start of current char class */
  char brnest[MAXTAGS];		/* bracket-nesting array */
  char *brnestp;		/* ptr to current bracket-nest */
  int classct;			/* class element count */
  int tags;			/* # of closed tags */

  if (*cp == redelim)		/* if first char is RE endmarker */
	return(cp++, expbuf);	/* leave existing RE unchanged */

  lastep = NULL;		/* there's no previous RE */
  brnestp = brnest;		/* initialize ptr to brnest array */
  tags = bcount = 0;		/* initialize counters */

  if (*ep++ = (*sp == '^'))	/* check for start-of-line syntax */
	sp++;

  for (;;) {
	if (ep >= expbuf + RELIMIT)	/* match is too large */
		return(cp = sp, BAD);
	if ((c = *sp++) == redelim) {	/* found the end of the RE */
		cp = sp;
		if (brnestp != brnest)	/* \(, \) unbalanced */
			return(BAD);
		*ep++ = CEOF;	/* write end-of-pattern mark */
		return(ep);	/* return ptr to compiled RE */
	}
	if (c != '*')		/* if we're a postfix op */
		lastep = ep;	/* get ready to match last */

	switch (c) {
	    case '\\':
		if ((c = *sp++) == '(') {	/* start tagged section */
			if (bcount >= MAXTAGS) return(cp = sp, BAD);
			*brnestp++ = bcount;	/* update tag stack */
			*ep++ = CBRA;	/* enter tag-start */
			*ep++ = bcount++;	/* bump tag count */
			continue;
		} else if (c == ')') {	/* end tagged section */
			if (brnestp <= brnest)	/* extra \) */
				return(cp = sp, BAD);
			*ep++ = CKET;	/* enter end-of-tag */
			*ep++ = *--brnestp;	/* pop tag stack */
			tags++;	/* count closed tags */
			continue;
		} else if (c >= '1' && c <= '9') {	/* tag use */
			if ((c -= '1') >= tags)	/* too few */
				return(BAD);
			*ep++ = CBACK;	/* enter tag mark */
			*ep++ = c;	/* and the number */
			continue;
		} else if (c == '\n')	/* escaped newline no good */
			return(cp = sp, BAD);
		else if (c == 'n')	/* match a newline */
			c = '\n';
		else if (c == 't')	/* match a tab */
			c = '\t';
		else if (c == 'r')	/* match a return */
			c = '\r';
		goto defchar;

	    case '\0':		/* ignore nuls */
		continue;

	    case '\n':		/* trailing pattern delimiter is missing */
		return(cp = sp, BAD);

	    case '.':		/* match any char except newline */
		*ep++ = CDOT;
		continue;
	    case '*':		/* 0..n repeats of previous pattern */
		if (lastep == NULL)	/* if * isn't first on line */
			goto defchar;	/* match a literal * */
		if (*lastep == CKET)	/* can't iterate a tag */
			return(cp = sp, BAD);
		*lastep |= STAR;/* flag previous pattern */
		continue;

	    case '$':		/* match only end-of-line */
		if (*sp != redelim)	/* if we're not at end of RE */
			goto defchar;	/* match a literal $ */
		*ep++ = CDOL;	/* insert end-symbol mark */
		continue;

	    case '[':		/* begin character set pattern */
		if (ep + 17 >= expbuf + RELIMIT) ABORT(REITL);
		*ep++ = CCL;	/* insert class mark */
		if (negclass = ((c = *sp++) == '^')) c = *sp++;
		svclass = sp;	/* save ptr to class start */
		do {
			if (c == '\0') ABORT(CGMSG);

			/* Handle character ranges */
			if (c == '-' && sp > svclass && *sp != ']')
				for (c = sp[-2]; c < *sp; c++)
					ep[c >> 3] |= bits[c & 7];

			/* Handle escape sequences in sets */
			if (c == '\\')
				if ((c = *sp++) == 'n')
					c = '\n';
				else if (c == 't')
					c = '\t';
				else if (c == 'r')
					c = '\r';

			/* Enter (possibly translated) char in set */
			ep[c >> 3] |= bits[c & 7];
		} while
			((c = *sp++) != ']');

		/* Invert the bitmask if all-but was specified */
		if (negclass) for (classct = 0; classct < 16; classct++)
				ep[classct] ^= 0xFF;
		ep[0] &= 0xFE;	/* never match ASCII 0 */
		ep += 16;	/* advance ep past set mask */
		continue;

  defchar:			/* match literal character */
	    default:		/* which is what we'd do by default */
		*ep++ = CCHR;	/* insert character mark */
		*ep++ = c;
	}
  }
}

static int cmdline(char *cbuf)	/* uses eflag, eargc, cmdf */
 /* Read next command from -e argument or command file */
{
  register int inc;		/* not char because must hold EOF */

  *cbuf-- = 0;			/* so pre-increment points us at cbuf */

  /* E command flag is on */
  if (eflag) {
	register char *p;	/* ptr to current -e argument */
	static char *savep;	/* saves previous value of p */

	if (eflag > 0) {	/* there are pending -e arguments */
		eflag = -1;
		if (eargc-- <= 0) quit(2);	/* if no arguments, barf */

		/* Else transcribe next e argument into cbuf */
		p = *++eargv;
		while (*++cbuf = *p++)
			if (*cbuf == '\\') {
				if ((*++cbuf = *p++) == '\0')
					return(savep = NULL, -1);
				else
					continue;
			} else if (*cbuf == '\n') {	/* end of 1 cmd line */
				*cbuf = '\0';
				return(savep = p, 1);
				/* We'll be back for the rest... */
			}

		/* Found end-of-string; can advance to next argument */
		return(savep = NULL, 1);
	}
	if ((p = savep) == NULL) return(-1);

	while (*++cbuf = *p++)
		if (*cbuf == '\\') {
			if ((*++cbuf = *p++) == '0')
				return(savep = NULL, -1);
			else
				continue;
		} else if (*cbuf == '\n') {
			*cbuf = '\0';
			return(savep = p, 1);
		}
	return(savep = NULL, 1);
  }

  /* If no -e flag read from command file descriptor */
  while ((inc = getc(cmdf)) != EOF)	/* get next char */
	if ((*++cbuf = inc) == '\\')	/* if it's escape */
		*++cbuf = inc = getc(cmdf);	/* get next char */
	else if (*cbuf == '\n')	/* end on newline */
		return(*cbuf = '\0', 1);	/* cap the string */

  return(*++cbuf = '\0', -1);	/* end-of-file, no more chars */
}

static char *address(char *expbuf)	/* uses cp, linenum */
 /* Expand an address at *cp... into expbuf, return ptr at following char */
{
  static int numl = 0;		/* current ind in addr-number table */
  register char *rcp;		/* temp compile ptr for forwd look */
  long lno;			/* computed value of numeric address */

  if (*cp == '$') {		/* end-of-source address */
	*expbuf++ = CEND;	/* write symbolic end address */
	*expbuf++ = CEOF;	/* and the end-of-address mark (!) */
	cp++;			/* go to next source character */
	return(expbuf);	/* we're done */
  }
  if (*cp == '/' || *cp == '\\') { /* start of regular-expression match */
	if (*cp == '\\') cp++;
	return(recomp(expbuf, *cp++));	/* compile the RE */
  }

  rcp = cp;
  lno = 0;			/* now handle a numeric address */
  while (*rcp >= '0' && *rcp <= '9')	/* collect digits */
	lno = lno * 10 + *rcp++ - '0';	/* compute their value */

  if (rcp > cp) {		/* if we caught a number... */
	*expbuf++ = CLNUM;	/* put a numeric-address marker */
	*expbuf++ = numl;	/* and the address table index */
	linenum[numl++] = lno;	/* and set the table entry */
	if (numl >= MAXLINES)	/* oh-oh, address table overflow */
		ABORT(TMLNR);	/* abort with error message */
	*expbuf++ = CEOF;	/* write the end-of-address marker */
	cp = rcp;		/* point compile past the address */
	return(expbuf);	/* we're done */
  }
  return(NULL);			/* no legal address was found */
}

static char *gettext(char *txp)	/* uses global cp */
 /* Accept multiline input from *cp..., discarding leading whitespace */
{
  register char *p = cp;	/* this is for speed */

  SKIPWS(p);			/* discard whitespace */
  do {
	if ((*txp = *p++) == '\\')	/* handle escapes */
		*txp = *p++;
	if (*txp == '\0')	/* we're at end of input */
		return(cp = --p, ++txp);
	else if (*txp == '\n')	/* also SKIPWS after newline */
		SKIPWS(p);
  } while
	(txp++);		/* keep going till we find that nul */
  return(txp);
}

static label *search(label *ptr)	/* uses global lablst */
 /* Find the label matching *ptr, return NULL if none */
{
  register label *rp;
  for (rp = lablst; rp < ptr; rp++)
	if ((rp->name != NULL) && (strcmp(rp->name, ptr->name) == 0))
		return(rp);
  return(NULL);
}

static void resolve(void)
{				/* uses global lablst */
  /* Write label links into the compiled-command space */
  register label *lptr;
  register sedcmd *rptr, *trptr;

  /* Loop through the label table */
  for (lptr = lablst; lptr < lab; lptr++)
	if (lptr->address == NULL) {	/* barf if not defined */
		fprintf(stderr, ULABL, lptr->name);
		quit(2);
	} else if (lptr->last) {/* if last is non-null */
		rptr = lptr->last;	/* chase it */
		while (trptr = rptr->u.link) {	/* resolve refs */
			rptr->u.link = lptr->address;
			rptr = trptr;
		}
		rptr->u.link = lptr->address;
	}
}

static char *ycomp(char *ep, char delim)
/* Compile a y (transliterate) command */
/* ep: where to compile to */
/* delim: end delimiter to look for */
{
  register char *tp, *sp;
  register int c;

  /* Scan the 'from' section for invalid chars */
  for (sp = tp = cp; *tp != delim; tp++) {
	if (*tp == '\\') tp++;
	if ((*tp == '\n') || (*tp == '\0')) return (BAD);
  }
  tp++;				/* tp now points at first char of 'to'
			 * section */

  /* Now rescan the 'from' section */
  while ((c = *sp++ & 0x7F) != delim) {
	if (c == '\\' && *sp == 'n') {
		sp++;
		c = '\n';
	}
	if ((ep[c] = *tp++) == '\\' && *tp == 'n') {
		ep[c] = '\n';
		tp++;
	}
	if ((ep[c] == delim) || (ep[c] == '\0')) return(BAD);
  }

  if (*tp != delim)		/* 'to', 'from' parts have unequal lengths */
	return(BAD);

  cp = ++tp;			/* point compile ptr past translit */

  for (c = 0; c < 128; c++)	/* fill in self-map entries in table */
	if (ep[c] == 0) ep[c] = c;

  return(ep + 0x80);		/* return first free location past table end */
}

/*
void quit(int n)
{
/ * Flush buffers and exit.  Now a historical relic.  Rely on exit to flush
 * the buffers.
 * /
  exit(n);
}
*/

int main(int argc, char *argv[])
/* Main sequence of the stream editor */
{
  eargc = argc;			/* set local copy of argument count */
  eargv = argv;			/* set local copy of argument list */
  cmdp->addr1 = pool;		/* 1st addr expand will be at pool start */
  if (eargc == 1) quit(0);	/* exit immediately if no arguments */
  /* Scan through the arguments, interpreting each one */
  while ((--eargc > 0) && (**++eargv == '-')) switch (eargv[0][1]) {
	    case 'e':
		eflag++;
		compile();	/* compile with e flag on */
		eflag = 0;
		continue;	/* get another argument */
	    case 'f':
		if (eargc-- <= 0)	/* barf if no -f file */
			quit(2);
		if ((cmdf = fopen(*++eargv, "r")) == NULL) {
			fprintf(stderr, COCFI, *eargv);
			quit(2);
		}
		compile();	/* file is O.K., compile it */
		fclose(cmdf);
		continue;	/* go back for another argument */
	    case 'g':
		gflag++;	/* set global flag on all s cmds */
		continue;
	    case 'n':
		nflag++;	/* no print except on p flag or w */
		continue;
	    default:
		fprintf(stdout, UFLAG, eargv[0][1]);
		continue;
	}


  if (cmdp == cmds) {		/* no commands have been compiled */
	eargv--;
	eargc++;
	eflag++;
	compile();
	eflag = 0;
	eargv++;
	eargc--;
  }
  if (bdepth)			/* we have unbalanced squigglies */
	ABORT(TMLBR);

  lablst->address = cmdp;	/* set up header of label linked list */
  resolve();			/* resolve label table indirections */
  execute();			/* execute commands */
  quit(0);			/* everything was O.K. if we got here */
  return(0);
}


