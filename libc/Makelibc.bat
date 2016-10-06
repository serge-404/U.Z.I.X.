@echo off

REM  Compile without optimization example
REM
REM  c:\SDCC\_HITECH>cpm -h c -v hello.c
REM  HI-TECH C COMPILER (CP/M-80) V3.09
REM  REM Copyright (C) 1984-87 HI-TECH SOFTWARE
REM  0:CPP -DCPM -DHI_TECH_C -Dz80 -I HELLO.C $CTMP1.$$$
REM  0:P1 $CTMP1.$$$ $CTMP2.$$$ $CTMP3.$$$
REM  0:CGEN $CTMP2.$$$ $CTMP1.$$$
REM  0:ZAS -N -oHELLO.OBJ $CTMP1.$$$
REM  ERA $CTMP1.$$$
REM  ERA $CTMP2.$$$
REM  ERA $CTMP3.$$$
REM  0:LINK -Z -Ptext=0,data,bss -C100H -OHELLO.COM CRTCPM.OBJ HELLO.OBJ LIBC.LIB
REM  ERA HELLO.OBJ
REM  ERA $$EXEC.$$$

REM  Compile with optimization example
REM
REM  c:\SDCC\_HITECH>cpm -h c -v -o -x hello.c
REM  HI-TECH C COMPILER (CP/M-80) V3.09
REM  Copyright (C) 1984-87 HI-TECH SOFTWARE
REM  0:CPP -DCPM -DHI_TECH_C -Dz80 -I HELLO.C $CTMP1.$$$
REM  0:P1 $CTMP1.$$$ $CTMP2.$$$ $CTMP3.$$$
REM  0:CGEN $CTMP2.$$$ $CTMP1.$$$
REM  0:OPTIM $CTMP1.$$$ $CTMP2.$$$
REM  REM 0:ZAS -X -J -N -oHELLO.OBJ $CTMP2.$$$
REM  ERA $CTMP1.$$$
REM  ERA $CTMP2.$$$
REM  ERA $CTMP3.$$$
REM  0:LINK -Z -X -Ptext=0,data,bss -C100H -OHELLO.COM CRTCPM.OBJ HELLO.OBJ LIBC.LIB
REM  ERA HELLO.OBJ
REM  ERA $$EXEC.$$$

del libc-old.lib
ren libc.lib libc-old.lib

for %%i in (*.AS) do (
   REM  compile main C-code ASM-files to OBJ-files
   echo %%i
   cpm zas -X -N %%i
)

for %%i in (*.AS_) do (
   REM  compile UXIZ syscalls ASM-files to OBJ-files
   echo %%i
   cpm zas -X -N %%i
)

for %%i in (*.C) do (
   REM  compile C-files to OBJ-files with optimization
   echo %%i
   cpm -h c -o -x -DMAKE_ALL -c %%i
)

for %%i in (*.CC) do (
   REM  compile C-files to OBJ-files without optimization
   echo %%i
   cpm CPP -DCPM -DHI_TECH_C -Dz80 -DMAKE_ALL -I %%i $CTMP1.$$$
   cpm P1 $CTMP1.$$$ $CTMP2.$$$ $CTMP3.$$$
   cpm CGEN $CTMP2.$$$ $CTMP1.$$$
   cpm ZAS -X -N -o%%~ni.obj $CTMP1.$$$
   del $CTMP1.$$$ $CTMP2.$$$  
)

REM c-lib modules

echo make libc-uzi.lib

