# Makefile to allow for cross_compilation on ARM64 target as well as native Linux

CROSS_COMPILE = 
CC = gcc

all: writer.c
	$(CROSS_COMPILE)$(CC) -Wall -Werror writer.c -o writer

clean:
	rm -f *.o writer

