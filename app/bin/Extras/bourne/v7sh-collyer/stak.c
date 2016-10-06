/*
 * UNIX shell
 *
 * Stacked-storage allocation.
 *
 * This replaces the original V7 shell's stak.c.  This implementation
 * does not rely on memory faults to manage storage.  See ``A Partial
 * Tour Through the UNIX Shell'' for details.  This implementation is
 * newer than the one published in that paper, but differs mainly in
 * just being a little more portable.  In particular, it works on
 * Ultrasparc and Alpha processors, which are insistently 64-bit processors.
 *
 * Maintains a linked stack (of mostly character strings), the top (most
 * recently allocated item) of which is a growing string, which pushstak()
 * inserts into and grows as needed.
 *
 * Each item on the stack consists of a pointer to the previous item
 * (the "word" pointer; stk.topitem points to the top item on the stack),
 * an optional magic number, and the data.  There may be malloc overhead storage
 * on top of this.  Heap storage returned by alloc() lacks the "word" pointer.
 *
 * Pointers returned by these routines point to the first byte of the data
 * in a stack item; users of this module should be unaware of the "word"
 * pointer and the magic number.  To confuse matters, stk.topitem points to the
 * "word" linked-list pointer of the top item on the stack, and the
 * "word" linked-list pointers each point to the corresponding pointer
 * in the next item on the stack.  This all comes to a head in tdystak().
 *
 * Geoff Collyer
 */

/* see also stak.h */
#include "defs.h"
#undef free				/* refer to free(3) here */

/*
 * was (10*1024) for testing; must be >= sizeof(struct fileblk) always.
 * must also be >= 2*(CPYSIZ in io.c [often 512])
 */
#define BRKINCR 1024

#define TPRS(s)		do { if (Tracemem) prs(s); } while (0)
#define TPRN(n)		do { if (Tracemem) prn(n); } while (0)
#define TPRNN(n)	do { if (Tracemem) prnn(n); } while (0)

#define STPRS(s)	do { if (Stackdebug) prs(s); } while (0)
#define STPRN(n)	do { if (Stackdebug) prn(n); } while (0)
#define STPRNN(n)	do { if (Stackdebug) prnn(n); } while (0)

enum {
	Tracemem = 0,
	Stackdebug = 0,

	STMAGICNUM = 0x1235,		/* stak item magic */
	HPMAGICNUM = 0x4276,		/* heap item magic */
};

/*
 * to avoid relying on features of the Plan 9 C compiler, these structs
 * are expressed rather awkwardly.
 */
typedef struct stackblk Stackblk;
typedef struct {
	Stackblk *word;			/* pointer to previous stack item */
	unsigned long long magic;	/* force pessimal alignment */
} Stackblkhdr;
struct stackblk {
	Stackblkhdr h;
	char	userdata[1];
};

typedef struct {
	unsigned long long magic;	/* force pessimal alignment */
} Heapblkhdr;
typedef struct {
	Heapblkhdr h;
	char	userdata[1];
} Heapblk;

typedef struct {
	char	*base;
	/*
	 * A chain of ptrs of stack blocks that have become covered
	 * by heap allocation.  `tdystak' will return them to the heap.
	 */
	Stackblk *topitem;
} Stack;

unsigned brkincr = BRKINCR;		/* used in stak.h only */

static Stack stk;

/* forwards */
void	prnn(int);
static char *stalloc(int);
static void debugsav(char *);

static void
tossgrowing(void)				/* free the growing stack */
{
	if (stk.topitem != 0) {		/* any growing stack? */
		Stackblk *nextitem;

		/* verify magic before freeing */
		if (stk.topitem->h.magic != STMAGICNUM) {
			prs("tossgrowing: stk.topitem->h.magic ");
			prn((long)stk.topitem->h.magic);
			prs("\n");
			error("tossgrowing: bad magic on stack");
		}
		stk.topitem->h.magic = 0;	/* erase magic */

		/* about to free the ptr to next, so copy it first */
		nextitem = stk.topitem->h.word;

		TPRS("tossgrowing freeing ");
		TPRN((int)stk.topitem);
		TPRS("\n");

		free(stk.topitem);
		stk.topitem = nextitem;
	}
}

