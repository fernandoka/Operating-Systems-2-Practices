CC=gcc
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)
TARGETS=$(SRC:.c=.elf)
CFLAGS=-Wall #-g

server.elf: server.o
	$(CC) $(CFLAGS) -o $@ server.o

clean:
	-rm -f $(TARGETS)
	-rm -f $(OBJS)

