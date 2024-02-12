СС=gcc
CFLAGS=-Wall -Wextra -Werror
LDFLAGS=-lm

.PHONY: all clean

all: lab11fvmN33532

clean:
	@rm -rf *.o lab11fvmN33532

lab11fvmN33532: lab11fvmN33532.o funcs.o
	$(CC) -o $@ $^ $(LDFLAGS)
lab11fvmN33532.o: lab11fvmN33532.c
	$(CC) $(CFLAGS) -c $<
funcs.o: funcs.c
	$(CC) $(CFLAGS) -c $<
