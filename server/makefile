INCLUDE_DIRS = 
LIB_DIRS = 

CROSS_COMPILE = 

CC ?= $(CROSS_COMPILE)gcc

CDEFS=
CFLAGS?= -g -Wall -Werror $(INCLUDE_DIRS) $(CDEFS)
LDFLAGS?= -lpthread -lrt
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
	$(CC) aesdsocket.c $(LDFLAGS) $(CFLAGS) -o $@

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<