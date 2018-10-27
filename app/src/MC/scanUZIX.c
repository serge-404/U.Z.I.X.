/*---------------------------------------------------------------*/
/* FAT file system module test program R0.0. (c)2007 Serge       */
/*---------------------------------------------------------------*/

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stat.h>

#include "filemgr.h"
#include "stringz.h"

extern char buffer[MAX_BUFF+1];         /* char[512+1] */
extern FRESULT res; 
extern FILINFO finfo;

/* FAT File status structure */
/*typedef struct _FILINFO {*/
/*	DWORD fsize; */			/* Size */
/*	WORD fdate;  */			/* Date */
/*	WORD ftime;  */			/* Time */
#ifdef ORI_UZIX
/*	WORD fattrib;	*/		/* Attributes */
/*	char fname[DIRNAMELEN+1]; */	/* Name (UZIX format, DIRNAMELEN=14) */	
#else
/*	BYTE fattrib; */		/* Attribute */
/*	char fname[8+1+3+1]; */		/* Name (8.3 format) */
#endif
/*} FILINFO;*/

/* UZIX time (nonUNIX, FATalike) */
/*typedef struct s_time { */
/*	uint	t_time; */
/*	uint	t_date; */
/*} time_t; */

/* UZIX data structure for stat() */
/* struct stat {		*/	/* USER! Really only used by users */
/*	uint	st_dev; 	*/	/* device number */
/*	uint	st_ino; 	*/	/* inode number */
/*	mode_t	st_mode;	*/	/* file mode - uint */
/*	uint	st_nlink;	*/	/* number of links */
/*	uint	st_uid; 	*/	/* owner id */
/*	uint	st_gid; 	*/	/* owner group */
/*	uint	st_rdev;	*/	/* */
/*	off_t	st_size;	*/	/* file size - long */
/*	time_t	st_atime;	*/	/* last access time */
/*	time_t	st_mtime;	*/	/* last modification time */
/*	time_t	st_ctime;	*/	/* file creation time */
/*}; #define DIRNAMELEN	14 */

#ifdef ORI_UZIX

/*
char prts[8][4] = {
		"---", "--x", "-w-", "-wx",
		"r--", "r-x", "rw-", "rwx"
	};


#define prot(stat) (prts[(stat) & 7])

char devdir(stat)
	register int stat;
{
	stat &= S_IFMT;

	switch (stat) {
	case S_IFDIR:	return 'd';
	case S_IFPIPE:	return 'p';
	case S_IFBLK:	return 'b';
	case S_IFCHR:	return 'c';
	case S_IFLNK:	return 'l';
	}
	return '-';
}

char* strls() 
{
		if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
			strcat(dname, "/");
		if (S_ISDEV(statbuf.st_mode))
			PF("%3d,%-5d",
				statbuf.st_rdev >> 8,
				statbuf.st_rdev & 0xFF);
		else	PF("%-9ld", statbuf.st_size);
		PF(" %c%s%s%s %2d @%-5u %s",
		       devdir(statbuf.st_mode),
		       prot(statbuf.st_mode >> 6),
		       prot(statbuf.st_mode >> 3),
		       prot(statbuf.st_mode >> 0),
		       statbuf.st_nlink,
		       statbuf.st_ino,
		       dname);
		if ((statbuf.st_mode & S_IFMT) == S_IFLNK) {
			dname[0] = 0;
			i = read(fd, dname, sizeof(dname)-1);
			dname[i] = 0;
			PF(" -> %s", dname);
			close(fd);
		}
}
*/

struct stat statbuf;

FRESULT scanUZIX(char* path, dir_callback OnFile)
{
	int d, i;
        char dname0;
	register int fd;
	direct_t buf;

	if ((! path) || (! *path) || (stat(path, &statbuf) != 0) || ((statbuf.st_mode & S_IFMT) != S_IFDIR) || ((d = open(path, 0)) < 0))
		return FR_INVALID_OBJECT;
	while (read(d, (char *) &buf, sizeof(direct_t)) == sizeof(direct_t)) {
                dname0 = *buf.d_name;
		if ( !dname0 || ( !buf.d_name[1] && (dname0='.')) ||  /* empty buf.d_name or buf.d_name="." */
                     (!path[1] && (!strcmp((char*)buf.d_name,".."))) )       /* or ( path="/" and buf.d_name="..") */
			continue;
		*buffer = '\0';
		strcpy(buffer, path);
		if (path[1]) strcat(buffer, "/");
		strncat(buffer, (char*)buf.d_name, DIRNAMELEN);
		fd = open(buffer, O_SYMLINK);
		i = (fd >= 0) ? fstat(fd, &statbuf) :
				stat(buffer, &statbuf);
		if (i) {
			if (fd >= 0)
				close(fd);
			continue;
		}
		finfo.fsize=statbuf.st_size;
		finfo.fdate=statbuf.st_mtime.t_date;
		finfo.ftime=statbuf.st_mtime.t_time;
		finfo.uattrib=statbuf.st_mode;
                finfo.fattrib=((statbuf.st_mode & S_IFMT) & S_IFDIR ? AM_DIR : 0);
		strncpy(finfo.fname,(char*)buf.d_name,DIRNAMELEN);
		finfo.fname[DIRNAMELEN]=0;

		res=OnFile(&finfo);
		if (res!=FR_OK) break;
	}
	close(d);
	return FR_OK; 
}

#endif
