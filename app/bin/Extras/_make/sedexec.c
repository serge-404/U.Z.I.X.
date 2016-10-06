/*+++++++++++++++*/

/*
   sedexec.c -- execute compiled form of stream editor commands

   The single entry point of this module is the function execute(). It
   may take a string argument (the name of a file to be used as text)  or
   the argument NULL which tells it to filter standard input. It executes
   the compiled commands in cmds[] on each line in turn.

   The function command() does most of the work. Match() and advance()
   are used for matching text against precompiled regular expressions and
   dosub() does right-hand-side substitution.  Getline() does text input;
   readout() and Memcmp() are output and string-comparison utilities.
*/

#include "stdio.h"
#include "ctype.h"
#include "sed.h"	

extern char linebuf[];		/* current-line buffer */
extern sedcmd cmds[];		/* hold compiled commands */
extern long linenum[];		/* numeric-addresses table */

/* Miscellaneous shared variables */
extern int nflag;			/* -n option flag */
extern int eargc;			/* scratch copy of argument count */
extern char **eargv;			/* scratch copy of argument list */
extern char bits[];


static char LTLMSG[] = "sed: line too long\n";

static char *spend;		/* current end-of-line-buffer pointer */
static long lnum = 0L;		/* current source line number */

/* Append buffer maintenance */
static sedcmd *appends[MAXAPPENDS];	/* array of ptrs to a,i,c commands */
static sedcmd **aptr = appends;	/* ptr to current append */

/* Genbuf and its pointers */
static char genbuf[GENSIZ];
static char *loc1;
static char *loc2;
static char *locs;

/* Command-logic flags */
static int lastline;		/* do-line flag */
static int jump;		/* jump to cmd's link address if set */
static int delete;		/* delete command flag */

/* Tagged-pattern tracking */
static char *bracend[MAXTAGS];	/* tagged pattern start pointers */
static char *brastart[MAXTAGS];	/* tagged pattern end pointers */

static int anysub;		/* true if any s on current line succeeded */


void execute(void)
/* Execute the compiled commands in cmds[] */
{
  register char *p1;		/* dummy copy ptrs */
  register sedcmd *ipc;		/* ptr to current command */
  char *execp;			/* ptr to source */


  initget();

  /* Here's the main command-execution loop */
  for (;;) {

	/* Get next line to filter */
	if ((execp = getline(linebuf)) == BAD) return;
	spend = execp;
	anysub = FALSE;

	/* Loop through compiled commands, executing them */
	for (ipc = cmds; ipc->command;) {
		if (!selected(ipc)) {
			ipc++;
			continue;
		}
		command(ipc);	/* execute the command pointed at */

		if (delete)	/* if delete flag is set */
			break;	/* don't exec rest of compiled cmds */

		if (jump) {	/* if jump set, follow cmd's link */
			jump = FALSE;
			if ((ipc = ipc->u.link) == 0) {
				ipc = cmds;
				break;
			}
		} else		/* normal goto next command */
			ipc++;
	}

	/* We've now done all modification commands on the line */

	/* Here's where the transformed line is output */
	if (!nflag && !delete) {
		for (p1 = linebuf; p1 < spend; p1++) putc(*p1, stdout);
		putc('\n', stdout);
	}

	/* If we've been set up for append, emit the text from it */
	if (aptr > appends) readout();

	delete = FALSE;		/* clear delete flag; about to get next cmd */
  }
}

static int selected(sedcmd *ipc)
/* Is current command selected */
{
  register char *p1 = ipc->addr1;	/* point p1 at first address */
  register char *p2 = ipc->addr2;	/* and p2 at second */
  int c;
  int sel = TRUE;		/* select by default */

  if (!p1)			/* No addresses: always selected */
	;
  else if (ipc->flags.inrange) {
	if (*p2 == CEND);
	else if (*p2 == CLNUM) {
		c = p2[1] & CMASK;
		if (lnum >= linenum[c]) {
			ipc->flags.inrange = FALSE;
			if (lnum > linenum[c]) sel = FALSE;
		}
	} else if (match(p2, 0))
		ipc->flags.inrange = FALSE;
  } else if (*p1 == CEND) {
	if (!lastline) sel = FALSE;
  } else if (*p1 == CLNUM) {
	c = p1[1] & CMASK;
	if (lnum != linenum[c])
		sel = FALSE;
	else if (p2)
		ipc->flags.inrange = TRUE;
  } else if (match(p1, 0)) {
	if (p2) ipc->flags.inrange = TRUE;
  } else
	sel = FALSE;

  return ipc->flags.allbut ? !sel : sel;
}

