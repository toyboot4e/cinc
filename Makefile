CFLAGS=-std=c11 -g -static

MAIN_SRC = cinc.c
MAIN_OBJ = cinc

ROOT = /Users/toy/dev/c/cinc
DOCK = docker run --rm -v "${ROOT}:/cinc" -w /cinc compilerbook
CC = ${DOCK} cc
CFLAGS=-std=c11 -g -static

cinc: ${MAIN_SRC}

test: ${MAIN_OBJ}
		./test

clean:
		${DOCK} rm -f ${MAIN_OBJ} *.o *~ tmp*

.PHONY: test clean
