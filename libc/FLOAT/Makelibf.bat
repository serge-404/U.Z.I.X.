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

del libf-old.lib
ren libf-uzi.lib libf-old.lib

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

REM f-lib modules

echo make libf-uzi.lib

cpm libr r libf.lib FNUM.obj
cpm libr r libf.lib ATOF.obj
cpm libr r libf.lib MODF.obj
cpm libr r libf.lib ACOS.obj
cpm libr r libf.lib ASIN.obj
cpm libr r libf.lib ATAN2.obj
cpm libr r libf.lib ATAN.obj
cpm libr r libf.lib TAN.obj
cpm libr r libf.lib TANH.obj
cpm libr r libf.lib LOG.obj
cpm libr r libf.lib COS.obj
cpm libr r libf.lib COSH.obj
cpm libr r libf.lib SIN.obj
cpm libr r libf.lib SINH.obj
cpm libr r libf.lib SQRT.obj
cpm libr r libf.lib EXP.obj
cpm libr r libf.lib CEIL.obj
cpm libr r libf.lib FLOOR.obj
cpm libr r libf.lib EVALPOLY.obj
cpm libr r libf.lib FABS.obj

cpm libr r libf.lib FBCD.obj
cpm libr r libf.lib FINC.obj
cpm libr r libf.lib FRNDINT.obj
cpm libr r libf.lib FTOL.obj
cpm libr r libf.lib LTOF.obj
cpm libr r libf.lib ASFLOAT.obj
cpm libr r libf.lib FLOAT.obj
cpm libr r libf.lib FRELOP.obj
cpm libr r libf.lib FREXP.obj
cpm libr r libf.lib ALLSH.obj
cpm libr r libf.lib ALRSH.obj
cpm libr r libf.lib IREGSET.obj

ren libf.lib libf-uzi.lib