static int match(char *expbuf, int gf)	/* uses genbuf */
 /* Match RE at expbuf against linebuf; if gf set, copy linebuf from genbuf */
{
  register char *p1, *p2, c;

  if (gf) {
	if (*expbuf) return(FALSE);
	p1 = linebuf;
	p2 = genbuf;
	while (*p1++ = *p2++);
	locs = p1 = loc2;
  } else {
	p1 = linebuf;
	locs = FALSE;
  }

  p2 = expbuf;
  if (*p2++) {
	loc1 = p1;
	if (*p2 == CCHR && p2[1] != *p1)	/* 1st char is wrong */
		return(FALSE);	/* so fail */
	return(advance(p1, p2));/* else try to match rest */
  }

  /* Quick check for 1st character if it's literal */
  if (*p2 == CCHR) {
	c = p2[1];		/* pull out character to search for */
	do {
		if (*p1 != c) continue;	/* scan the source string */
		if (advance(p1, p2))	/* found it, match the rest */
			return(loc1 = p1, 1);
	} while
		(*p1++);
	return(FALSE);		/* didn't find that first char */
  }

  /* Else try for unanchored match of the pattern */
  do {
	if (advance(p1, p2)) return(loc1 = p1, 1);
  } while
	(*p1++);

  /* If got here, didn't match either way */
  return(FALSE);
}

static int advance(char *lp, char *ep)
/* Attempt to advance match pointer by one pattern element */
/* lp: source (linebuf) ptr */
/* ep: regular expression element ptr */
{
  register char *curlp;		/* save ptr for closures */
  char c;			/* scratch character holder */
  char *bbeg;
  int ct;

  for (;;) switch (*ep++) {
	    case CCHR:		/* literal character */
		if (*ep++ == *lp++)	/* if chars are equal */
			continue;	/* matched */
		return(FALSE);	/* else return false */

	    case CDOT:		/* anything but newline */
		if (*lp++)	/* first NUL is at EOL */
			continue;	/* keep going if didn't find */
		return(FALSE);	/* else return false */

	    case CNL:		/* start-of-line */
	    case CDOL:		/* end-of-line */
		if (*lp == 0)	/* found that first NUL? */
			continue;	/* yes, keep going */
		return(FALSE);	/* else return false */

	    case CEOF:		/* end-of-address mark */
		loc2 = lp;	/* set second loc */
		return(TRUE);	/* return true */

	    case CCL:		/* a closure */
		c = *lp++ & 0177;
		if (ep[c >> 3] & bits[c & 07]) {	/* is char in set? */
			ep += 16;	/* then skip rest of bitmask */
			continue;	/* and keep going */
		}
		return(FALSE);	/* else return false */

	    case CBRA:		/* start of tagged pattern */
		brastart[*ep++] = lp;	/* mark it */
		continue;	/* and go */

	    case CKET:		/* end of tagged pattern */
		bracend[*ep++] = lp;	/* mark it */
		continue;	/* and go */

	    case CBACK:
		bbeg = brastart[*ep];
		ct = bracend[*ep++] - bbeg;

		if (Memcmp(bbeg, lp, ct)) {
			lp += ct;
			continue;
		}
		return(FALSE);

	    case CBACK | STAR:
		bbeg = brastart[*ep];
		ct = bracend[*ep++] - bbeg;
		curlp = lp;
		while (Memcmp(bbeg, lp, ct)) lp += ct;

		while (lp >= curlp) {
			if (advance(lp, ep)) return(TRUE);
			lp -= ct;
		}
		return(FALSE);


	    case CDOT | STAR:	/* match .* */
		curlp = lp;	/* save closure start loc */
		while (*lp++);	/* match anything */
		goto star;	/* now look for followers */

	    case CCHR | STAR:	/* match <literal char>* */
		curlp = lp;	/* save closure start loc */
		while (*lp++ == *ep);	/* match many of that char */
		ep++;		/* to start of next element */
		goto star;	/* match it and followers */

	    case CCL | STAR:	/* match [...]* */
		curlp = lp;	/* save closure start loc */
		do {
			c = *lp++ & 0x7F;	/* match any in set */
		} while
			(ep[c >> 3] & bits[c & 07]);
		ep += 16;	/* skip past the set */
		goto star;	/* match followers */

  star:				/* the recursion part of a * or + match */
		if (--lp == curlp)	/* 0 matches */
			continue;

		if (*ep == CCHR) {
			c = ep[1];
			do {
				if (*lp != c) continue;
				if (advance(lp, ep)) return (TRUE);
			} while
				(lp-- > curlp);
			return(FALSE);
		}
		if (*ep == CBACK) {
			c = *(brastart[ep[1]]);
			do {
				if (*lp != c) continue;
				if (advance(lp, ep)) return (TRUE);
			} while
				(lp-- > curlp);
			return(FALSE);
		}
		do {
			if (lp == locs) break;
			if (advance(lp, ep)) return (TRUE);
		} while
			(lp-- > curlp);
		return(FALSE);

	    default:
		fprintf(stderr, "sed: RE error, %o\n", *--ep);
		quit(2);
	}
}

