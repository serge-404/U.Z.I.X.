/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 */

#include "defs.h"

/* ========	input output and file copying ======== */

void
initf(UFD fd)
{
	FILE f = standin;

	f->fdes = fd;
	f->fsiz = ((flags&(oneflg|ttyflg)) == 0? BUFSIZ: 1);
	f->fnxt = f->fend = f->fbuf;
	f->feval = 0;
	f->flin = 1;
	f->feof = FALSE;
}

int
estabf(STRING s)
{
	FILE f = standin;

	f->fdes = -1;
	f->fend = length(s) + (f->fnxt = s);
	f->flin = 1;
	return f->feof = (s == 0);
}

void
push(FILE f)
{
	f->fstak = standin;
	f->feof = 0;
	f->feval = 0;
	standin = f;
}

int
pop(void)
{
	FILE f = standin;

	if (f->fstak) {
		if (f->fdes >= 0)
			close(f->fdes);
		standin = f->fstak;
		return(TRUE);
	} else
		return(FALSE);
}

void
chkpipe(int *pv)
{
	if (pipe(pv) < 0 || pv[INPIPE] < 0 || pv[OTPIPE] < 0)
		error(piperr);
}

int
chkopen(STRING idf)
{
	int rc;

	if ((rc = open(idf, 0)) < 0)
		failed(idf, badopen);
	else
		return(rc);
}

/* renumber, actually */
void
myrename(int f1, int f2)
{
	if (f1 >= 0 && f2 >= 0 && f1 != f2) {
		close(f2);
		if (dup2(f1, f2) < 0)
			error("sh: dup2 in myrename failed");
		close(f1);
		if (f2 == 0)
			ioset |= 1;
	}
}

int
create(STRING s)
{
	int rc;

	if ((rc = creat(s, 0666)) < 0)
		failed(s, badcreate);
	else
		return(rc);
}

int
tmpfil(void)
{
	itos(serial++);
	movstr(numbuf, shtmpnam);
	return(create(tmpout));
}

/* set by trim */
BOOL nosubst;

void
copy(IOPTR ioparg)
{
	int fd;
	char c, *ends, *cline, *clinep;
	IOPTR iop;

	if (iop = ioparg) {
		copy(iop->iolst);
		ends = mactrim(iop->ioname);
		if (nosubst)
			iop->iofile &= ~IODOC;
		fd = tmpfil();
		iop->ioname = cpystak(tmpout);
		iop->iolst = iotemp;
		iotemp = iop;
		cline = locstak();

		for (; ;) {
			clinep = cline;
			chkpr(NL);
			while (c = (nosubst? readc(): nextc(*ends)),
			    !eolchar(c))
				*clinep++ = c;
			*clinep = 0;
			if (eof || eq(cline, ends))
				break;
			*clinep++ = NL;
			write(fd, cline, clinep - cline);
		}
		close(fd);
	}
}
