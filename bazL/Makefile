# WINDOWS := true

CC := gcc
CFLAGS := -Iinclude -Wall -Wextra -pedantic -O3

ifdef WINDOWS
CFLAGS += -fexec-charset=850
endif

all: obj obj/bazL.o obj/main.o bazL-debugger

obj:
	mkdir obj

obj/bazL.o obj/main.o: obj src/bazL.c src/main.c
	$(CC) $(CFLAGS) -c src/bazL.c -o obj/bazL.o
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

bazL-debugger: obj/bazL.o obj/main.o
	$(CC) -s obj/bazL.o obj/main.o -o bazL-debugger
