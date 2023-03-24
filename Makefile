CC=gcc
CFLAGS=-Wall -Wextra -pedantic

bruteForce: bruteForce.c aux.c aux.h
	$(CC) $(CFLAGS) -o bruteForce bruteForce.c aux.c