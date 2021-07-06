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

all:$(TARGET) clean

%.o:%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(TARGET):$(OBJS)
	$(CC) -o $@ $^
	./$(TARGET)

clean:
	rm -rf $(TARGET) $(OBJS)
