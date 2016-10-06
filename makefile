# Makefile for UZIX modules 

.SUFFIXES:	.c .obj .as .lib

CPM		 = cpm -h
CC	     = cpm -h c
AS	     = cpm -h zas
LINK	 = cpm -h link
LIBR	 = cpm -h libr
OBJHEX	 = cpm -h objtohex
M80      = cpm m80n
L80      = cpm l80m
RM	     = rm
#DEFINES = -DORI_UTIL
#DEFINES = -DORI_UZIX
DEFINES = -DORI_FDISK
CFLAGS	 = -O -x
ASFLAGS	 = -N
OBJ_MKFS = md.obj fs.obj dmisc.obj dsk.obj dio.obj dfd.obj dtty.obj sc1.obj data.obj
OBJ_BD   = md.obj fs.obj dmisc.obj dsk.obj dio.obj dfd.obj dtty.obj sc1.obj data.obj
OBJ_FSCK = md.obj fs.obj dmisc.obj dsk.obj dio.obj dfd.obj dtty.obj sc1.obj data.obj 
OBJ_UCP  = md.obj fs.obj dmisc.obj dsk.obj dio.obj dfd.obj dtty.obj sc1.obj sc2.obj XFS.obj ucs.obj data.obj 
OBJ_UZIX = md.obj fs.obj dmisc.obj dsk.obj dio.obj dfd.obj dtty.obj sc1.obj sc2.obj sc3.obj pr1.obj pr2.obj swp.obj
OBJ_FDSK = fdisk2.obj fdisk1.obj f_mkfs.obj dsk.obj

.c.obj :
	$(CC) $(CFLAGS) $(DEFINES) -c $*.c

.as.obj : 
	$(AS) $(ASFLAGS) -L$*.lst $*.as

# compiled without optimization (-O) because optimizer spoiling #asm blocks :
md.obj : md.c machdep.orn machdep2.orn
	$(CC) $(DEFINES) -x -c md.c

# corecompiled (step-by-step) because no memory for $$EXEC (sc1.c too big)
sc1.obj : sc1.c 
	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I sc1.c CTMP1.TMP
	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
	$(CPM) OPTIM CTMP1.TMP CTMP2.TMP
	$(CPM) ZAS -X -J -N -osc1.obj CTMP2.TMP
	$(RM) CTMP1.TMP CTMP2.TMP 

# corecompiled (step-by-step) because no memory for $$EXEC (sc2.c too big)
sc2.obj : sc2.c 
	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I sc2.c CTMP1.TMP
	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
	$(CPM) OPTIM CTMP1.TMP CTMP2.TMP
	$(CPM) ZAS -X -J -N -osc2.obj CTMP2.TMP
	$(RM) CTMP1.TMP CTMP2.TMP 

idebdos.com : idebdos.mac
	$(M80) idebdos,=idebdos
	$(L80) /p:100,idebdos,idebdos/n/e
	
bd.com : $(OBJ_BD) bd.c
	$(CC) $(CFLAGS) $(DEFINES) bd.c $(OBJ_BD)

fsck.com : $(OBJ_FSCK) fsck.c
	$(CC) $(CFLAGS) $(DEFINES) fsck.c $(OBJ_FSCK)

mkfs.com : $(OBJ_MKFS) mkfs.c
	$(CC) $(CFLAGS) $(DEFINES) mkfs.c $(OBJ_MKFS)

ucp.com : $(OBJ_UCP) ucp.c 
	$(CC) $(CFLAGS) $(DEFINES) ucp.c $(OBJ_UCP)

uzix.com : $(OBJ_UZIX) uzix.c dispatch.c
	$(CC) -x $(DEFINES) uzix.c $(OBJ_UZIX)

fdisk.com : $(OBJ_FDSK) fdisk.c
	$(CC) $(CFLAGS) fdisk.c $(OBJ_FDSK)

# set "DEFINES" to "-DORI_UTIL" and do "make clean" before doing "make utils" !
utils : idebdos.com bd.com fsck.com mkfs.com ucp.com 

# set "DEFINES" to "-DORI_UZIX" and do "make clean" before doing "make kernel" !
kernel : idebdos.com uzix.com

clean :
	$(RM) $(OBJ_UZIX) data.obj XFS.obj ucs.obj bd.obj fsck.obj mkfs.obj ucp.obj uzix.obj F_MKFS.OBJ FDISK.OBJ FDISK1.OBJ FDISK2.OBJ
