CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O0 -ggdb
EXECUTABLES = lab1_a3

.PHONY: all clean

all: $(EXECUTABLES)

lab1_a3: lab1_a3.o

lab1_a3.o: lab1_a3.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm *.o