static char *
stalloc(int size)		/* allocate requested stack space (no frills) */
{
	Stackblk *nstk;

	TPRS("stalloc allocating ");
	TPRN(sizeof(Stackblkhdr) + size);
	TPRS(" user bytes ");

	if (size < sizeof(long long))
		size = sizeof(long long);
	nstk = malloc(sizeof(Stackblkhdr) + size);
	if (nstk == 0)
		error(nostack);

	TPRS("@ ");
	TPRN((int)nstk);
	TPRS("\n");

	/* stack this item */
	nstk->h.word = stk.topitem;		/* point back @ old stack top */
	stk.topitem = nstk;			/* make this new stack top */

	nstk->h.magic = STMAGICNUM;	/* add magic number for verification */
	return nstk->userdata;
}

static void
grostalloc(void)			/* allocate growing stack */
{
	int size = BRKINCR;

	/* fiddle global variables to point into this (growing) stack */
	staktop = stakbot = stk.base = stalloc(size);
	stakend = stk.base + size - 1;
}

/*
 * allocate requested stack.
 * staknam() assumes that getstak just realloc's the growing stack,
 * so we must do just that.  Grump.
 */
char *
getstak(int asize)
{
	int staklen;
	char *nstk;

	/* +1 is because stakend points at the last byte of the growing stack */
	staklen = stakend + 1 - stk.base;	/* # of usable bytes */

	TPRS("getstak(");
	TPRN(asize);
	TPRS(") calling growstak(");
	TPRNN(asize - staklen);
	TPRS("):\n");

	nstk = growstak(asize - staklen); /* grow growing stack to requested size */
	grostalloc();				/* allocate new growing stack */
	return nstk;
}

void
prnn(int i)
{
	if (i < 0) {
		prs("-");
		i = -i;
	}
	prn(i);
}

/*
 * set up stack for local use (i.e. make it big).
 * should be followed by `endstak'
 */
char *
locstak(void)
{
	if (stakend + 1 - stakbot < BRKINCR) {
		TPRS("locstak calling growstak(");
		TPRNN(BRKINCR - (stakend + 1 - stakbot));
		TPRS("):\n");
		(void) growstak(BRKINCR - (stakend + 1 - stakbot));
	}
	return stakbot;
}

/*
 * return an address to be used by tdystak later,
 * so it must be returned by getstak because it may not be
 * a part of the growing stack, which is subject to moving.
 */
char *
savstak(void)
{
/*	assert(staktop == stakbot);	/* assert empty stack */
	return getstak(1);
}

/*
 * tidy up after `locstak'.
 * make the current growing stack a semi-permanent item and
 * generate a new tiny growing stack.
 */
char *
endstak(char *argp)
{
	char *ostk;

	*argp++ = 0;				/* terminate the string */
	TPRS("endstak calling growstak(");
	TPRNN(-(stakend + 1 - argp));
	TPRS("):\n");
	ostk = growstak(-(stakend + 1 - argp));	/* reduce growing stack size */
	grostalloc();				/* alloc. new growing stack */
	return ostk;				/* perm. addr. of old item */
}

/*
 * Try to bring the "stack" back to sav (the address of userdata[0] in some
 * Stackblk, returned by growstak()), and bring iotemp's stack back to iosav
 * (an old copy of iotemp, which may be zero).
 */
void
tdystak(char *sav /* , struct ionod *iosav */ )
{
	Stackblk *blk = NIL;
	struct ionod *iosav = NIL;

	rmtemp(iosav);				/* pop temp files */

	if (sav != 0)
		blk = (Stackblk *)(sav - sizeof(Stackblkhdr));
	if (sav == 0)
		STPRS("tdystak(0)\n");
	else if (blk->h.magic == STMAGICNUM) {
		STPRS("tdystak(data ptr: ");
		STPRN((int)sav);
		STPRS(")\n");
	} else {
		STPRS("tdystak(garbage: ");
		STPRN((int)sav);
		STPRS(")\n");
		error("tdystak: bad magic in argument");
	}

	/*
	 * pop stack to sav (if zero, pop everything).
	 * stk.topitem points at the ptr before the data & magic.
	 */
	while (stk.topitem != 0 && (sav == 0 || stk.topitem != blk)) {
		debugsav(sav);
		tossgrowing();			/* toss the stack top */
	}
	debugsav(sav);
	STPRS("tdystak: done popping\n");
	grostalloc();				/* new growing stack */
	STPRS("tdystak: exit\n");
}

static void
debugsav(char *sav)
{
	if (stk.topitem == 0)
		STPRS("tdystak: stk.topitem == 0\n");
	else if (sav != 0 &&
	    stk.topitem == (Stackblk *)(sav - sizeof(Stackblkhdr))) {
		STPRS("tdystak: stk.topitem == link ptr of arg: ");
		STPRN((int)stk.topitem);
		STPRS("\n");
	} else {
		STPRS("tdystak: stk.topitem == link ptr of item above arg: ");
		STPRN((int)stk.topitem);
		STPRS("\n");
	}
}

