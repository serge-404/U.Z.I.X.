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
#ifndef ORION_UTILS_TARGET
  printf("\nHello, world\n");
#else
  printf("\nHello, ORION_UTILS_TARGET\n");
#endif
}

