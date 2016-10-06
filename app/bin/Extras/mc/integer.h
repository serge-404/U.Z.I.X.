#ifndef _INTEGER

typedef int		INT;
typedef unsigned int	UINT;

typedef char		CHAR;
typedef unsigned char	UCHAR;
#ifdef BYTE
#undef BYTE
#endif
typedef unsigned char	BYTE;

typedef short		SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;

typedef long		LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

#ifdef BOOL
#undef BOOL
#endif
typedef unsigned char	BOOL;

#define FALSE	0
#define TRUE	1

#define _INTEGER
#endif
