# Makefile for MC

.SUFFIXES:	.c .o .as .lib .y

CPM		 = cpm -h
CC	     = cpm -h c
AS	     = cpm -h zas
LINK	 = cpm -h link
LIBR	 = cpm -h libr
RM	     = rm
DEFINES  = -Dunix
CFLAGS	 = -O -x
ASFLAGS	 = -N
OBJ_MC   = cp.o ctrl.o dio.o fm.o ls.o m1.o mc.0 rm.o scan.o scr.o str.o wnd.o
OBJ_FF	 = ff.o ffp.o

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
#ytab.o : ytab.c 
#	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I ytab.c CTMP1.TMP
#	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
#	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
#	$(CPM) ZAS -X -N -oytab.o CTMP1.TMP
#	$(RM) CTMP1.TMP CTMP2.TMP 

libff.lib: $(OBJ_LIB)
	$(LIBR) r libff.lib $(OBJ_FF)
 
mc : $(OBJ_MC)
	$(LINK) -S -X -Z -Ptext=256,data,bss,bssend -C256 -Omc C0U.O $(OBJ_MC) LIBFF.LIB LIBC-UZI.LIB

all: libff.lib mc 

clean :
	$(RM) $(OBJ_MC) $(OBJ_FF)
