# include "dextern.h"

extern int chfind(int t, char *s);
extern int skipcom();
int defin(int t, char  *s);
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

gettok() {
	register i, base;
	static int peekline; /* number of '\n' seen in lookahead */
	register c, match, reserve;

begin:
	reserve = 0;
	lineno += peekline;
	peekline = 0;
	c = getc(finput);
	while( c==' ' || c=='\n' || c=='\t' || c=='\f' ){
		if( c == '\n' ) ++lineno;
		c=getc(finput);
		}
	if( c == '/' ){ /* skip comment */
		lineno += skipcom();
		goto begin;
		}

	switch(c){

	case EOF:
		return(ENDFILE);
	case '{':
		ungetc( c, finput );
		return( '=' );  /* action ... */
	case '<':  /* get, and look up, a type name (union member name) */
		i = 0;
		while( (c=getc(finput)) != '>' && c>=0 && c!= '\n' ){
			tokname[i] = c;
			if( ++i >= NAMESIZE ) --i;
			}
		if( c != '>' ) error( "unterminated < ... > clause" );
		tokname[i] = '\0';
		for( i=1; i<=ntypes; ++i ){
			if( !strcmp( typeset[i], tokname ) ){
				numbval = i;
				return( TYPENAME );
				}
			}
		typeset[numbval = ++ntypes] = cstash( tokname );
		return( TYPENAME );

	case '"':	
	case '\'':
		match = c;
		tokname[0] = ' ';
		i = 1;
		for(;;){
			c = getc(finput);
			if( c == '\n' || c == EOF )
				error("illegal or missing ' or \"" );
			if( c == '\\' ){
				c = getc(finput);
				tokname[i] = '\\';
				if( ++i >= NAMESIZE ) --i;
				}
			else if( c == match ) break;
			tokname[i] = c;
			if( ++i >= NAMESIZE ) --i;
			}
		break;

	case '%':
	case '\\':

		switch(c=getc(finput)) {

		case '0':	return(TERM);
		case '<':	return(LEFT);
		case '2':	return(BINARY);
		case '>':	return(RIGHT);
		case '%':
		case '\\':	return(MARK);
		case '=':	return(PREC);
		case '{':	return(LCURLY);
		default:	reserve = 1;
			}

	default:

		if( isdigit(c) ){ /* number */
			numbval = c-'0' ;
			base = (c=='0') ? 8 : 10 ;
			for( c=getc(finput); isdigit(c) ; c=getc(finput) ){
				numbval = numbval*base + c - '0';
				}
			ungetc( c, finput );
			return(NUMBER);
			}
		else if( islower(c) || isupper(c) || c=='_' || c=='.' || c=='$' ){
			i = 0;
			while( islower(c) || isupper(c) || isdigit(c) || c=='_' || c=='.' || c=='$' ){
				tokname[i] = c;
				if( reserve && isupper(c) ) tokname[i] += 'a'-'A';
				if( ++i >= NAMESIZE ) --i;
				c = getc(finput);
				}
			}
		else return(c);

		ungetc( c, finput );
		}

	tokname[i] = '\0';

	if( reserve ){ /* find a reserved word */
		if( !strcmp(tokname,"term")) return( TERM );
		if( !strcmp(tokname,"token")) return( TERM );
		if( !strcmp(tokname,"left")) return( LEFT );
		if( !strcmp(tokname,"nonassoc")) return( BINARY );
		if( !strcmp(tokname,"binary")) return( BINARY );
		if( !strcmp(tokname,"right")) return( RIGHT );
		if( !strcmp(tokname,"prec")) return( PREC );
		if( !strcmp(tokname,"start")) return( START );
		if( !strcmp(tokname,"type")) return( TYPEDEF );
		if( !strcmp(tokname,"union")) return( UNION );
		error("invalid escape, or illegal reserved word: %s", tokname );
		}

	/* look ahead to distinguish IDENTIFIER from C_IDENTIFIER */

	c = getc(finput);
	while( c==' ' || c=='\t'|| c=='\n' || c=='\f' || c== '/' ) {
		if( c == '\n' ) ++peekline;
		else if( c == '/' ){ /* look for comments */
			peekline += skipcom();
			}
		c = getc(finput);
		}
	if( c == ':' ) return( C_IDENTIFIER );
	ungetc( c, finput );
	return( IDENTIFIER );
}

finact(){
	/* finish action routine */

	fclose(faction);

	fprintf( ftable, "# define YYERRCODE %d\n", tokset[2].value );

}

defin( t, s ) char  *s; {
/*	define s to be a terminal if t=0
	or a nonterminal if t=1		*/

	register val;

	if (t) {
		if( ++nnonter >= NNONTERM ) error("too many nonterminals, limit %d",NNONTERM);
		nontrst[nnonter].name = cstash(s);
		return( NTBASE + nnonter );
		}
	/* must be a token */
	if( ++ntokens >= NTERMS ) error("too many terminals, limit %d",NTERMS );
	tokset[ntokens].name = cstash(s);

	/* establish value for token */

	if( s[0]==' ' && s[2]=='\0' ) /* single character literal */
		val = s[1];
	else if ( s[0]==' ' && s[1]=='\\' ) { /* escape sequence */
		if( s[3] == '\0' ){ /* single character escape sequence */
			switch ( s[2] ){
					 /* character which is escaped */
			case 'n': val = '\n'; break;
			case 'r': val = '\r'; break;
			case 'b': val = '\b'; break;
			case 't': val = '\t'; break;
			case 'f': val = '\f'; break;
			case '\'': val = '\''; break;
			case '"': val = '"'; break;
			case '\\': val = '\\'; break;
			default: error( "invalid escape" );
				}
			}
		else if( s[2] <= '7' && s[2]>='0' ){ /* \nnn sequence */
			if( s[3]<'0' || s[3] > '7' || s[4]<'0' ||
				s[4]>'7' || s[5] != '\0' ) error("illegal \\nnn construction" );
			val = 64*s[2] + 8*s[3] + s[4] - 73*'0';
			if( val == 0 ) error( "'\\000' is illegal" );
			}
		}
	else {
		val = extval++;
		}
	tokset[ntokens].value = val;
	toklev[ntokens] = 0;
	return( ntokens );
	}

defout(){ /* write out the defines (at the end of the declaration section) */

	register int i, c;
	register char *cp;

	for( i=ndefout; i<=ntokens; ++i ){

		cp = tokset[i].name;
		if( *cp == ' ' ) ++cp;  /* literals */

		for( ; (c= *cp)!='\0'; ++cp ){

			if( islower(c) || isupper(c) || isdigit(c) || c=='_' );  /* VOID */
			else goto nodef;
			}

		fprintf( ftable, "# define %s %d\n", tokset[i].name, tokset[i].value );
		if( fdefine != NULL ) fprintf( fdefine, "# define %s %d\n", tokset[i].name, tokset[i].value );

	nodef:	;
		}

	ndefout = ntokens+1;

}

char *
cstash( s ) register char *s; {
	char *temp;

	temp = cnamp;
	do {
		if( cnamp >= &cnames[cnamsz] ) error("too many characters in id's and literals" );
		else *cnamp++ = *s;
		}  while ( *s++ );
	return( temp );
}

