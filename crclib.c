#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define ONE_K     1024
#define MAX_BUF   (32 * ONE_K)
#define TABLE_SIZE  256
#define MASK  0xf


unsigned int buf_size;
char     *buf;
static unsigned int crc16tab[TABLE_SIZE];



int  getcrc(char *, unsigned long *);
unsigned long getChecksumForFilepointer(FILE *);
void init(void);
void make_table(void);


void init(void)
{
make_table();
buf_size = MAX_BUF;
buf = (char *)malloc(buf_size);
}


void make_table(void) //generates the CRC-16 lookup table
{
int i, inx;
int carry16;
unsigned int entry16;

for (inx = 0; inx < TABLE_SIZE; ++inx)
	{
	entry16 = inx;

	for (i = 0; i < 8; ++i)
		{
		carry16 = entry16 & 1;
		entry16 >>= 1;
		if (carry16)
			{ entry16 ^= 0xa001;}
		}
	crc16tab[inx] = entry16;
	}
}


int getcrc(char *filename, unsigned long *checksumme) //calculates a CRC-16 checksum for a given file

{
unsigned int	bytes_read,i;
unsigned long	crc16;
unsigned char	inx16;
int				ok;
FILE			*fd;

crc16 = 0;
ok = -1;
fd = fopen (filename,"rb");
if (fd!=NULL)
	{
	ok= 0;
    crc16 = getChecksumForFilepointer(fd);
	fclose (fd);
	}

*checksumme = crc16;
return ok;
}

unsigned long getChecksumForFilepointer (FILE *fp)
{
    unsigned long	bytes_read;
    unsigned long   checksumme;
    unsigned long	i;
    unsigned char	inx16;
    checksumme = 0;
    do
    {
        bytes_read = fread(buf,sizeof(char),buf_size,fp);

        for (i = 0; i < bytes_read; ++i)
        {
            inx16 = buf[i] ^ checksumme;
            checksumme >>= 8;
            checksumme ^= crc16tab[inx16];
        }
    } while (bytes_read == buf_size);
    return checksumme;
}