cpm libr r libc.lib ASSERT.obj
cpm libr r libc.lib CRYPT.obj
cpm libr r libc.lib INITGRUP.obj
cpm libr r libc.lib FGETGREN.obj
cpm libr r libc.lib FGETPWEN.obj
cpm libr r libc.lib GETGRGID.obj
cpm libr r libc.lib GETGRNAM.obj
cpm libr r libc.lib GETPW.obj
cpm libr r libc.lib GETPWNAM.obj
cpm libr r libc.lib GETPWUID.obj
cpm libr r libc.lib GETPWENT.obj
cpm libr r libc.lib SETPWENT.obj
cpm libr r libc.lib PUTPWENT.obj
cpm libr r libc.lib SETGRENT.obj
cpm libr r libc.lib GETGRENT.obj
cpm libr r libc.lib PERROR.obj
cpm libr r libc.lib ERROR.obj
cpm libr r libc.lib SYSTEM.obj
cpm libr r libc.lib REGEXP.obj
cpm libr r libc.lib REGSUB.obj
cpm libr r libc.lib REGERROR.obj
cpm libr r libc.lib PRINTF.obj
cpm libr r libc.lib FPRINTF.obj
cpm libr r libc.lib SPRINTF.obj
cpm libr r libc.lib VPRINTF.obj
cpm libr r libc.lib VSPRINTF.obj
cpm libr r libc.lib VFPRINTF.obj
cpm libr r libc.lib SCANF.obj
cpm libr r libc.lib FSCANF.obj
cpm libr r libc.lib SSCANF.obj
cpm libr r libc.lib VSCANF.obj
cpm libr r libc.lib VSSCANF.obj
cpm libr r libc.lib VFSCANF.obj
cpm libr r libc.lib GETS.obj
cpm libr r libc.lib FPUTS.obj
cpm libr r libc.lib FPUTC.obj
cpm libr r libc.lib LSTAT.obj
cpm libr r libc.lib POPEN.obj
cpm libr r libc.lib PUTENV.obj
cpm libr r libc.lib REWIND.obj
cpm libr r libc.lib RENAME.obj
cpm libr r libc.lib UNGETC.obj
cpm libr r libc.lib TERMCAP.obj
cpm libr r libc.lib TMPNAM.obj
cpm libr r libc.lib TPARAM.obj
cpm libr r libc.lib TTYNAME.obj
cpm libr r libc.lib GETOPT.obj
cpm libr r libc.lib GETPASS.obj
cpm libr r libc.lib PUTGETCH.obj
cpm libr r libc.lib GETCWD.obj
cpm libr r libc.lib FTELL.obj
cpm libr r libc.lib FWRITE.obj
cpm libr r libc.lib FGETS.obj
cpm libr r libc.lib FGETC.obj
cpm libr r libc.lib FREAD.obj
cpm libr r libc.lib FOPEN.obj
cpm libr r libc.lib FCLOSE.obj
cpm libr r libc.lib STDIO0.obj
cpm libr r libc.lib SETBUFF.obj
cpm libr r libc.lib SETVBUFF.obj
cpm libr r libc.lib FFLUSH.obj
cpm libr r libc.lib ISATTY.obj
cpm libr r libc.lib SLEEP.obj
cpm libr r libc.lib UTSNAME.obj
cpm libr r libc.lib EXECLE.obj
cpm libr r libc.lib EXECLP.obj
cpm libr r libc.lib EXECLPE.obj
cpm libr r libc.lib EXECT.obj
cpm libr r libc.lib EXECV.obj
cpm libr r libc.lib EXECVP.obj
cpm libr r libc.lib EXECVPE.obj
cpm libr r libc.lib EXECL.obj
cpm libr r libc.lib ABORT.obj
cpm libr r libc.lib CTIME.obj
cpm libr r libc.lib DIFFTIME.obj
cpm libr r libc.lib LOCALTIM.obj
cpm libr r libc.lib ASCTIME.obj
cpm libr r libc.lib BSEARCH.obj
cpm libr r libc.lib CLOCK.obj
cpm libr r libc.lib XITOA.obj
cpm libr r libc.lib XLTOA.obj
cpm libr r libc.lib LTOSTR.obj
cpm libr r libc.lib LTOA.obj
cpm libr r libc.lib ULTOA.obj
cpm libr r libc.lib ITOA.obj
cpm libr r libc.lib STRTOD.obj
cpm libr r libc.lib STRTOK.obj
cpm libr r libc.lib STRCAT.obj
cpm libr r libc.lib STRCPY.obj
cpm libr r libc.lib STRCSPN.obj
cpm libr r libc.lib STRDUP.obj
cpm libr r libc.lib STRICMP.obj
cpm libr r libc.lib STRNCAT.obj
cpm libr r libc.lib STRNCMP.obj
cpm libr r libc.lib STRNCPY.obj
cpm libr r libc.lib STRNICMP.obj
cpm libr r libc.lib STRSEP.obj
cpm libr r libc.lib STRSPN.obj
cpm libr r libc.lib STRSTR.obj
cpm libr r libc.lib STRPBRK.obj
cpm libr r libc.lib STRCHR.obj
cpm libr r libc.lib STRCMP.obj
cpm libr r libc.lib STRRCHR.obj
cpm libr r libc.lib READLINK.obj
cpm libr r libc.lib READDIR.obj
cpm libr r libc.lib CLOSEDIR.obj
cpm libr r libc.lib MKDIR.obj
cpm libr r libc.lib OPENDIR.obj
cpm libr r libc.lib REWINDIR.obj
cpm libr r libc.lib RMDIR.obj
cpm libr r libc.lib MKTIME.obj
cpm libr r libc.lib CONVTIME.obj
cpm libr r libc.lib GMTIME.obj
cpm libr r libc.lib TZSET.obj
cpm libr r libc.lib GETENV.obj
cpm libr r libc.lib SETENV.obj
cpm libr r libc.lib REALLOC.obj
cpm libr r libc.lib CALLOC.obj
cpm libr r libc.lib MALLOC.obj
cpm libr r libc.lib ALLOCA.obj
cpm libr r libc.lib FREE.obj
cpm libr r libc.lib ATOI.obj
cpm libr r libc.lib ATOL.obj
cpm libr r libc.lib QSORT.obj
cpm libr r libc.lib RAND.obj
cpm libr r libc.lib STRTOL.obj
cpm libr r libc.lib STRTOUL.obj
cpm libr r libc.lib Strlen.obj
cpm libr r libc.lib ATEXIT.obj
cpm libr r libc.lib ctype.obj
cpm libr r libc.lib MEMCCPY.obj
cpm libr r libc.lib MEMCHR.obj
cpm libr r libc.lib memcmp.obj
cpm libr r libc.lib memcpy.obj
cpm libr r libc.lib memmove.obj
cpm libr r libc.lib memset.obj
cpm libr r libc.lib Setjmp.obj
cpm libr r libc.lib ncsv.obj
cpm libr r libc.lib rcsv.obj

