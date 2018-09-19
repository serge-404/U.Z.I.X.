#include "stdio.h"
#include <string.h>

/*
#ifdef CPM
#error test1
#endif

#ifndef CPM
#error test1
#endif
*/

char fname[100]="/path/filename.tXt";

int strucmp(char *d, char *s)	/* CASE insensitive string compare */
{
	register char c1, *s1 = (char *)d, *s2 = (char *)s, c2;

	while ( ( c1 = ((c1=*s1++) > 0x60 ? c1 & 0x5F : c1 )) == ( c2 = ((c2=*s2++) > 0x60 ? c2 & 0x5F : c2 )) && c1)
		;
	return c1 - c2;
}

int main()
{
  int cc = 0666; 
  char* fext;  

  printf("%d\n",cc); 
 
  if ( (unsigned)(void*)strrchr(fname, '.') - (unsigned)(void*)fname == strlen(fname)-4 )
    printf("3-char ext\n");
  else
    printf("ext not found\n");

  if ( (fext=strrchr(fname, '.')) && strucmp(fext,".COM"))
    printf("Not COM file\n");
  else
    printf("COM file\n");

  if ( (fext=strrchr(fname, '.')) && strucmp(fext,".cOm"))
    printf("Not COM file\n");
  else
    printf("COM file\n");

  if ( (fext=strrchr(fname, '.')) && strucmp(fext,".TxT"))
    printf("Not TXT file\n");
  else
    printf("TXT file\n");


#ifndef ORION_UTILS_TARGET
  printf("\nHello, world\n");
#else
  printf("\nHello, ORION_UTILS_TARGET\n");
#endif
}

