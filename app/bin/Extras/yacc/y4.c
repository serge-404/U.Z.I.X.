# include "dextern.h"

int chfind(int t, char *s);
int skipcom();
int fdtype(int t);
extern int gettok();
extern int defin(int t, char  *s);
extern int	strcmp(char*, char*);

# define IDENTIFIER 257
# define MARK 258
# define TERM 259
# define LEFT 260
# define RIGHT 261
# define BINARY 262
# define PREC 263
# define LCURLY 264
# define C_IDENTIFIER 265  /* name followed by colon */
# define NUMBER 266
# define START 267
# define TYPEDEF 268
# define TYPENAME 269
# define UNION 270
# define ENDFILE 0

	/* communication variables between various I/O routines */

extern char *infile;	/* input file name */
extern int numbval;		/* value of an input number */
extern char tokname[];	/* input token name */

	/* storage of names */

extern char cnames[];	/* place where token and nonterminal names are stored */
extern int cnamsz;		/* size of cnames */
extern char * cnamp;	/* place where next name is to be put in */
extern int ndefout;  	/* number of defined symbols output */

	/* storage of types */
extern int ntypes;			/* number of types defined */
extern char * typeset[];	/* pointers to type tags */

	/* symbol tables for tokens and nonterminals */

extern int ntokens;
extern struct toksymb tokset[];
extern int toklev[];
extern int nnonter;
extern struct ntsymb nontrst[];
extern int start;	/* start symbol */

	/* assigned token type values */
extern int extval;

	/* input and output file descriptors */

extern FILE * finput;		/* yacc input file */
extern FILE * faction;		/* file for saving actions */
extern FILE * fdefine;		/* file for # defines */
extern FILE * ftable;		/* y.tab.c file */
extern FILE * ftemp;		/* tempfile to pass 2 */
extern FILE * foutput;		/* y.output file */

	/* storage for grammar rules */

extern int mem0[]; 		/* production storage */
extern int *mem;
extern int nprod;		/* number of productions */
extern int *prdptr[];	/* pointers to descriptions of productions */
extern int levprd[] ;	/* precedence levels for the productions */

void cpyact2(int jj, int off, int tokk){
			fprintf( faction, "yypvt[-%d]", jj); 
			if( ntypes ){ 										/* put out the proper tag */
				if( off <= 0 && tokk < 0 ) error( "must specify type of $%d", off);
				if( tokk < 0 ) tokk = fdtype( prdptr[nprod][off] );
				fprintf( faction, ".%s", typeset[tokk] );
			}
}

cpyact(offset){ /* copy C action to the next ; or closing } */
	int brac, c, match, j, s, tok;

	fprintf( faction, "\n# line %d \"%s\"\n", lineno, infile );

	brac = 0;

	while (1) {
/*loop:*/
	c = getc(finput);
swt:
	switch( c ){

case ';':
		if( brac == 0 ){
			putc( c , faction );
			return;
		}
		goto lcopy;

case '{':
		brac++;
		goto lcopy;

case '$':
		s = 1;
		tok = -1;
		c = getc(finput);
		if( c == '<' ){ 				/* type description */
			ungetc( c, finput );
			if( gettok() != TYPENAME ) error( "bad syntax on $<ident> clause" );
			tok = numbval;
			c = getc(finput);
		}
		if( c == '$' ){
			fprintf( faction, "yyval");
			if( ntypes ){ /* put out the proper tag... */
				if( tok < 0 ) tok = fdtype( *prdptr[nprod] );
				fprintf( faction, ".%s", typeset[tok] );
			}
			/* goto loop; */ continue;
		}
		if( c == '-' ){
			s = -s;
			c = getc(finput);
		}
		if( isdigit(c) ){
			j=0;
			while( isdigit(c) ){
				j= j*10+c-'0';
				c = getc(finput);
			}

			j = j*s - offset;
			if( j > 0 ){
				error( "Illegal use of $%d", j+offset );
			}

			cpyact2(-j, j+offset, tok); 
			goto swt;
		}
		putc( '$' , faction );
		if( s<0 ) putc('-', faction );
		goto swt;

case '}':
		if( --brac ) goto lcopy;
		putc( c, faction );
		return;


case '/':	/* look for comments */
		putc( c , faction );
		c = getc(finput);
		if( c != '*' ) goto swt;

		/* it really is a comment */

		putc( c , faction );
		c = getc(finput);
		while( c != EOF ){
			while( c=='*' ){
				putc( c , faction );
				if( (c=getc(finput)) == '/' ) goto lcopy;
				}
			putc( c , faction );
			if( c == '\n' )++lineno;
			c = getc(finput);
		}
		error( "EOF inside comment" );

case '\'':	/* character constant */
		match = '\'';
		goto string;

case '"':	/* character string */
		match = '"';

	string:

		putc( c , faction );
		while( c=getc(finput) ){

			if( c=='\\' ){
				putc( c , faction );
				c=getc(finput);
				if( c == '\n' ) ++lineno;
				}
			else if( c==match ) goto lcopy;
			else if( c=='\n' ) error( "newline in string or char. const." );
			putc( c , faction );
		}
		error( "EOF in string or character constant" );

case EOF:
		error("action does not terminate" );

case '\n':	++lineno;
		goto lcopy;

	}

lcopy:
	putc( c , faction );
	} /* while(1) */ /*goto loop;*/
}

