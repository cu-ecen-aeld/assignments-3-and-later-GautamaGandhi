INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc
CROSS_COMPILE = 

CDEFS=
CFLAGS= -O0 -g -Wall -Werror $(INCLUDE_DIRS) $(CDEFS)
LDFLAGS= -pthread -lrt
LIBS= 

HFILES= 
CFILES= .c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	aesdsocket

clean:
	-rm -f *.o *.d
	-rm -f aesdsocket

aesdsocket: aesdsocket.c
	$(CROSS_COMPILE)$(CC) aesdsocket.c $(LDFLAGS) $(CFLAGS) -o $@

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<