static int substitute(sedcmd *ipc)
/* Perform s command */
/* ipc: ptr to s command struct */
{
  int nullmatch;

  if (match(ipc->u.lhs, 0)) {	/* if no match */
	nullmatch = (loc1 == loc2);
	dosub(ipc->rhs);	/* perform it once */
  } else
	return(FALSE);		/* command fails */

  if (ipc->flags.global)	/* if global flag enabled */
	while (*loc2) {		/* cycle through possibles */
		if (nullmatch) loc2++;
		if (match(ipc->u.lhs, 1)) {	/* found another */
			nullmatch = (loc1 == loc2);
			dosub(ipc->rhs);	/* so substitute */
		} else		/* otherwise, */
			break;	/* we're done */
	}
  return(TRUE);			/* we succeeded */
}

static void dosub(char *rhsbuf)	/* uses linebuf, genbuf, spend */
/* Generate substituted right-hand side (of s command) */
/* rhsbuf: where to put the result */
{
  register char *lp, *sp, *rp;
  int c;

  /* Copy linebuf to genbuf up to location  1 */
  lp = linebuf;
  sp = genbuf;
  while (lp < loc1) *sp++ = *lp++;

  for (rp = rhsbuf; c = *rp++;) {
	if (c == '&') {
		sp = place(sp, loc1, loc2);
		continue;
	} else if (c & 0200 && (c &= 0177) >= '1' && c < MAXTAGS + '1') {
		sp = place(sp, brastart[c - '1'], bracend[c - '1']);
		continue;
	}
	*sp++ = c & 0177;
	if (sp >= genbuf + MAXBUF) fprintf(stderr, LTLMSG);
  }
  lp = loc2;
  loc2 = sp - genbuf + linebuf;
  while (*sp++ = *lp++)
	if (sp >= genbuf + MAXBUF) fprintf(stderr, LTLMSG);
  lp = linebuf;
  sp = genbuf;
  while (*lp++ = *sp++);
  spend = lp - 1;
}

static char *place(char *asp, char *al1, char *al2)	/* uses genbuf */
 /* Place chars at *al1...*(al1 - 1) at asp... in genbuf[] */
{
  while (al1 < al2) {
	*asp++ = *al1++;
	if (asp >= genbuf + MAXBUF) fprintf(stderr, LTLMSG);
  }
  return(asp);
}

static void listto(char *p1, FILE *fp)
/* Write a hex dump expansion of *p1... to fp */
/* p1: the source */
/* fp: output stream to write to */
{
  p1--;
  while (*p1++)
	if (isprint(*p1))
		putc(*p1, fp);	/* pass it through */
	else {
		putc('\\', fp);	/* emit a backslash */
		switch (*p1) {
		    case '\b':
			putc('b', fp);
			break;	/* BS */
		    case '\t':
			putc('t', fp);
			break;	/* TAB */
		    case '\n':
			putc('n', fp);
			break;	/* NL */
		    case '\r':
			putc('r', fp);
			break;	/* CR */
		    case '\33':
			putc('e', fp);
			break;	/* ESC */
		    default:
			fprintf(fp, "%02x", *p1 & 0xFF);
		}
	}
  putc('\n', fp);
}

