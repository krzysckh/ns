TARGET=ns

CFLAGS=-Wall -Wextra -std=c89 -ggdb -DUSE_COLOR -DUSE_X -Wno-unused-variable\
			 -DUSE_CURL `pkg-config --cflags x11 xft libcurl` \
			 #-fsanitize=undefined -fsanitize=nullability
LDFLAGS=`pkg-config --libs x11 xft libcurl`
OFILES=ns.o html.o render.o css.o net.o

all: $(OFILES)
	$(CC) $(OFILES) -o $(TARGET) $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.o $(TARGET) *.core

