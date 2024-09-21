CC = gcc
CFLAGS = -g -Wall -std=c99 -fsanitize=address

spchk: spchk.c 
	$(CC) $(CFLAGS) spchk.c arraylist.c -o spchk
	
readlines: readlines.c 
	$(CC) $(CFLAGS) readlines.c -o readlines

