/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"
#include <errno.h>

#define ARGMK	01

extern int errno;
extern STRING sysmsg[];

static STRING	execs(STRING ap, STRING t[]);
static void	gsort(STRING from[], STRING to[]);
static int	split(STRING s);

/* fault handling */
#ifndef ETXTBSY
#define ETXTBSY 26
#endif

/* service routines for `execute' */

void
initio(IOPTR iop)
{
	STRING ion;
	int iof, fd = 0;

	if (iop) {
		iof = iop->iofile;
		ion = mactrim(iop->ioname);
		if (*ion && (flags & noexec) == 0) {
			if (iof & IODOC) {
				subst(chkopen(ion), (fd = tmpfil()));
				close(fd);
				fd = chkopen(tmpout);
				unlink(tmpout);
			} else if (iof & IOMOV) {
				if (eq(minus, ion)) {
					fd = -1;
					close(iof & IOUFD);
				} else if ((fd = stoi(ion)) >= USERIO)
					failed(ion, badfile);
				else {
					fd = dup(fd);
					if (fd < 0)
						perror("dup");
				}
			} else if ((iof & IOPUT) == 0)
				fd = chkopen(ion);
			else if (flags & rshflg)
				failed(ion, restricted);
			else if (iof & IOAPP && (fd = open(ion, 1)) >= 0)
				lseek(fd, 0L, 2);
			else
				fd = create(ion);
			if (fd >= 0)
				myrename(fd, iof & IOUFD);
		}
		initio(iop->ionxt);
	}
}

STRING
getpath(STRING s)
{
	STRING path;

	if (any('/', s)) {
		if (flags & rshflg)
			failed(s, restricted);
		else
			return(nullstr);
	} else if ((path = pathnod.namval) == 0)
		return(defpath);
	else
		return(cpystak(path));
}

int
pathopen(STRING path, STRING name)
{
	UFD f;

	do {
		path = catpath(path, name);
	} while ((f = open(curstak(), 0)) < 0 && path);
	return f;
}

STRING
catpath(STRING path, STRING name)
{
	/* leaves result on top of stack */
	STRING scanp = path, argp = locstak();

	while (*scanp && *scanp != COLON)
		*argp++ = *scanp++;
	if (scanp != path)
		*argp++ = '/';
	if (*scanp == COLON)
		scanp++;
	path = (*scanp? scanp: 0);
	scanp = name;
	while ((*argp++ = *scanp++) != 0)
		continue;
	return path;
}

static STRING xecmsg;
static STRING *xecenv;

void
execa(STRING t[])
{
	STRING path;

	if ((flags & noexec) == 0) {
		xecmsg = notfound;
		path = getpath(*t);
		namscan(exname);
		xecenv = shsetenv();
		while (path = execs(path, t))
			continue;
		failed(*t, xecmsg);
	}
}

static STRING
execs(STRING ap, STRING t[])
{
	STRING p, prefix;

	prefix = catpath(ap, t[0]);
	trim(p = curstak());

	sigchk();
	execve(p, &t[0], xecenv);

	/* exec failed.  figure out why */
	switch (errno) {
	case ENOEXEC:
		flags = 0;
		comdiv = 0;
		ioset = 0;
		clearup();	/* remove open files and for loop junk */
		if (input)
			close(input);
		close(output);
		output = 2;
		input = chkopen(p);

		/* set up new args */
		setargs(t);
		longjmp(subshell, 1);

	case ENOMEM:
		failed(p, toobig);

	case E2BIG:
		failed(p, arglist);

	case ETXTBSY:
		failed(p, txtbsy);

	default:
		xecmsg = badexec;
		/* FALLTHROUGH */
	case ENOENT:
		return(prefix);
	}
}

/* for processes to be waited for */
#define MAXP 60
static int pwlist[MAXP];
static int pwc;

void
postclr(void)
{
	int *pw = pwlist;

	while (pw <= &pwlist[pwc])
		*pw++ = 0;
	pwc = 0;
}

