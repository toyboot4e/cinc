SRCS     = $(wildcard src/*.c)
HEADERS  = $(wildcard src/*.h)
OBJS     = $(SRCS:src/.c=obj/.o)
MAIN_OBJ = obj/cinc

# for macOS
ROOT = /Users/toy/dev/c/cinc
DOCK = docker run --rm -it -v "${ROOT}:/cinc" -w /cinc compilerbook

CC = $(DOCK) cc
CFLAGS=-std=c11 -g -static

$(MAIN_OBJ): $(OBJS)
		$(CC) -o $(MAIN_OBJ) $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADERS)

test: ${MAIN_OBJ}
		$(DOCK) ./test

clean:
		$(DOCK) rm -f $(MAIN_OBJ) obj/*.o obj/*~ obj/tmp*


.PHONY: test clean
