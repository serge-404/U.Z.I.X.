Консольные Windows32-утилиты для работы с образами дискет (*.DSK) и MBR-образами жестких дисков (*.OHI) с файловой системой UZIX в образах.

fsck example:
=============
REM
REM  HDD partition 1 image fsck
REM
c:\Temp\WCX_BIN>fsck 1:c:\Temp\WCX_BIN\uzix-ori.ohi

fsck - UZIX utility for filesystem image check, V1.0.

Checking drive `1:c:\Temp\WCX_BIN\uzix-ori.ohi` with fsize 1440 blocks, isize 25
 blocks, rsize 0 blocks. Confirm? y

Pass 1: Checking inodes.
Inode 184 with mode 0173366 is not of correct type. Zap? y
Inode 185 with mode 0173366 is not of correct type. Zap? y
Pass 2: Rebuilding free list.
        Rebuild free list? y
Pass 3: Checking block allocation.
Pass 4: Checking directory entries.
Pass 5: Checking link counts.
Done.

REM
REM  HDD partition 0 imsge fsck
REM
c:\Temp\WCX_BIN>
c:\Temp\WCX_BIN>fsck 0:c:\Temp\WCX_BIN\uzix-ori.ohi

fsck - UZIX utility for filesystem image check, V1.0.

Checking drive `0:c:\Temp\WCX_BIN\uzix-ori.ohi` with fsize 1440 blocks, isize 25
 blocks, rsize 60 blocks. Confirm? y

Pass 1: Checking inodes.
Pass 2: Rebuilding free list.
        Rebuild free list? y
Pass 3: Checking block allocation.
Pass 4: Checking directory entries.
Pass 5: Checking link counts.
Done.

REM
REM  FDD image fsck
REM
c:\Temp\WCX_BIN>
c:\Temp\WCX_BIN>fsck -y c:\Temp\WCX_BIN\uzix.udi

fsck - UZIX utility for filesystem image check, V1.0.

Checking drive `c:\Temp\WCX_BIN\uzix.udi` with fsize 1440 blocks, isize 25 block
s, rsize 60 blocks. Confirm? Y

Pass 1: Checking inodes.
Pass 2: Rebuilding free list.
        Rebuild free list? Y
Pass 3: Checking block allocation.
Pass 4: Checking directory entries.
Pass 5: Checking link counts.
Done.
