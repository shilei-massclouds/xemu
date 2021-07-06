#
# Makefile
#

CC = gcc
CFLAGS = -Wall
LDFLAGS =

INC = -I./

TESTS = $(subst .c,.elf, $(wildcard *.c))

%.o:%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

%.elf:%.o
	$(CC) -o $@ $^
	./$@

all:${TESTS} clean

clean:
	rm -rf *.elf *.o
