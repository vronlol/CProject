
CC=gcc -w
CFLAGS=-I.

default: all

all:
	$(CC) -o crlcrc crlcrc.c crclib.c $(CFLAGS)
	$(CC) -o chlcrc chlcrc.c crclib.c $(CFLAGS)
	sudo mv crlcrc /usr/local/bin/
	sudo mv chlcrc /usr/local/bin/


crlcrc: crlcrc.c crclib.c
	$(CC) -o crlcrc crlcrc.c crclib.c $(CFLAGS)
	sudo mv crlcrc /usr/local/bin/

chlcrc: chlcrc.c crclib.c
	$(CC) -o chlcrc chlcrc.c crclib.c $(CFLAGS)
	sudo mv chlcrc /usr/local/bin/



