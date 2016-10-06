# Makefile for AWK

.SUFFIXES:	.c .o .as .lib

CPM		 = cpm -h
CC	     = cpm -h c
AS	     = cpm -h zas
LINK	 = cpm -h link
LIBR	 = cpm -h libr
RM	     = rm
DEFINES  = -DORI
CFLAGS	 = -O -x
ASFLAGS	 = -N
OBJ_AWK1 = main.o b.o lex.o lib.o parse.o tran.o ytab.o
OBJ_AWK2 = run.o run0.o run1.o run2.o

.as.o : 
	$(AS) $(ASFLAGS) -L$*.lst $*.as

# corecompiled (step-by-step) with optimization
.c.o :
	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I $*.c CTMP1.TMP
	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
	$(CPM) OPTIM CTMP1.TMP CTMP2.TMP
	$(CPM) ZAS -X -J -N -o$*.o CTMP2.TMP
	$(RM) CTMP1.TMP CTMP2.TMP 

# corecompiled (step-by-step) without optimization
ytab.o : ytab.c 
	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I ytab.c CTMP1.TMP
	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
	$(CPM) ZAS -X -N -oytab.o CTMP1.TMP
	$(RM) CTMP1.TMP CTMP2.TMP 

awk.com : $(OBJ_AWK1) $(OBJ_AWK2)
	$(LIBR) R LR.LIB $(OBJ_AWK2)
	$(LINK) -S -X -Z -Ptext=256,data,bss,bssend -C256 -Oawk.com C0U.O $(OBJ_AWK1) LR.LIB LF.LIB LC.LIB

all : awk.com

clean :
	$(RM) $(OBJ_AWK)
