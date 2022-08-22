# change for your system
CC=8c
LD=8l

TARGET=ns

OFILES=ns.cpp.o html.cpp.o render.cpp.o css.cpp.o net.cpp.o

CPPFLAGS=-DUSE_9 -DDUMB_WARNINGS

%.cpp.c: %.c
	cpp $CPPFLAGS $stem.c > $stem.cpp.c
%.cpp.o: %.cpp.c
	$CC $CFLAGS -o $stem.cpp.o -c $stem.cpp.c

all: $OFILES
	$LD -o $TARGET $LDFLAGS $OFILES
clean:
	rm -f *.o *.cpp.c $TARGET