static void truncated(int h)
{
  static long last = 0L;

  if (lnum == last) return;
  last = lnum;

  fprintf(stderr, "sed: ");
  fprintf(stderr, h ? "hold space" : "line %ld", lnum);
  fprintf(stderr, " truncated to %d characters\n", MAXBUF);
}

static void command(sedcmd *ipc)
/* Execute compiled command pointed at by ipc */
{
  static char holdsp[MAXHOLD + 1];	/* the hold space */
  static char *hspend = holdsp;	/* hold space end pointer */
  register char *p1, *p2;
  char *execp;
  int didsub;			/* true if last s succeeded */

  switch (ipc->command) {
      case ACMD:		/* append */
	*aptr++ = ipc;
	if (aptr >= appends + MAXAPPENDS) fprintf(stderr,
			"sed: too many appends after line %ld\n",
			lnum);
	*aptr = 0;
	break;

      case CCMD:		/* change pattern space */
	delete = TRUE;
	if (!ipc->flags.inrange || lastline) printf("%s\n", ipc->u.lhs);
	break;

      case DCMD:		/* delete pattern space */
	delete++;
	break;

      case CDCMD:		/* delete a line in hold space */
	p1 = p2 = linebuf;
	while (*p1 != '\n')
		if (delete = (*p1++ == 0)) return;
	p1++;
	while (*p2++ = *p1++) continue;
	spend = p2 - 1;
	jump++;
	break;

      case EQCMD:		/* show current line number */
	fprintf(stdout, "%ld\n", lnum);
	break;

      case GCMD:		/* copy hold space to pattern space */
	p1 = linebuf;
	p2 = holdsp;
	while (*p1++ = *p2++);
	spend = p1 - 1;
	break;

      case CGCMD:		/* append hold space to pattern space */
	*spend++ = '\n';
	p1 = spend;
	p2 = holdsp;
	do
		if (p1 > linebuf + MAXBUF) {
			truncated(0);
			p1[-1] = 0;
			break;
		}
	while (*p1++ = *p2++);

	spend = p1 - 1;
	break;

      case HCMD:		/* copy pattern space to hold space */
	p1 = holdsp;
	p2 = linebuf;
	while (*p1++ = *p2++);
	hspend = p1 - 1;
	break;

      case CHCMD:		/* append pattern space to hold space */
	*hspend++ = '\n';
	p1 = hspend;
	p2 = linebuf;
	do
		if (p1 > holdsp + MAXBUF) {
			truncated(1);
			p1[-1] = 0;
			break;
		}
	while (*p1++ = *p2++);

	hspend = p1 - 1;
	break;

      case ICMD:		/* insert text */
	printf("%s\n", ipc->u.lhs);
	break;

      case BCMD:		/* branch to label */
	jump = TRUE;
	break;

      case LCMD:		/* list text */
	listto(linebuf, (ipc->fout != NULL) ? ipc->fout : stdout);
	break;

      case NCMD:		/* read next line into pattern space */
	if (!nflag) puts(linebuf);	/* flush out the current line */
	if (aptr > appends) readout();	/* do pending a, r commands */
	if ((execp = getline(linebuf)) == BAD) {
		delete = TRUE;
		break;
	}
	spend = execp;
	anysub = FALSE;
	break;

      case CNCMD:		/* append next line to pattern space */
	if (aptr > appends) readout();
	*spend++ = '\n';
	if ((execp = getline(spend)) == BAD) {
		*--spend = 0;
		break;
	}
	spend = execp;
	anysub = FALSE;
	break;

      case PCMD:		/* print pattern space */
	puts(linebuf);
	break;

      case CPCMD:		/* print one line from pattern space */
cpcom:				/* so s command can jump here */
	for (p1 = linebuf; *p1 != '\n' && *p1 != '\0';) putc(*p1++, stdout);
	putc('\n', stdout);
	break;

      case QCMD:		/* quit the stream editor */
	if (!nflag) puts(linebuf);	/* flush out the current line */
	if (aptr > appends)
		readout();	/* do any pending a and r commands */
	quit(0);

      case RCMD:		/* read a file into the stream */
	*aptr++ = ipc;
	if (aptr >= appends + MAXAPPENDS) fprintf(stderr,
			"sed: too many reads after line %ld\n",
			lnum);
	*aptr = 0;
	break;

      case SCMD:		/* substitute RE */
	didsub = substitute(ipc);
	if (didsub) anysub = TRUE;
	if (ipc->flags.print && didsub)
		if (ipc->flags.print == TRUE)
			puts(linebuf);
		else
			goto cpcom;
	if (didsub && ipc->fout) fprintf(ipc->fout, "%s\n", linebuf);
	break;

      case TCMD:		/* branch on any s successful */
      case CTCMD:		/* branch on any s failed */
	if (anysub == (ipc->command == CTCMD))
		break;		/* no branch if any s failed, else */
	anysub = FALSE;
	jump = TRUE;		/* set up to jump to assoc'd label */
	break;

      case CWCMD:		/* write one line from pattern space */
	for (p1 = linebuf; *p1 != '\n' && *p1 != '\0';)
		putc(*p1++, ipc->fout);
	putc('\n', ipc->fout);
	break;

      case WCMD:		/* write pattern space to file */
	fprintf(ipc->fout, "%s\n", linebuf);
	break;

      case XCMD:		/* exchange pattern and hold spaces */
	p1 = linebuf;
	p2 = genbuf;
	while (*p2++ = *p1++) continue;
	p1 = holdsp;
	p2 = linebuf;
	while (*p2++ = *p1++) continue;
	spend = p2 - 1;
	p1 = genbuf;
	p2 = holdsp;
	while (*p2++ = *p1++) continue;
	hspend = p2 - 1;
	break;

      case YCMD:
	p1 = linebuf;
	p2 = ipc->u.lhs;
	while (*p1 = p2[*p1]) p1++;
	break;
  }
}

