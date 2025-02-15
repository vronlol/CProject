
CC=gcc -w
CFLAGS=-I.

default: all

all:
	$(CC) -o checksum checksum.c crclib.c $(CFLAGS)
	sudo cp checksum /usr/local/bin/

checksum: checksum.c crclib.c
	$(CC) -o checksum checksum.c crclib.c $(CFLAGS)
	sudo mv checksum /usr/local/bin/


