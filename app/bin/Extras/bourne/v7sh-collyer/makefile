CFLAGS = -O # -w
OBJS=args.o builtin.o cmd.o ctype.o error.o expand.o fault.o io.o macro.o \
	main.o msg.o name.o print.o service.o stak.o string.o word.o xec.o
HDRS=ctype.h defs.h mac.h mode.h name.h stak.h sym.h timeout.h

all:	sh

cp:	sh
	cp sh /bin/sh
	rm sh *.o

cmp:	sh
	cmp sh /bin/sh
	rm sh *.o

sh:	args.o builtin.o cmd.o ctype.o error.o expand.o fault.o io.o macro.o \
	main.o msg.o name.o print.o service.o stak.o string.o word.o xec.o 
$(OBJS): $(HDRS)

sh:
	$(CC) -o $@ *.o

newsys:;	:newsys file=msg; make sh
install:;	:install
diff:;		:diff
list:;		list Makefile *.h *.s *.c

clean:
	rm -f sh *.o
