
CC=gcc -w
CFLAGS=-I.

default: all

all:
	$(CC) -o checksum checksum.c crclib.c $(CFLAGS)
	sudo mv checksum /usr/local/bin/

#	$(CC) -o crlcrc crlcrc.c crclib.c $(CFLAGS)
#	$(CC) -o chlcrc chlcrc.c crclib.c $(CFLAGS)
#	sudo mv crlcrc /usr/local/bin/
#	sudo mv chlcrc /usr/local/bin/


crlcrc: crlcrc.c crclib.c
	$(CC) -o crlcrc crlcrc.c crclib.c $(CFLAGS)
	sudo mv crlcrc /usr/local/bin/

chlcrc: chlcrc.c crclib.c
	$(CC) -o chlcrc chlcrc.c crclib.c $(CFLAGS)
	sudo mv chlcrc /usr/local/bin/

checksum: checksum.c crclib.c
	$(CC) -o checksum checksum.c crclib.c $(CFLAGS)
	sudo mv checksum /usr/local/bin/


