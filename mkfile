# no idea if it works

TARGET=ns

OFILES=ns.o html.o render.o css.o net.o

CFLAGS=\
	-DUSE_9 \
	-DDUMB_WARNINGS \
	-DFONTDIR9=\"/lib/font\" \
	-DFONTTYPE9=\"lucsans\" \
	-DFONTNAME9=\"typeunicode\" \
	-DFONTSIZE9=\"7\"

%.o: %.c
	$CC $CFLAGS -c $stem.c

all: $OFILES
	$LD $OFILES -o $TARGET $LDFLAGS
clean:
	rm -f *.o $TARGET

