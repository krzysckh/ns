TARGET=ns

CFLAGS=-Wall -Wextra -std=c89 -ggdb -DUSE_X `pkg-config --cflags x11 xft`
LDFLAGS=`pkg-config --libs x11 xft`
OFILES=ns.o html.o render.o

all: $(OFILES)
	$(CC) $(OFILES) -o $(TARGET) $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.o $(TARGET) *.core

