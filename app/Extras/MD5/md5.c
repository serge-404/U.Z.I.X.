/*
 * $Id: md5.c,v 1.7.2.2 1998/03/06 08:15:33 jkh Exp $
 *
 * Derived from:
 */

/*
 * MDDRIVER.C - test driver for MD2, MD4 and MD5
 */

/*
 *  Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
 *  rights reserved.
 *
 *  RSA Data Security, Inc. makes no representations concerning either
 *  the merchantability of this software or the suitability of this
 *  software for any particular purpose. It is provided "as is"
 *  without express or implied warranty of any kind.
 *
 *  These notices must be retained in any copies of any part of this
 *  documentation and/or software.
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "md5.h"
#include "global.h"

/*
 * Length of test block, number of test blocks.
 */
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

static void MDString PROTO_LIST((char *));
static void MDTimeTrial PROTO_LIST((void));
static void MDTestSuite PROTO_LIST((void));
static void MDFilter PROTO_LIST((int));
static void usage PROTO_LIST((void));

/* Main driver.

Arguments (may be any combination):
  -sstring - digests string
  -t       - runs time trial
  -x       - runs test script
  filename - digests file
  (none)   - digests standard input
 */
int
main(argc, argv)
	int     argc;
	char   *argv[];
{
	int     ch;
	char   *p;
	char	buf[33];

	if (argc > 1) {
		while ((ch = getopt(argc, argv, "ps:tx")) != -1) {
			switch (ch) {
			case 'p':
				MDFilter(1);
				break;
			case 's':
				MDString(optarg);
				break;
			case 't':
				MDTimeTrial();
				break;
			case 'x':
				MDTestSuite();
				break;
			default:
				usage();
			}
		}
		while (optind < argc) {
			p = MD5File(argv[optind], buf);
			if (!p)
				perror(argv[optind]);
			else
				printf("MD5 (%s) = %s\n", argv[optind], p);
			optind++;
		}
	} else
		MDFilter(0);

	return (0);
}
/*
 * Digests a string and prints the result.
 */
static void
MDString(string)
	unsigned char   *string;
{
	unsigned int len = strlen(string);
	char buf[33];

	printf("MD5 (\"%s\") = %s\n", string, MD5Data(string, len, buf));
}
/*
 * Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte blocks.
 */
static void
MDTimeTrial()
{
	MD5_CTX context;
	time_t  endTime, startTime;
	unsigned char block[TEST_BLOCK_LEN];
	unsigned int i;
	char   *p, buf[33];

	printf
	    ("MD5 time trial. Digesting %d %d-byte blocks ...",
	    TEST_BLOCK_COUNT, TEST_BLOCK_LEN);

	/* Initialize block */
	for (i = 0; i < TEST_BLOCK_LEN; i++)
		block[i] = (unsigned char) (i & 0xff);

	/* Start timer */
	time(&startTime);

	/* Digest blocks */
	MD5Init(&context);
	for (i = 0; i < TEST_BLOCK_COUNT; i++)
		MD5Update(&context, block, TEST_BLOCK_LEN);
	p = MD5End(&context,buf);

	/* Stop timer */
	time(&endTime);

	printf(" done\n");
	printf("Digest = %s", p);
	printf("\nTime = %ld seconds\n", (long) (endTime - startTime));
	/* Be careful that endTime-startTime is not zero. (Bug fix from Ric
	 * Anderson, ric@Artisoft.COM.) */
	printf
	    ("Speed = %ld bytes/second\n",
	    (long) TEST_BLOCK_LEN * (long) TEST_BLOCK_COUNT / ((endTime - startTime) != 0 ? (endTime - startTime) : 1));
}
/*
 * Digests a reference suite of strings and prints the results.
 */
static void
MDTestSuite()
{

	printf("MD5 test suite:\n");

	MDString("");
	MDString("a");
	MDString("abc");
	MDString("message digest");
	MDString("abcdefghijklmnopqrstuvwxyz");
	MDString
	    ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	MDString
	    ("1234567890123456789012345678901234567890\
1234567890123456789012345678901234567890");
}

/*
 * Digests the standard input and prints the result.
 */
static void
MDFilter(pipe)
	int pipe;
{
	MD5_CTX context;
	int     len;
	unsigned char buffer[BUFSIZ];
	char buf[33];

	MD5Init(&context);
	while ((len = fread(buffer, 1, BUFSIZ, stdin))) {
		if(pipe && (len != fwrite(buffer, 1, len, stdout))) {
			perror("stdout");
			exit(1);
		}
		MD5Update(&context, buffer, len);
	}
	printf("%s\n", MD5End(&context,buf));
}

static void
usage()
{

	fprintf(stderr, "usage: md5 [-ptx] [-s string] [files ...]\n");
	exit(1);
}
