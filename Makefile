CC=gcc
CFLAGS=-Wall -Wextra -pedantic

script: main.c connect.o bruteforce.o
	$(CC) $(CFLAGS) -o script main.c connect.o bruteforce.o -lpthread
	rm connect.o bruteforce.o

connect.o: connect.c connect.h
	$(CC) $(CFLAGS) -c connect.c

bruteforce.o: bruteforce.c bruteforce.h connect.h
	$(CC) $(CFLAGS) -c bruteforce.c

clean:
	rm -f *.o bruteForce