CC=gcc
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)
TARGETS=$(SRC:.c=.elf)
CFLAGS=-Wall #-g


cliente.elf: cliente.o
	$(CC) $(CFLAGS) -o $@ cliente.o

clean:
	-rm -f $(TARGETS)
	-rm -f $(OBJS)

