del libvi.lib

call ucc -o -c display  libvi.lib

call ucc -o -c edit2    libvi.lib

call ucc -o -c edit1    libvi.lib

call ucc -o -c exec2    libvi.lib

call ucc -o -c exec1    libvi.lib

call ucc -o -c blockio  libvi.lib

call ucc -o -c find     libvi.lib

call ucc -o -c globals  libvi.lib

call ucc -o -c insert   libvi.lib

call ucc -o -c misc     libvi.lib

call ucc -o -c modify   libvi.lib

call ucc -o -c move     libvi.lib

call ucc -o -c ucsd     libvi.lib

call ucc -o -c undo     libvi.lib

call ucc -o -c unixcall libvi.lib

call ucc -o -c wildargs libvi.lib

call ucc -o main libvi.lib

del main.o
del vi.old
ren vi. vi.old
ren main. vi.
