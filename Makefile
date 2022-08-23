OS=unix
# unix - compiles a version for linux/*BSD etc
# plan9 - compiles a version for plan9/9front
# plan9port - compiles a version for plan9port

TARGET=ns
OFILES=ns.o html.o render.o css.o net.o click.o

PLAN9PORT_BASEDIR=/usr/local/plan9
# on openbsd

.if ${OS} == "unix"
CFLAGS=-Wall -Wextra -std=c89 -ggdb -Wno-unused-variable\
			 `pkg-config --cflags x11 xft libcurl` \
			 -DUSE_COLOR \
			 -DUSE_X \
			 -DUSE_CURL \
			 -DDUMB_WARNINGS
LDFLAGS=`pkg-config --libs x11 xft libcurl`
.endif

.if ${OS} == "plan9port"
CC=9c
LD=9l
CFLAGS=\
	-DUSE_9 \
	-DDUMB_WARNINGS \
	-DPLAN9PORT_BASEDIR=\"$(PLAN9PORT_BASEDIR)\" \
	-DFONTDIR9=\"/usr/local/plan9/font\" \
	-DFONTTYPE9=\"lucsans\" \
	-DFONTNAME9=\"typeunicode\" \
	-DFONTSIZE9=\"7\"
.endif


all: $(OFILES)
.if ${OS} == "plan9" 
	mk
.else
	$(CC) $(OFILES) -o $(TARGET) $(LDFLAGS)
.endif
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.o $(TARGET) *.core

