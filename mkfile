# change for your system
CC=6c
LD=6l

TARGET=ns

OFILES=ns.o html.o render.o css.o net.o click.o console.o

CFLAGS=-FBwp -DUSE_9 -DDUMB_WARNINGS -DUSE_CONSOLE

%.o: %.c
	$CC $CFLAGS -o $stem.o -c $stem.c

all: $OFILES
	$LD -o $TARGET $LDFLAGS $OFILES
nuke:
	rm -f *.o *.cpp.c $TARGET

