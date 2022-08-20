TARGET=ns

CFLAGS=-Wall -Wextra -std=c89 -ggdb -Wno-unused-variable\
			 `pkg-config --cflags x11 xft libcurl` \
			 -DUSE_COLOR \
			 -DUSE_X \
			 -DUSE_CURL \
			 -DDUMB_WARNINGS
LDFLAGS=`pkg-config --libs x11 xft libcurl`
OFILES=ns.o html.o render.o css.o net.o

all: $(OFILES)
	$(CC) $(OFILES) -o $(TARGET) $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.o $(TARGET) *.core

