OS=unix
# unix - compiles a version for linux/*BSD etc
# plan9 - compiles a version for plan9/9front
# plan9port - compiles a version for plan9port

TARGET=ns
OFILES=ns.o html.o render.o css.o net.o click.o console.o

PLAN9PORT_BASEDIR=/usr/local/plan9
# on openbsd

.if ${OS} == "unix"
CFLAGS=-Wall -Wextra -std=c89 -ggdb -Wno-unused-variable -Wno-switch \
       -Wno-unused-parameter \
	`pkg-config --cflags x11 xft libcurl` \
	-DUSE_COLOR \
	-DUSE_X \
	-DUSE_CURL \
	-DUSE_CONSOLE \
	-DNO_WITH_HSL \
	-DDUMB_WARNINGS
LDFLAGS=`pkg-config --libs x11 xft libcurl` -lm
.endif

.if ${OS} == "plan9port"
CC=9c
LD=9l
CFLAGS=\
	-DUSE_9 \
	-DDUMB_WARNINGS \
	-DPLAN9PORT_BASEDIR=\"$(PLAN9PORT_BASEDIR)\" \
	-DFONTDIR9=\"$(PLAN9PORT_BASEDIR)/font\" \
	-DFONTTYPE9=\"lucsans\" \
	-DFONTNAME9=\"typeunicode\" \
	-DFONTSIZE9=\"7\"\
	-DWITH_HSL \
	-DUSE_CONSOLE
.endif


all: $(OFILES)
.if ${OS} == "plan9" 
	mk
.elif ${OS} == "plan9port"
	$(LD) $(OFILES) -o $(TARGET) $(LDFLAGS)
.else
	$(CC) $(OFILES) -o $(TARGET) $(LDFLAGS)
.endif
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.o $(TARGET) *.core

