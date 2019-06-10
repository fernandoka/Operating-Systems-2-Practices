CC=gcc
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)
TARGETS=$(SRC:.c=.elf)
CFLAGS=-Wall -g

dem.elf: dem.o
	$(CC) $(CFLAGS) -o $@ dem.o

clean:
	-rm -f $(TARGETS)
	-rm -f $(OBJS)

