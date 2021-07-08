#
# Makefile
#

.PHONY: all clean

CC = gcc
CFLAGS = -Wall
LDFLAGS =

INC = -I./

TARGET = xemu

OBJS = $(subst .c,.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

all:$(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(TARGET):$(OBJS) $(HEADERS)
	$(CC) -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJS)
