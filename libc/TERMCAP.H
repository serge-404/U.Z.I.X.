#ifndef __TERMCAP_H
#define __TERMCAP_H
#ifndef __TYPES_H
#include "types.h"
#endif

typedef struct buffer_s {
	char	*beg;
	int	size;
	char	*ptr;
	int	ateof;
	int	full;
} buffer_t, *pbuffer;

extern char PC;
extern char *UP;
extern char *BC;
extern int ospeed;

extern int tgetent __P((char *, char *));
extern int tgetflag __P((char *));
extern int tgetnum __P((char *));
extern char *tgetstr __P((char *, char **));

typedef int (*outfun_t) __P((int));

extern void tputs __P((char *, int, outfun_t));
extern char *tgoto __P((char *, int, int));
extern char *tparam();	/* VARARGS */

#endif /* _TERMCAP_H */

