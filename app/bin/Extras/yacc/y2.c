# include "dextern.h"

extern int gettok();
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

char *infile;	/* input file name */
int numbval;	/* value of an input number */
char tokname[NAMESIZE];	/* input token name */

	/* storage of names */

char cnames[CNAMSZ];	/* place where token and nonterminal names are stored */
int cnamsz = CNAMSZ;	/* size of cnames */
char * cnamp = cnames;	/* place where next name is to be put in */
int ndefout = 3;  /* number of defined symbols output */

	/* storage of types */
int ntypes;	/* number of types defined */
char * typeset[NTYPES];	/* pointers to type tags */

	/* symbol tables for tokens and nonterminals */

int ntokens = 0;
struct toksymb tokset[NTERMS];
int toklev[NTERMS];
int nnonter = -1;
struct ntsymb nontrst[NNONTERM];
int start;	/* start symbol */

	/* assigned token type values */
int extval = 0;

	/* input and output file descriptors */

FILE * finput;		/* yacc input file */
FILE * faction;		/* file for saving actions */
FILE * fdefine;		/* file for # defines */
FILE * ftable;		/* y.tab.c file */
FILE * ftemp;		/* tempfile to pass 2 */
FILE * foutput;		/* y.output file */

	/* storage for grammar rules */

int mem0[MEMSIZE] ; /* production storage */
int *mem = mem0;
int nprod= 1;	/* number of productions */
int *prdptr[NPROD];	/* pointers to descriptions of productions */
int levprd[NPROD] ;	/* precedence levels for the productions */

#define UpCase(ch) (islower(ch)?toupper(ch):ch)


setup(argc,argv) int argc; char *argv[];
{
	char cc;

	foutput = NULL;
	fdefine = NULL;
/*	i = 1; */

	while( argc >= 2  && argv[1][0] == '-' ) {
		while( *++(argv[1]) ){	
			if ((cc=UpCase(*argv[1]))=='V') {
				foutput = fopen(FILEU, "w" );
				if( foutput == NULL ) error( "cannot open y.output" );
				continue;
			} else if (cc=='D') {
				fdefine = fopen( FILED, "w" );
			} else if (cc=='O') {
				fprintf( stderr, "`o' flag now default in yacc\n" );
			} else
				error( "illegal option: %c", *argv[1]);
		}
		argv++;
		argc--;
	}

	ftable = fopen( OFILE, "w" );
	if( ftable == NULL ) error( "cannot open table file" );

	ftemp = fopen( TEMPNAME, "w" );
	faction = fopen( ACTNAME, "w" );
	if( ftemp==NULL || faction==NULL ) error( "cannot open temp file" );

	if( argc < 2 || ((finput=fopen( infile=argv[1], "r" )) == NULL ) ){
		error( "cannot open input file" );
		}

	cnamp = cnames;
	defin(0,"$end");
	extval = 0400;
	defin(0,"error");
	defin(1,"$accept");
	mem=mem0;
}

void setup2() {
	defout();

	fprintf( ftable,  "#define yyclearin yychar = -1\n" );
	fprintf( ftable,  "#define yyerrok yyerrflag = 0\n" );
	fprintf( ftable,  "extern int yychar;\nextern short yyerrflag;\n" );
	fprintf( ftable,  "#ifndef YYMAXDEPTH\n#define YYMAXDEPTH 150\n#endif\n" );
	if( !ntypes ) fprintf( ftable,  "#ifndef YYSTYPE\n#define YYSTYPE int\n#endif\n" );
	fprintf( ftable,  "YYSTYPE yylval, yyval;\n" );

	prdptr[0]=mem;
	/* added production */
	*mem++ = NTBASE;
	*mem++ = start;  /* if start is 0, we will overwrite with the lhs of the first rule */
	*mem++ = 1;
	*mem++ = 0;
	prdptr[1]=mem;
}

void setup4(int *xt, int xj, int xty) {

					if( xty ){
						if( TYPE(toklev[xj]) ) error( "redeclaration of type of %s", tokname );
						SETTYPE(toklev[xj],xty);
					}
					if( (*xt=gettok()) == NUMBER ){
						tokset[xj].value = numbval;
						if( xj < ndefout && xj>2 ){
							error( "please define type number of %s earlier",
								tokset[xj].name );
						}
						*xt=gettok();
					}
}

void setup3() {
		register tempty;

		*mem++ = -nprod;

		/* check that default action is reasonable */

		if( ntypes && !(levprd[nprod]&ACTFLAG) && nontrst[*prdptr[nprod]-NTBASE].tvalue ){
			/* no explicit action, LHS has value */
			tempty = prdptr[nprod][1];
			if( tempty < 0 ) error( "must return a value, since LHS has a type" );
			else if( tempty >= NTBASE ) tempty = nontrst[tempty-NTBASE].tvalue;
			else tempty = TYPE( toklev[tempty] );
			if( tempty != nontrst[*prdptr[nprod]-NTBASE].tvalue ){
				error( "default action causes potential type clash" );
			}
		}

		if( ++nprod >= NPROD ) error( "more than %d rules", NPROD );
		prdptr[nprod] = mem;
		levprd[nprod]=0;
}

