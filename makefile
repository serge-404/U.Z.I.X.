# Makefile for UZIX modules 

CPM     := cpm -C -h
CC       = $(CPM) C
AS       = $(CPM) zas
LINK     = $(CPM) link
LIBR     = $(CPM) libr
OBJHEX   = $(CPM) objtohex
M80      = $(CPM) m80n
L80      = $(CPM) l80m
CFLAGS	 = -O -x
ASFLAGS	 = -N
OBJ_MKFS = MD.OBJ FS.OBJ DMISC.OBJ DSK.OBJ DIO.OBJ DFD.OBJ DTTY.OBJ SC1.OBJ DATA.OBJ
OBJ_BD   = MD.OBJ FS.OBJ DMISC.OBJ DSK.OBJ DIO.OBJ DFD.OBJ DTTY.OBJ SC1.OBJ DATA.OBJ
OBJ_FSCK = MD.OBJ FS.OBJ DMISC.OBJ DSK.OBJ DIO.OBJ DFD.OBJ DTTY.OBJ SC1.OBJ DATA.OBJ 
OBJ_UCP  = MD.OBJ FS.OBJ DMISC.OBJ DSK.OBJ DIO.OBJ DFD.OBJ DTTY.OBJ SC1.OBJ SC2.OBJ XFS.OBJ UCS.OBJ DATA.OBJ 
OBJ_UZIX = MD.OBJ FS.OBJ DMISC.OBJ DSK.OBJ DIO.OBJ DFD.OBJ DTTY.OBJ SC1.OBJ SC2.OBJ SC3.OBJ PR1.OBJ PR2.OBJ SWP.OBJ
OBJ_FDSK = FDISK2.OBJ FDISK1.OBJ F_MKFS.OBJ DSK.OBJ

%.OBJ : %.C
	$(CC) $(CFLAGS) $(DEFINES) -c $*.C

%.OBJ : %.c
	$(CC) $(CFLAGS) $(DEFINES) -c $*.c

%.OBJ : %.AS 
	$(AS) $(ASFLAGS) -L$*.LST $*.AS

# compiled without optimization (-O) because optimizer spoiling #asm blocks :
MD.OBJ :
	$(CC) $(DEFINES) -x -c MD.C

# corecompiled (step-by-step) because no memory for $$EXEC (sc1.c too big)
SC1.OBJ : SC1.C 
	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I SC1.C CTMP1.TMP
	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
	$(CPM) OPTIM CTMP1.TMP CTMP2.TMP
	$(CPM) ZAS -X -J -N -osc1.obj CTMP2.TMP
	$(RM) CTMP1.TMP CTMP2.TMP 

# corecompiled (step-by-step) because no memory for $$EXEC (sc2.c too big)
SC2.OBJ : SC2.C
	$(CPM) CPP -DCPM -DHI_TECH_C -Dz80 $(DEFINES) -I SC2.C CTMP1.TMP
	$(CPM) P1 CTMP1.TMP CTMP2.TMP CTMP3.TMP
	$(CPM) CGEN CTMP2.TMP CTMP1.TMP
	$(CPM) OPTIM CTMP1.TMP CTMP2.TMP
	$(CPM) ZAS -X -J -N -osc2.obj CTMP2.TMP
	$(RM) CTMP1.TMP CTMP2.TMP 

idebdos.com :
	$(M80) idebdos,=idebdos
	$(L80) /p:100,idebdos,idebdos/n/e
	
emu.com : 
	$(M80) emu,=emu
	$(L80) /p:100,emu,emu/n/e

bd.com : DEFINES = -DORI_UTIL
bd.com : BD.C $(OBJ_BD)
	$(CC) $(CFLAGS) $(DEFINES) BD.C $(OBJ_BD)

fsck.com : DEFINES = -DORI_UTIL
fsck.com : FSCK.C $(OBJ_FSCK)
	$(CC) $(CFLAGS) $(DEFINES) FSCK.C $(OBJ_FSCK)

mkfs.com : DEFINES = -DORI_UTIL
mkfs.com : MKFS.C $(OBJ_MKFS)
	$(CC) $(CFLAGS) $(DEFINES) MKFS.C $(OBJ_MKFS)

ucp.com : DEFINES = -DORI_UTIL
ucp.com : UCP.C $(OBJ_UCP)
	$(CC) $(CFLAGS) $(DEFINES) UCP.C $(OBJ_UCP)

uzix.com : DEFINES = -DORI_UZIX
uzix.com : $(OBJ_UZIX)
	$(CC) -x $(DEFINES) UZIX.C $(OBJ_UZIX)

fdisk.com : DEFINES = -DORI_FDISK
fdisk.com : FDISK.C $(OBJ_FDSK)
	$(CC) $(CFLAGS) FDISK.C $(OBJ_FDSK)

.PHONY : help utils cutils kernel ckernel fdisk cfdisk all clean cleank cleanu cleanf

# set "DEFINES" to "-DORI_UTIL" and clean common OBJs before doing "make utils"
utils : idebdos.com bd.com fsck.com mkfs.com ucp.com 

# set "DEFINES" to "-DORI_UZIX" and clean common OBJs before doing "make kernel"
kernel : idebdos.com emu.com uzix.com

# set "DEFINES" to "-DORI_FDISK" and clean common OBJs before doing "make fdisk"
fdisk : fdisk.com

# build with cleanup
ckernel : cleank kernel

cutils : cleanu utils 

cfdisk : cleanf fdisk.com

# nested $(MAKE) used because main MAKE confuses with rebuild the same sources (other DEFINEs)
all : ckernel
	$(MAKE) cutils
	$(MAKE) cfdisk

clean : cleank cleanu cleanf

cleank :
	$(RM) $(OBJ_UZIX)

cleanu :
	$(RM) $(OBJ_UCP)

cleanf :
	$(RM) $(OBJ_FDSK)

help :
	@echo ""
	@echo "Usage: make [<mode>]"
	@echo "Available modes:"
	@echo "  kernel  - rebuild EMU.COM,IDEBDOS.COM,UZIX.COM with existing OBJs"
	@echo "  ckernel - clean&rebuild OBJs, build EMU.COM,IDEBDOS.COM,UZIX.COM"
	@echo "  fdisk   - rebuild FDISK.COM with existing OBJs"
	@echo "  cfdisk  - clean&rebuild OBJs, build fdisk.com"
	@echo "  utils   - rebuild IDEBDOS.COM,BD.COM,FSCK.COM,MKFS.COM,UCP.COM"
	@echo "  cutils  - clean&rebuild OBJs, build BD.COM,FSCK.COM,MKFS.COM,UCP.COM"	
	@echo "  all     - clean all OBJs and rebuild all the above COM files"
	@echo "  clean   - clean all OBJs and RELs"
	@echo ""

