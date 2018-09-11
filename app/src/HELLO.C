#include "stdio.h"

/*
#ifdef CPM
#error test1
#endif

#ifndef CPM
#error test1
#endif
*/

int main()
{
  int cc = 0666; 
  printf("%d\n",cc); 
#ifndef ORION_UTILS_TARGET
  printf("\nHello, world\n");
#else
  printf("\nHello, ORION_UTILS_TARGET\n");
#endif
}