setup1() {
	int i,j,lev,t, ty;
	int c;
	int *p;
	char actname[8];

	lev=ty=i=0;

	/* sorry -- no yacc parser here.....
		we must bootstrap somehow... */

	for( t=gettok();  t!=MARK && t!= ENDFILE; ){
		switch( t ){

		case ';':
			t = gettok();
			break;

		case START:
			if( (t=gettok()) != IDENTIFIER ){
				error( "bad %%start construction" );
				}
			start = chfind(1,tokname);
			t = gettok();
			continue;

		case TYPEDEF:
			if( (t=gettok()) != TYPENAME ) error( "bad syntax in %%type" );
			ty = numbval;
			for(;;){
				t = gettok();
				switch( t ){

				case IDENTIFIER:
					if( (t=chfind( 1, tokname ) ) < NTBASE ) {
						j = TYPE( toklev[t] );
						if( j!= 0 && j != ty ){
							error( "type redeclaration of token %s",
								tokset[t].name );
							}
						else SETTYPE( toklev[t],ty);
						}
					else {
						j = nontrst[t-NTBASE].tvalue;
						if( j != 0 && j != ty ){
							error( "type redeclaration of nonterminal %s",
								nontrst[t-NTBASE].name );
							}
						else nontrst[t-NTBASE].tvalue = ty;
						}
				case ',':
					continue;

				case ';':
					t = gettok();
					break;
				default:
					break;
					}
				break;
				}
			continue;

		case UNION:
			/* copy the union declaration to the output */
			cpyunion();
			t = gettok();
			continue;

		case LEFT:
		case BINARY:
		case RIGHT:
			++i;
		case TERM:
			lev = t-TERM;  /* nonzero means new prec. and assoc. */
			ty = 0;

			/* get identifiers so defined */

			t = gettok();
			if( t == TYPENAME ){ /* there is a type defined */
				ty = numbval;
				t = gettok();
				}

			for(;;) {
				if (t==',') {
					t = gettok();
					continue;
				} else if (t==';') {
					break;
				} else if (t==IDENTIFIER) {
					j = chfind(0,tokname);
					if( lev ){
						if( ASSOC(toklev[j]) ) error( "redeclaration of precedence of %s", tokname );
						SETASC(toklev[j],lev);
						SETPLEV(toklev[j],i);
					}
					setup4(&t,j,ty);
					continue;
				}

				break;
			}

			continue;

		case LCURLY:
			defout();
			cpycode();
			t = gettok();
			continue;
		default:
			error( "syntax error" );
		}
	}

	if( t == ENDFILE ){
		error( "unexpected EOF before %%" );
		}

	/* t is MARK */

	setup2();

	while( (t=gettok()) == LCURLY ) cpycode();

	if( t != C_IDENTIFIER ) error( "bad syntax on first rule" );

	if( !start ) prdptr[0][1] = chfind(1,tokname);

	/* read rules */

	while( t!=MARK && t!=ENDFILE ){

		/* process a rule */

		if( t == '|' ){
			*mem++ = *prdptr[nprod-1];
			}
		else if( t == C_IDENTIFIER ){
			*mem = chfind(1,tokname);
			if( *mem < NTBASE ) error( "token illegal on LHS of grammar rule" );
			++mem;
			}
		else error( "illegal rule: missing semicolon or | ?" );

		/* read rule body */


		t = gettok();
	more_rule:
		while( t == IDENTIFIER ) {
			*mem = chfind(1,tokname);
			if( *mem<NTBASE ) levprd[nprod] = toklev[*mem];
			++mem;
			t = gettok();
			}


		if( t == PREC ){
			if( gettok()!=IDENTIFIER) error( "illegal %%prec syntax" );
			j = chfind(2,tokname);
			if( j>=NTBASE)error("nonterminal %s illegal after %%prec", nontrst[j-NTBASE].name);
			levprd[nprod]=toklev[j];
			t = gettok();
			}

		if( t == '=' ){
			levprd[nprod] |= ACTFLAG;
			fprintf( faction, "\ncase %d:", nprod );
			cpyact( mem-prdptr[nprod]-1 );
			fprintf( faction, " break;" );
			if( (t=gettok()) == IDENTIFIER ){
				/* action within rule... */

				sprintf( actname, "$$%d", nprod );
				j = chfind(1,actname);  /* make it a nonterminal */

				/* the current rule will become rule number nprod+1 */
				/* move the contents down, and make room for the null */

				for( p=mem; p>=prdptr[nprod]; --p ) p[2] = *p;
				mem += 2;

				/* enter null production for action */

				p = prdptr[nprod];

				*p++ = j;
				*p++ = -nprod;

				/* update the production information */

				levprd[nprod+1] = levprd[nprod] & ~ACTFLAG;
				levprd[nprod] = ACTFLAG;

				if( ++nprod >= NPROD ) error( "more than %d rules", NPROD );
				prdptr[nprod] = p;

				/* make the action appear in the original rule */
				*mem++ = j;

				/* get some more of the rule */

				goto more_rule;
				}

			}

		while( t == ';' ) t = gettok();

		setup3();
	}

	/* end of all rules */

	finact();
	if( t == MARK ){
		fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
		while( (c=getc(finput)) != EOF ) putc( c, ftable );
	}
	fclose( finput );
}