fdtype( t ){ /* determine the type of a symbol */
	register v;
	if( t >= NTBASE ) v = nontrst[t-NTBASE].tvalue;
	else v = TYPE( toklev[t] );
	if( v <= 0 ) error( "must specify type for %s", (t>=NTBASE)?nontrst[t-NTBASE].name:
			tokset[t].name );
	return( v );
	}

chfind( t, s ) char *s; {
	register int i;

	if (s[0]==' ')t=0;
	TLOOP(i){
		if(!strcmp(s,tokset[i].name)){
			return( i );
			}
		}
	NTLOOP(i){
		if(!strcmp(s,nontrst[i].name)) {
			return( i+NTBASE );
			}
		}
	/* cannot find name */
	if( t>1 )
		error( "%s should have been defined earlier", s );
	return( defin( t, s ) );
}

cpyunion(){
	/* copy the union declaration to the output, and the define file if present */

	int level, c;
	fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
	fprintf( ftable, "typedef union " );
	if( fdefine ) fprintf( fdefine, "\ntypedef union " );

	level = 0;
	for(;;){
		if( (c=getc(finput)) < 0 ) error( "EOF encountered while processing %%union" );
		putc( c, ftable );
		if( fdefine ) putc( c, fdefine );

		switch( c ){

		case '\n':
			++lineno;
			break;

		case '{':
			++level;
			break;

		case '}':
			--level;
			if( level == 0 ) { /* we are finished copying */
				fprintf( ftable, " YYSTYPE;\n" );
				if( fdefine ) fprintf( fdefine, " YYSTYPE;\nextern YYSTYPE yylval;\n" );
				return;
			}
		}
	}
}

cpycode(){ /* copies code between \{ and \} */

	int c;
	c = getc(finput);
	if( c == '\n' ) {
		c = getc(finput);
		lineno++;
	}
	fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
	while( c>=0 ){
		if( c=='\\' )
			if( (c=getc(finput)) == '}' ) return;
			else putc('\\', ftable );
		if( c=='%' )
			if( (c=getc(finput)) == '}' ) return;
			else putc('%', ftable );
		putc( c , ftable );
		if( c == '\n' ) ++lineno;
		c = getc(finput);
	}
	error("eof before %%}" );
}

skipcom(){ /* skip over comments */
	register c, i=0;  /* i is the number of lines skipped */

	/* skipcom is called after reading a / */

	if( getc(finput) != '*' ) error( "illegal comment" );
	c = getc(finput);
	while( c != EOF ){
		while( c == '*' ){
			if( (c=getc(finput)) == '/' ) return( i );
		}
		if( c == '\n' ) ++i;
		c = getc(finput);
	}
	error( "EOF inside comment" );
	/* NOTREACHED */
}

