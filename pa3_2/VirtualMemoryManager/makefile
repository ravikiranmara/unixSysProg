# compiler and flags
CC=g++
CFLAGS= -Wno-write-strings

all:
	$(CC) -c -pthread -Wall zlog.c -o zlog.obj
	$(CC) $(CFLAGS) main.cpp -pthread -o a.out zlog.obj

Debug:
	all

Release:
	all

clean:
	rm a.out
	rm zlog.obj


