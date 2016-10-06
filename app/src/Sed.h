/*+++++++++++++++*/
#ifndef	_SED_H
#define _SED_H

/* Sed.h -- types and constants for the stream editor */

/* Data area sizes used by both modules */
#define MAXBUF		4000	/* current line buffer size */
#define MAXAPPENDS	20	/* maximum number of appends */
#define MAXTAGS		9	/* tagged patterns are \1 to \9 */

/***** public stuff ******/

#define MAXCMDS		500	/* maximum number of compiled commands */
#define MAXLINES	256	/* max # numeric addresses to compile */

/***** module common stuff *****/

#define POOLSIZE	20000	/* size of string-pool space */
#define WFILES		10	/* max # w output files that can be compiled */
#define RELIMIT		256	/* max chars in compiled RE */
#define MAXDEPTH	20	/* maximum {}-nesting level */
#define MAXLABS		50	/* max # of labels that can be handled */

#define SKIPWS(pc)	while ((*pc==' ') || (*pc=='\t')) pc++
#define ABORT(msg)	(fprintf(stderr, msg, linebuf), quit(2))
#define IFEQ(x, v)	if (*x == v) x++ ,	/* do expression */

/*****  *****/

#define MAXHOLD	 MAXBUF		/* size of the hold space */
#define GENSIZ	 MAXBUF		/* maximum genbuf size */

#define TRUE	 1
#define FALSE	 0

#ifndef CMASK
#define CMASK  0xFF		/* some char type should have been unsigned char? */
#endif

/* Constants for compiled-command representation */
#define EQCMD	0x01		/* = -- print current line number	 */
#define ACMD	0x02		/* a -- append text after current line	 */
#define BCMD	0x03		/* b -- branch to label			 */
#define CCMD	0x04		/* c -- change current line		 */
#define DCMD	0x05		/* d -- delete all of pattern space */
#define CDCMD	0x06		/* D -- delete first line of pattern space */
#define GCMD	0x07		/* g -- copy hold space to pattern space */
#define CGCMD	0x08		/* G -- append hold space to pattern space */
#define HCMD	0x09		/* h -- copy pattern space to hold space */
#define CHCMD	0x0A		/* H -- append pattern space to hold space */
#define ICMD	0x0B		/* i -- insert text before current line	 */
#define LCMD	0x0C		/* l -- print pattern space in escaped form */
#define NCMD	0x0D		/* n -- get next line into pattern space */
#define CNCMD	0x0E		/* N -- append next line to pattern space */
#define PCMD	0x0F		/* p -- print pattern space to output	 */
#define CPCMD	0x10		/* P -- print first line of pattern space */
#define QCMD	0x11		/* q -- exit the stream editor		 */
#define RCMD	0x12		/* r -- read in a file after current line */
#define SCMD	0x13		/* s -- regular-expression substitute	 */
#define TCMD	0x14		/* t -- branch on any substitute successful */
#define CTCMD	0x15		/* T -- branch on any substitute failed	 */
#define WCMD	0x16		/* w -- write pattern space to file	 */
#define CWCMD	0x17		/* W -- write first line of pattern space */
#define XCMD	0x18		/* x -- exhange pattern and hold spaces	 */
#define YCMD	0x19		/* y -- transliterate text		 */

struct cmd_t {			/* compiled-command representation */
  char *addr1;			/* first address for command */
  char *addr2;			/* second address for command */
  union {
	char *lhs;		/* s command lhs */
	struct cmd_t *link;	/* label link */
  } u;
  char command;			/* command code */
  char *rhs;			/* s command replacement string */
  FILE *fout;			/* associated output file descriptor */
  struct {
	char allbut;		/* was negation specified? */
	char global;		/* was g postfix specified? */
	char print;		/* was p postfix specified? */
	char inrange;		/* in an address range? */
  } flags;
};
typedef struct cmd_t sedcmd;	/* use this name for declarations */

struct label_t {		/* represent a command label */
  char *name;			/* the label name */
  sedcmd *last;			/* it's on the label search list */
  sedcmd *address;		/* pointer to the cmd it labels */
};

typedef struct label_t label;


#define BAD	((char *) -1)	/* guaranteed not a string ptr */



/* Address and regular expression compiled-form markers */
#define STAR	1		/* marker for Kleene star */
#define CCHR	2		/* non-newline character to be matched
			 * follows */
#define CDOT	4		/* dot wild-card marker */
#define CCL	6		/* character class follows */
#define CNL	8		/* match line start */
#define CDOL	10		/* match line end */
#define CBRA	12		/* tagged pattern start marker */
#define CKET	14		/* tagged pattern end marker */
#define CBACK	16		/* backslash-digit pair marker */
#define CLNUM	18		/* numeric-address index follows */
#define CEND	20		/* symbol for end-of-source */
#define CEOF	22		/* end-of-field mark */

#define quit(n) exit(n)

static void compile(void);
static int cmdcomp(char cchar);
static char *rhscomp(char *rhsp, int delim);
static char *recomp(char *expbuf, int redelim);
static int cmdline(char *cbuf);
static char *address(char *expbuf);
static char *gettext(char *txp);
static void resolve(void);
static char *ycomp(char *ep, char delim);
static label *search(label *ptr);
void execute(void);
static int selected(sedcmd *ipc);
static int match(char *expbuf, int gf);
static int advance(char *lp, char *ep);
static int substitute(sedcmd *ipc);
static void dosub(char *rhsbuf);
static char *place(char *asp, char *al1, char *al2);
static void listto(char *p1, FILE *fp);
static void truncated(int h);
static void command(sedcmd *ipc);
static void openfile(char *file);
static void get(void);
static void initget(void);
static char *getline(char *buf);
static int Memcmp(char *a, char *b, int count);
static void readout(void);

#endif	/* _SED_H */
/* Sed.h ends here */
