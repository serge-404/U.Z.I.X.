/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"
#include "sym.h"
#include "timeout.h"

UFD	output = 2;
static BOOL beenhere = FALSE;

#define	TMPNAM 7	/* offset to variable part of tmpout */
char	tmpout[30] = "/tmp/sh-";

FILEBLK	stdfile;
FILE	standin = &stdfile;

static void exfile(BOOL);

int
main(int c, STRING v[])
{
	int rflag = ttyflg;

	/* renounce set-id; security fix courtesy of Michael Baldwin */
	setgid(getgid());
	setuid(getuid());

	stdsigs();

	/* initialise storage allocation */
	stakbot = 0;
	addblok((unsigned)0);

	/* set names from userenv */
	mygetenv();

	/* look for restricted */
	/*	if (c > 0 && any('r', *v)) rflag = 0; */

	/* look for options */
	dolc = options(c, v);
	if (dolc < 2)
		flags |= stdflg;
	if ((flags & stdflg) == 0)
		dolc--;
	dolv = v + c - dolc;
	dolc--;

	/* return here for shell file execution */
	setjmp(subshell);

	/* number of positional parameters */
	assnum(&dolladr, dolc);
	cmdadr = dolv[0];

	/* set pidname */
	assnum(&pidadr, getpid());

	/* set up temp file names */
	settmp();

	/* default ifs */
	dfault(&ifsnod, sptbnl);

	if ((beenhere++) == FALSE) {	/*? profile */
		if (*cmdadr == '-' &&
		    (input = pathopen(nullstr, profile)) >= 0) {
			exfile(rflag);
			flags &= ~ttyflg;
		}
		if (rflag == 0)
			flags |= rshflg;

		/* open input file if specified */
		if (comdiv) {
			estabf(comdiv);
			input = -1;
		} else {
			input = (flags & stdflg? 0: chkopen(cmdadr));
			comdiv--;
		}
	} else {
		/* *execargs = dolv;	/* for `ps' cmd */
	}

	exfile(0);
	done();
	return 0;
}

static void
exfile(BOOL prof)
{
	int userid;
	long mailtime = 0;
	struct stat statb;

	/* move input */
	if (input > 0) {
		Ldup(input, INIO);
		input = INIO;
	}

	/* move output to safe place */
	if (output == 2) {
		int nfd = dup(2);

		if (nfd < 0)
			perror("dup(2)");
		Ldup(nfd, OTIO);
		output = OTIO;
#ifdef PLAN9
		/*
		 * TODO: fix this.  Plan 9 APE needs it; Unix doesn't.
		 * Without it, we don't get "command: not found" messages
		 * on Plan 9.  APE has had problems with dup before.
		 */
		(void) fcntl(output, F_SETFD, 0);	/* no close-on-exec */
#endif
	}

	userid = getuid();

	/* decide whether interactive */
	if ((flags&intflg) ||
	    ((flags&oneflg) == 0 && isatty(output) && isatty(input))) {
		dfault(&ps1nod, (userid? stdprompt: supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg | prompt;
		ignsig(SIGKILL);
	} else {
		flags |= prof;
		flags &= ~prompt;
	}

	if (setjmp(errshell) && prof) {
		close(input);
		return;
	}

	/* error return here */
	loopcnt = breakcnt = peekc = 0;
	iopend = 0;
	if (input >= 0)
		initf(input);

	/* command loop */
	for (; ;) {
		tdystak(0);
		stakchk();		/* may reduce sbrk */
		exitset();
		if ((flags & prompt) && standin->fstak == 0 && !eof) {
			if (mailnod.namval && stat(mailnod.namval, &statb) >= 0
			    && statb.st_size && statb.st_mtime != mailtime &&
			    mailtime)
				prs(mailmsg);
			mailtime = statb.st_mtime;
			prs(ps1nod.namval);
			alarm(TIMEOUT);
			flags |= waiting;
		}

		trapnote = 0;
		peekc = readc();
		if (eof)
			return;
		alarm(0);
		flags &= ~waiting;
		execute(cmd(NL, MTFLG), 0, NIL, NIL);
		eof |= (flags & oneflg);
	}
}

void
chkpr(char eor)
{
	if ((flags & prompt) && standin->fstak == 0 && eor == NL)
		prs(ps2nod.namval);
}

void
settmp(void)
{
	itos(getpid());
	serial = 0;
	shtmpnam = movstr(numbuf, &tmpout[TMPNAM]);
}

/* dup file descriptor fa to fb, close fa and set fb to close-on-exec */
void
Ldup(int fa, int fb)
{
	if (fa < 0 || fb < 0 || fa == fb)
		return;
#ifdef FIOCLEX
	if (dup2(fa, fb) < 0)
		error("sh: dup2 in Ldup failed");
	(void) ioctl(fb, FIOCLEX, 0);
#else
	close(fb);
	if (fcntl(fa, F_DUPFD, fb) < 0)
		error("sh: dup2 in Ldup failed");
	(void) fcntl(fb, F_SETFD, FD_CLOEXEC);
#endif
	close(fa);
}