void
post(int pcsid)
{
	int *pw = pwlist;

	if (pcsid) {
		while (*pw)
			pw++;
		if (pwc >= MAXP - 1)
			pw--;
		else
			pwc++;
		*pw = pcsid;
	}
}

void
await(int i)
{
	int rc = 0, wx = 0, w, ipwc = pwc;

	post(i);
	while (pwc) {
		int p, sig, w_hi;

		{
			int *pw = pwlist;

			p = wait(&w);
			while (pw <= &pwlist[ipwc])
				if (*pw == p) {
					*pw = 0;
					pwc--;
				} else
					pw++;
		}
		if (p == -1)
			continue;

		w_hi = (w >> 8) & LOBYTE;
		if (sig = w & 0177) {
			if (sig == 0177	/* ptrace! return */) {
				prs("ptrace: ");
				sig = w_hi;
			}
			if (sysmsg[sig]) {
				if (i != p || (flags & prompt) == 0) {
					prp();
					prn(p);
					blank();
				}
				prs(sysmsg[sig]);
				if (w & 0200)
					prs(coredump);
			}
			newline();
		}

		if (rc == 0)
			rc = (sig? sig | SIGFLG: w_hi);
		wx |= w;
	}

	if (wx && flags & errflg)
		exitsh(rc);
	exitval = rc;
	exitset();
}

BOOL nosubst;

void
trim(STRING at)
{
	STRING p;
	char c, q = 0;

	if (p = at)
		while (c = *p) {
			*p++ = c & STRIP;
			q |= c;
		}
	nosubst = q & QUOTE;
}

STRING
mactrim(STRING s)
{
	STRING t = macro(s);

	trim(t);
	return t;
}

STRING *
scan(int argn)
{
	ARGPTR argp = (ARGPTR)(Rcheat(gchain) & ~ARGMK);
	STRING *comargn, *comargm;

	comargn = (STRING *)getstak(BYTESPERWORD * argn + BYTESPERWORD);
	comargm = comargn += argn;
	*comargn = ENDARGS;

	while (argp) {
		*--comargn = argp->argval;
		if (argp = argp->argnxt)
			trim(*comargn);
		if (argp == 0 || Rcheat(argp) & ARGMK) {
			gsort(comargn, comargm);
			comargm = comargn;
		}
		/* Lcheat(argp) &= ~ARGMK; */
		argp = (ARGPTR)(Rcheat(argp) & ~ARGMK);
	}
	return comargn;
}

static void
gsort(STRING from[], STRING to[])
{
	int i, j, k, m, n;

	if ((n = to - from) <= 1)
		return;
	for (j = 1; j <= n; j *= 2)
		;
	for (m = 2 * j - 1; m /= 2;) {
		k = n - m;
		for (j = 0; j < k; j++)
			for (i = j; i >= 0; i -= m) {
				STRING *fromi;

				fromi = &from[i];
				if (cf(fromi[m], fromi[0]) > 0)
					break;
				else {
					STRING s;

					s = fromi[m];
					fromi[m] = fromi[0];
					fromi[0] = s;
				}
			}
	}
}

/* Argument list generation */

int
getarg(COMPTR ac)
{
	int count = 0;
	ARGPTR argp;
	COMPTR c;

	if (c = ac)
		for (argp = c->comarg; argp; argp = argp->argnxt)
			count += split(macro(argp->argval));
	return count;
}

static int
split(STRING s)
{
	STRING	argp;
	int c, count = 0;

	for (; ;) {
		sigchk();
		argp = locstak() + BYTESPERWORD;
		while ((c = *s++, !any(c, ifsnod.namval) && c))
			*argp++ = c;
		if (argp == staktop + BYTESPERWORD) {
			if (c)
				continue;
			else
				return count;
		} else if (c == 0)
			s--;
		argp = (STRING)endstak(argp);
		if (c = expand(((ARGPTR)argp)->argval, 0))
			count += c;
		else {		/* assign(&fngnod, argp->argval); */
			makearg(argp);
			count++;
		}
		gchain = (ARGPTR)(((int)gchain) | ARGMK);
	}
	return 0;
}
