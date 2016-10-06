/*

String management functions
(c) 2010 Serge

*/

#ifndef _STRINGZ

#include <hitech.h>

#define FALSE	0
#define TRUE	1

/* Functions forwards */

extern char* alt2koi(char* strn);
extern char* koi2alt(char* strn);
extern int __atoi(char *st);
extern char* UpperCase(char* ss);
extern BOOL FileFilter(register char* name, char* filter);

#define _STRINGZ
#endif
