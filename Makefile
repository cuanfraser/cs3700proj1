CC=gcc
CFLAGS=-I.

hellomake: client.c
	$(CC) -o client client.c $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o client
