del libff.lib
del libscr.lib
del libmenu.lib

call ucc -o -c screen.c   libscr.lib

call ucc -o -c windows.c  libscr.lib

call ucc -o -c controls.c libscr.lib


call ucc -o -c ffp.c    libff.lib

call ucc -o -c ff.c     libff.lib

call ucc -o -c diskio.c libff.lib


call ucc -o -c stringz.c libmenu.lib 

call ucc -o -c scanFAT.c libmenu.lib 

call ucc -o -c scanCPM.c libmenu.lib 

call ucc -o -c scanUZIX.c libmenu.lib 

call ucc -o -c filemgr.c libmenu.lib 

call ucc -o -c menu2.c   libmenu.lib 

call ucc -o -c menu1.c   libmenu.lib 


call ucc -o fat.c libff.lib libmenu.lib libscr.lib

del fat.o
del mc.old
ren mc. mc.old
ren fat. mc.

