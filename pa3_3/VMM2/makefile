# compiler and flags
CC=g++
CFLAGS= -Wno-write-strings -std=c++11

all:
	$(CC) -c -pthread -Wall zlog.c -o zlog.obj
	$(CC) $(CFLAGS) main.cpp -pthread -o vmm.out zlog.obj

Debug:
	all

Release:
	all

clean:
	rm a.out
	rm zlog.obj