void
stakchk(void)			/* reduce growing-stack size if feasible */
{
	if (stakend - staktop > 2*BRKINCR) { /* lots of unused stack headroom */
		TPRS("stakchk calling growstak(");
		TPRNN(-(stakend - staktop - BRKINCR));
		TPRS("):\n");
		(void) growstak(-(stakend - staktop - BRKINCR));
	}
}

char *			/* address of copy of newstak */
cpystak(char *newstak)
{
	return strcpy(getstak(strlen(newstak) + 1), newstak);
}

char *				/* new address of grown stak */
growstak(int incr)		/* grow the growing stack by incr */
{
	int staklen;
	unsigned topoff, botoff, basoff;
	char *oldbsy;

	if (stk.topitem == 0)			/* paranoia */
		grostalloc();			/* make a trivial stack */

	/* paranoia: during realloc, point @ previous item in case of signals */
	oldbsy = (char *)stk.topitem;
	stk.topitem = stk.topitem->h.word;

	topoff = staktop - oldbsy;
	botoff = stakbot - oldbsy;
	basoff = stk.base - oldbsy;

	/* +1 is because stakend points at the last byte of the growing stack */
	staklen = stakend + 1 + incr - oldbsy;

	if (staklen < sizeof(Stackblkhdr))	/* paranoia */
		staklen = sizeof(Stackblkhdr);

	TPRS("growstak growing ");
	TPRN((int)oldbsy);
	TPRS(" from ");
	TPRN(stakend + 1 - oldbsy);
	TPRS(" bytes; ");

	if (incr < 0) {
		/*
		 * V7 realloc wastes the memory given back when
		 * asked to shrink a block, so we malloc new space
		 * and copy into it in the hope of later reusing the old
		 * space, then free the old space.
		 */
		char *new = malloc((unsigned)staklen);

		if (new == NIL)
			error(nostack);
		memcpy(new, oldbsy, staklen);
		free(oldbsy);
		oldbsy = new;
	} else
		/* get realloc to grow the stack to match the stack top */
		if ((oldbsy = realloc(oldbsy, (unsigned)staklen)) == NIL)
			error(nostack);
	TPRS("now @ ");
	TPRN((int)oldbsy);
	TPRS(" of ");
	TPRN(staklen);
	TPRS(" bytes (");
	if (incr < 0) {
		TPRN(-incr);
		TPRS(" smaller");
	} else {
		TPRN(incr);
		TPRS(" bigger");
	}
	TPRS(")\n");

	stakend = oldbsy + staklen - 1;	/* see? points at the last byte */
	staktop = oldbsy + topoff;
	stakbot = oldbsy + botoff;
	stk.base = oldbsy + basoff;

	stk.topitem = (Stackblk *)oldbsy;	/* restore after realloc */
	return stk.base;			/* addr of 1st usable byte */
}

/* ARGSUSED reqd */
void
addblok(unsigned reqd)			/* called from main at start only */
{
	USED(reqd);
	if (stakbot == 0)
		grostalloc();			/* allocate initial arena */
}

/*
 * Heap allocation.
 */
char *
alloc(unsigned size)
{
	Heapblk *p = malloc(sizeof(Heapblkhdr) + size);

	if (p == NIL)
		error(nospace);

	p->h.magic = HPMAGICNUM;

	TPRS("alloc allocated ");
	TPRN(size);
	TPRS(" user bytes @ ");
	TPRN((int)p->userdata);
	TPRS("\n");
	return p->userdata;
}

/*
 * the shell's private "free" - frees only heap storage.
 * only works on non-null pointers to heap storage
 * (below the data break and stamped with HPMAGICNUM).
 * so it is "okay" for the shell to attempt to free data on its
 * (real) stack, including its command line arguments and environment,
 * or its fake stak.
 * this permits a quick'n'dirty style of programming to "work".
 * the use of sbrk is relatively slow, but effective.
 *
 * the return type of sbrk is apparently changing from char* to void*.
 */
void
shfree(void *ap)
{
	char *p = ap;

	if (p != 0 && p < (char *)sbrk(0)) {	/* plausible data seg ptr? */
		Heapblk *blk = (Heapblk *)(p - sizeof(Heapblkhdr));

		TPRS("shfree freeing user data @ ");
		TPRN((int)p);
		TPRS("\n");
		/* ignore attempts to free non-heap storage */
		if (blk->h.magic == HPMAGICNUM) {
			blk->h.magic = 0;		/* erase magic */
			free(blk);
		}
	}
}
