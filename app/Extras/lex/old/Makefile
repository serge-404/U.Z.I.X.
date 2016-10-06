# Makefile for LEX

.SUFFIXES:	.c .o .as .lib .y

CPM		 = cpm -h
CC	     = cpm -h c
AS	     = cpm -h zas
LINK	 = cpm -h link
LIBR	 = cpm -h libr
YACC     = cpm -h yacc
RM	     = rm
DEFINES  = -DSMALL -Dunix
CFLAGS	 = -O -x
ASFLAGS	 = -N
OBJ_LEX  = lmain.o ytab.o sub1.o sub2.o header.o
OBJ_LIB  = allprint.o main.o reject.o yyless.o yywrap.o 

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

ytab.c: parser.y
	$(YACC) parser.y

# corecompiled (step-by-step) without optimization
ytab.o : ytab.c 
	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I ytab.c CTMP1.TMP
	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
	$(CPM) ZAS -X -N -oytab.o CTMP1.TMP
	$(RM) CTMP1.TMP CTMP2.TMP 

#ytab.o: ytab.c ldefs.h

lmain.o:lmain.c ldefs.h once.c

sub1.o: sub1.c ldefs.h

sub2.o: sub2.c ldefs.h

header.o: header.c ldefs.h

libln.a: $(OBJ_LIB)
	$(LIBR) r libln.lib $(OBJ_LIB)
 
lex : $(OBJ_LEX)
	$(LINK) -S -X -Z -Ptext=256,data,bss,bssend -C256 -Olex C0U.O $(OBJ_LEX) LIBC-UZI.LIB

all: lex libln.a

clean :
	$(RM) $(OBJ_LEX) $(OBJ_LIB)