REM math & main

cpm libr r libc.lib ASALLSH.obj
cpm libr r libc.lib ASALRSH.obj
cpm libr r libc.lib ASAR.obj
cpm libr r libc.lib ASDIV.obj
cpm libr r libc.lib ASLADD.obj
cpm libr r libc.lib ASLAND.obj
cpm libr r libc.lib ASLL.obj
cpm libr r libc.lib ASLLRSH.obj
cpm libr r libc.lib ASLMUL.obj
cpm libr r libc.lib ASLOR.obj
cpm libr r libc.lib ASLR.obj
cpm libr r libc.lib ASLSUB.obj
cpm libr r libc.lib ASLXOR.obj
cpm libr r libc.lib ASMOD.obj
cpm libr r libc.lib ASMUL.obj
cpm libr r libc.lib ALLSH.obj
cpm libr r libc.lib ALRSH.obj
cpm libr r libc.lib BITFIELD.obj
cpm libr r libc.lib BRELOP.obj
cpm libr r libc.lib LRELOP.obj
cpm libr r libc.lib WRELOP.obj
cpm libr r libc.lib frelop.obj
cpm libr r libc.lib IDIV.obj
cpm libr r libc.lib IMUL.obj
cpm libr r libc.lib IREGSET.obj
cpm libr r libc.lib LADD.obj
cpm libr r libc.lib LAND.obj
cpm libr r libc.lib LDIV.obj
cpm libr r libc.lib LINC.obj
cpm libr r libc.lib LINCHL.obj
cpm libr r libc.lib LLRSH.obj
cpm libr r libc.lib LMUL.obj
cpm libr r libc.lib LOR.obj
cpm libr r libc.lib LSUB.obj
cpm libr r libc.lib LXOR.obj
cpm libr r libc.lib PSHLNG.obj
cpm libr r libc.lib SHAR.obj
cpm libr r libc.lib SHLL.obj
cpm libr r libc.lib SHLR.obj

REM syscalls

cpm libr r libc.lib Access.obj
cpm libr r libc.lib Alarm.obj
cpm libr r libc.lib Brk.obj
cpm libr r libc.lib Chdir.obj
cpm libr r libc.lib Chmod.obj
cpm libr r libc.lib Chown.obj
cpm libr r libc.lib Chroot.obj
cpm libr r libc.lib Close.obj
cpm libr r libc.lib Creat.obj
cpm libr r libc.lib Dup.obj
cpm libr r libc.lib Dup2.obj
cpm libr r libc.lib Execve.obj
cpm libr r libc.lib Fork.obj
cpm libr r libc.lib Fstat.obj
cpm libr r libc.lib Getegid.obj
cpm libr r libc.lib Geteuid.obj
cpm libr r libc.lib Getfsys.obj
cpm libr r libc.lib Getgid.obj
cpm libr r libc.lib Getpid.obj
cpm libr r libc.lib Getppid.obj
cpm libr r libc.lib Getprio.obj
cpm libr r libc.lib Getuid.obj
cpm libr r libc.lib Ioctl.obj
cpm libr r libc.lib Kill.obj
cpm libr r libc.lib Link.obj
cpm libr r libc.lib Lseek.obj
cpm libr r libc.lib Mkfifo.obj
cpm libr r libc.lib Mknod.obj
cpm libr r libc.lib Mount.obj
cpm libr r libc.lib Open.obj
cpm libr r libc.lib Pause.obj
cpm libr r libc.lib Pipe.obj
cpm libr r libc.lib Read.obj
cpm libr r libc.lib Reboot.obj
cpm libr r libc.lib Sbrk.obj
cpm libr r libc.lib Setgid.obj
cpm libr r libc.lib Setprio.obj
cpm libr r libc.lib Setuid.obj
cpm libr r libc.lib Signal.obj
cpm libr r libc.lib Stat.obj
cpm libr r libc.lib Stime.obj
cpm libr r libc.lib Symlink.obj
cpm libr r libc.lib Sync.obj
cpm libr r libc.lib Systrace.obj
cpm libr r libc.lib Time.obj
cpm libr r libc.lib Times.obj
cpm libr r libc.lib Umask.obj
cpm libr r libc.lib Umount.obj
cpm libr r libc.lib Unlink.obj
cpm libr r libc.lib Utime.obj
cpm libr r libc.lib Waitpid.obj
cpm libr r libc.lib Write.obj

ren libc.lib libc-uzi.lib

REM  compile starting unit (C0U.OBJ=CRTCPM.OBJ)
echo c0u.asm
cpm zas -X -N -oc0u.O c0u.asm


