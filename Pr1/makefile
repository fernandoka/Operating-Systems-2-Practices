CC=gcc
DIR=$(shell pwd)/lib
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)
LIBS_STATIC=-lmitiempo_s -lm 
LIBS_DYNAMIC=-lmitiempo_S -lm 
TARGET_DYNAMIC=mulMatrixDynamic.elf
TARGET_STATIC=mulMatrixStatic.elf
CFLAGS=-Wall -g -o1

$(TARGET_DYNAMIC): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) -L$(DIR) -Wl,-rpath=$(DIR) $(LIBS_DYNAMIC)

all: $(TARGET_DYNAMIC) $(TARGET_STATIC)

static: $(TARGET_STATIC)

$(TARGET_STATIC): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) -L$(DIR) $(LIBS_STATIC)

clean:
	-rm -f $(TARGET_STATIC)
	-rm -f $(TARGET_DYNAMIC)
	-rm -f $(OBJS)
