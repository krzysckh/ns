CFLAGS=-Wall -Wextra -std=c89 -ggdb
LDFLAGS=
OFILES=ns.o html.o

all: $(OFILES)
	$(CC) $(OFILES) -o ns $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.o ns

