OS=unix
# unix - compiles a version for linux/*BSD etc
# plan9 - compiles a version for plan9/9front
# plan9port - compiles a version for plan9port

TARGET=ns
OFILES=ns.o html.o render.o css.o net.o

.if ${OS} == "unix"
CFLAGS=-Wall -Wextra -std=c89 -ggdb -Wno-unused-variable\
			 `pkg-config --cflags x11 xft libcurl` \
			 -DUSE_COLOR \
			 -DUSE_X \
			 -DUSE_CURL \
			 -DDUMB_WARNINGS
LDFLAGS=`pkg-config --libs x11 xft libcurl`
.endif

.if ${OS} == "plan9"
echo untested - exiting.
exit
CFLAGS=\
	-DUSE_9 \
	-DDUMB_WARNINGS
.endif

.if ${OS} == "plan9port"
CC=9c
LD=9l
CFLAGS=\
	-DUSE_9 \
	-DDUMB_WARNINGS
.endif


all: $(OFILES)
.if ${OS} == "plan9" || ${OS} == "plan9port"
	$(LD) $(OFILES) -o $(TARGET) $(LDFLAGS)
.else
	$(CC) $(OFILES) -o $(TARGET) $(LDFLAGS)
.endif
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.o $(TARGET) *.core

