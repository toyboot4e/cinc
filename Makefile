SRCS=$(wildcard *.c)
HEADERS=$(wildcard *.h)
OBJS=$(SRCS:.c=.o)

MAIN_OBJ = cinc

ROOT = /Users/toy/dev/c/cinc
DOCK = docker run --rm -v "${ROOT}:/cinc" -w /cinc compilerbook

CC = ${DOCK} cc
CFLAGS=-std=c11 -g -static

$(MAIN_OBJ): $(OBJS)
		$(CC) -o $(MAIN_OBJ) $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADERS)

test: ${MAIN_OBJ}
		$(DOCK) ./test

clean:
		${DOCK} rm -f ${MAIN_OBJ} *.o *~ tmp*


.PHONY: test clean