static void openfile(char *file)
/* Replace stdin by given file */
{
  if (freopen(file, "r", stdin) == NULL) {
	fprintf(stderr, "sed: can't open %s\n", file);
	quit(1);
  }
}

static int c;			/* Will be the next char to read, a kind of
			 * lookahead */

static void get(void)
/* Read next character into c treating all argument files as run through cat */
{
  while ((c = getchar()) == EOF && --eargc >= 0) openfile(*eargv++);
}

static void initget(void)
/* Initialise character input */
{
  if (--eargc >= 0) openfile(*eargv++);	/* else input == stdin */
  get();
}

static char *getline(char *buf)
/* Get next line of text to be edited, return pointer to end */
/* buf: where to send the input */
{
  if (c == EOF) return BAD;

  lnum++;			/* we can read a new line */

  do {
	if (c == '\n') {
		get();
		break;
	}
	if (buf <= linebuf + MAXBUF) *buf++ = c;
	get();
  } while (c != EOF);

  if (c == EOF) lastline = TRUE;

  if (buf > linebuf + MAXBUF) {
	truncated(0);
	--buf;
  }
  *buf = 0;
  return buf;
}

static int Memcmp(char *a, char *b, int count)
/* Return TRUE if *a... == *b... for count chars, FALSE otherwise */
{
  while (count--)		/* look at count characters */
	if (*a++ != *b++)	/* if any are nonequal	 */
		return(FALSE);	/* return FALSE for false */
  return(TRUE);			/* compare succeeded */
}

static void readout(void)
/* Write file indicated by r command to output */
{
  register int t;		/* hold input char or EOF */
  FILE *fi;			/* ptr to file to be read */

  aptr = appends - 1;		/* arrange for pre-increment to work right */
  while (*++aptr)
	if ((*aptr)->command == ACMD)	/* process "a" cmd */
		printf("%s\n", (*aptr)->u.lhs);
	else {			/* process "r" cmd */
		if ((fi = fopen((*aptr)->u.lhs, "r")) == NULL) {
			fprintf(stderr, "sed: can't open %s\n",
				(*aptr)->u.lhs);
			continue;
		}
		while ((t = getc(fi)) != EOF) putc((char) t, stdout);
		fclose(fi);
	}
  aptr = appends;		/* reset the append ptr */
  *aptr = 0;
}

/* Sedexec.c ends here */
