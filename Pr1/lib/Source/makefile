#Makefile bibliotecas
CC=gcc
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)
LIBS=-lm 
TARGET=mitiempoTest.elf
TARGET_DEBUG=mitiempoTestDebug

CFLAGS=-Wall -g 


$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

debug:$(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET)
	-rm -f $(TARGET_DEBUG)
	-rm -f $(OBJS)
