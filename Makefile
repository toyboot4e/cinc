SRCS     = $(wildcard src/*.c)
HEADERS  = $(wildcard src/*.h)
OBJS     = $(SRCS:src/.c=obj/.o)
MAIN_OBJ = obj/cinc

ROOT = $$HOME/dev/c/cinc
DOCKER = docker run --rm -it -v "${ROOT}:/cinc" -w /cinc compilerbook

CC = $(DOCKER) cc
CFLAGS=-std=c11 -g -static

$(MAIN_OBJ): $(OBJS)
		$(CC) -o $(MAIN_OBJ) $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADERS)

test: ${MAIN_OBJ}
		$(DOCKER) ./test

clean:
		$(DOCKER) rm -f $(MAIN_OBJ) obj/*.o obj/*~ obj/tmp*

# doc: 

.PHONY: test clean
