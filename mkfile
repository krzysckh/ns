# no idea if it works

TARGET=ns

OFILES=ns.cpp.o html.cpp.o render.cpp.o css.cpp.o net.cpp.o

CPPFLAGS=-DUSE_9 -DDUMB_WARNINGS

%.cpp.c: %.c
	cpp $CPPFLAGS $stem.c > $stem.cpp.c
%.cpp.o: %.cpp.c
	$CC $CFLAGS -c $stem.c

all: $OFILES
	$LD $OFILES -o $TARGET $LDFLAGS
clean:
	rm -f *.o $TARGET

