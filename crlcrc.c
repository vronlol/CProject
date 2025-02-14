#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int readfiles (char *dirsname, char *dirlname, FILE *outfile);
int readdirs (char *dirsname, char *dirlname, FILE *outfile, int count);
int getcrc(char *, unsigned *);
void init(void);

#define TRUE 1
#define FALSE 0
#define DoDebug FALSE

//define global variables
int		SubMode,AppMode,RMode;
long		fCount;
unsigned long 	lenCount;
int		MaxLevel,MaxMode;


int main (argc,argv)
    int argc;
	char *argv[];

{
int		i;
char	puf[200],*pt;
char	outname[250];
FILE	*outfile;
char	oldPath[256],newPath[256];


strcpy (outname,"00lcrc16.crc");
SubMode = FALSE;
AppMode = FALSE;
RMode = FALSE;
fCount = 0;
lenCount = 0;
MaxLevel = 20;
MaxMode = 0;
init ();

fprintf (stderr,"CRLCRC V1.0.0\n");
if (DoDebug)
	{
	printf ("DEBUGMODE:\n==========\n");
	for (i=0;i<argc;i++)
		{ printf ("argv[%d] = '%s'\n",i,argv[i]); }
	}

for (i=1;i<argc;i++) //define options
	{
	if (*(argv[i])=='-' || *(argv[i])=='/')
		{
		if (DoDebug)
			{ printf ("Option '%s' gefunden\n",argv[i]); }
		pt = (argv[i]+1);

		if (tolower(*pt)=='s')
			{ 
			SubMode = TRUE; 
			if (DoDebug)
				{ printf ("Option 'SubDir' gefunden\n"); }
			}

		if (tolower(*pt)=='a')
			{ 
			AppMode = TRUE; 
			if (DoDebug)
				{ printf ("Option 'AppMode' gefunden\n"); }
			}

		if (tolower(*pt)=='r')
			{ 
			RMode = TRUE; 
			if (DoDebug)
				{ printf ("Option 'RMode' gefunden\n"); }
			}

		if (tolower(*pt)=='c')
			{ 
			pt = pt + 1;
			strcpy (outname,pt);
			if (DoDebug)
				{ printf ("Option 'CRC-File=%s' gefunden\n",pt); }
			}

		if (tolower(*pt)=='h' || tolower(*pt)=='?')
			{ 
			printf ("Usage: crlcrc <options>                     V1.2, 011017\n"); 
			printf ("       -s     = Rekursiv: Auch SubDirs\n");
			printf ("       -a     = Haengt an vorhandenes CRC-File an\n"); 
			printf ("       -rROOT = Setzt 'ROOT' als Startverzeichnis, nicht aktuelles\n"); 
			printf ("       -cOUT  = Erzeugt nicht 00lcrc16.crc sondern Datei 'OUT'\n");
			return (1);
			}
		}
	}

if (AppMode)
	{ outfile = fopen (outname,"a+"); }
else
	{ outfile = fopen (outname,"w+"); }

if (outfile !=NULL) //process command line arguments
	{
	if (RMode)
		{
		for (i=1;i<argc;i++)
			{
			pt = argv[i];
			if ( (*pt=='/' || *pt=='-') && (tolower(*(pt+1))=='r') )
				{
				pt = argv[i]+2;
				getcwd (oldPath,255);
				chdir (pt);
				getcwd (newPath,255);
				if (strcmp(oldPath,newPath)==0)
					{
					fprintf (stderr,"\nFEHLER: Verzeichnis %s nicht gefunden\n",pt);
					}
				else
					{
					if (SubMode)
						{ readdirs (pt,pt,outfile,0); }
					else
						{ readfiles (pt,pt,outfile); }
					}
				chdir (oldPath);
				}
			}
		}
	else
		{
		if (SubMode)
			{ 
			if (DoDebug)
				{ printf ("Starting SubMode\n"); }
			readdirs (".",".",outfile,0);
			}
		else
			{ 
			if (DoDebug)
				{ printf ("Starting FileMode\n"); }
			readfiles ("","",outfile); 
			}
		}
	fclose (outfile);
	if (MaxMode>0)
	   { fprintf (stderr,"%ld Dateien, %.2lf MBytes\n\n",fCount,((double)(lenCount) / 1024.0)); }
	else
	   { fprintf (stderr,"%ld Dateien, %.2lf MBytes\n\n",fCount,((double)(lenCount / 1024) / 1024.0)); }
	}


	
return (1);
}



//**************************************************************************

int readdirs (char *dirsname, char *dirlname, FILE *outfile, int count) // recursively reads directories & reads their content

{
int			errnum;
struct		stat fileinfo;
long		dwResult, handle;
char		puf[256];
char		fullName[4000],shortName[4000],theName[4000];
char		oldPath[4000],newPath[4000];
DIR			*dir;
struct dirent	*direntry;


errnum = 0;
if (count>=MaxLevel)
	{
	fprintf (outfile,"/FEHLER: Verzeichnis '%s' tiefer als %d Ebenen !!\n",dirlname,MaxLevel);
	errnum=1;
	}
else
	{
	getcwd (oldPath,255);
	//handle	 = stat( "*.*", &fileinfo );
	dir = opendir (".");
	if (dir) 
		{
		direntry = readdir (dir);
		while (direntry)
			{
			strcpy (puf,direntry->d_name);
			strcpy (theName,direntry->d_name);
			handle	 = stat( theName, &fileinfo );
			if (DoDebug)
				{
				if (S_ISDIR(fileinfo.st_mode))
					{ printf ("Found Dir  '%s' = '%s'\n",puf,theName); }
				else if (S_ISREG(fileinfo.st_mode))
					{ printf ("Found File '%s' = '%s'\n",puf,theName); }
				else
					{ printf ("Found Other '%s' = '%s'\n",puf,theName); }
				}
			if ((S_ISDIR(fileinfo.st_mode)) && *(theName)!='.')
				{
				if (strcmp(puf,"recycler")!=0)
					{
					if (strlen(dirlname)<1)
						{ sprintf (fullName,"%s",theName);}
					else
						{ sprintf (fullName,"%s\\%s",dirlname,theName);}

					if (strlen(dirsname)<1)
						{ sprintf (shortName,"%s",puf);}
					else
						{ sprintf (shortName,"%s",puf);}

					chdir (theName);
					getcwd (newPath,255);
					if (strcmp(oldPath,newPath)==0)
						{
						printf ("/FEHLER: Konnte nicht nach '%s' wechseln\n",fullName);
						fprintf (outfile,"/FEHLER: Konnte nicht nach '%s' wechseln\n",fullName);
						chdir (oldPath);
						}
					else
						{
						readdirs (shortName,fullName,outfile,count+1);
						chdir (oldPath);
						}
					}
				}
			direntry = readdir (dir);
			}
		closedir (dir);
		}

	
	chdir (oldPath);
	fprintf (stderr,"%s\n",dirlname);
	readfiles (dirsname,dirlname,outfile);
	}

return (errnum);
}

//**************************************************************************

int readfiles (char *dirsname, char *dirlname, FILE *outfile) //scans and processes files in current directory, skipping certain file types. writing file details to an output file

{
int		errnum,year,retcode;
unsigned chksum;
long	dwResult, handle;
struct	stat fileinfo;
char	puf[200],*pt_ext, puf_lower[256];
char	fullName[4000],shortName[4000],theName[4000];
struct	tm *writetime;
DIR			*dir;
struct dirent	*direntry;


errnum = 0;
chksum = 0;
//handle	 = stat( "*.*", &fileinfo );
dir = opendir (".");
if (dir) 
	{
	direntry = readdir (dir);
	while (direntry)
		{
		strcpy (puf,direntry->d_name);
		strcpy (theName,direntry->d_name);
		handle	 = stat( theName, &fileinfo );

		if (strlen(dirlname)<1)
			{ sprintf (fullName,"%s",theName);}
		else
			{ sprintf (fullName,"%s\\%s",dirlname,theName);}

		if (strlen(dirsname)<1)
			{ sprintf (shortName,"%s",theName);}
		else
			{ sprintf (shortName,"%s",theName);}

		if (DoDebug)
			{
			if (S_ISDIR(fileinfo.st_mode))
				{ printf ("Found Dir  '%s' = '%s'\n",puf,theName); }
			else if (S_ISREG(fileinfo.st_mode))
				{ printf ("Found File '%s' = '%s'\n",puf,theName); }
			else
				{ printf ("Found Other '%s' = '%s'\n",puf,theName); }
			}

		if (S_ISREG(fileinfo.st_mode) && *(theName)!='.')
			{
			if (DoDebug)
				{
				if (S_ISREG(fileinfo.st_mode))
					{ printf ("Processing File '%s' = '%s' (%ld)\n",puf,theName,fileinfo.st_size); }
				}
			strcpy (puf_lower,theName);

			pt_ext = puf_lower;
			if (strlen(puf_lower)>4)
				{
				pt_ext = puf_lower + strlen(puf_lower) - 4;
				}

			if (strcmp(pt_ext,".crc")!=0 && strcmp(pt_ext,".tmp")!=0 && strcmp(pt_ext,".bak")!=0 && 
				strcmp(puf_lower,"pagefile.sys")!=0 && strcmp(puf_lower,"win386.swp")!=0 && strcmp(puf_lower,"00lcrc16.log")!=0)
				{
				fCount++;
				if (lenCount>2000000000)
					{ MaxMode = 1; lenCount = lenCount / 1024; }

				if (MaxMode>0)
					{ lenCount = lenCount + (fileinfo.st_size / 1024); }
				else
					{ lenCount = lenCount + fileinfo.st_size; }

				printf ("%8.8ld %-51.51s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
						fCount,puf_lower);
				// printf ("%8.8ld %-51.51s\n",fCount,puf_lower);

				writetime = gmtime(&(fileinfo.st_mtime));
				// printf ("got writetime = %ld\n",writetime); // test !!
				
				if (writetime>0) 
					{ year = writetime->tm_year; }
				else
					{ year = 0; }
				// printf ("got year\n"); // test !!
				if (year>99)
					{ year = year - 100; }
				// printf ("corrected year: %d\n",year); // test !!

				retcode = getcrc (theName, &chksum);
				// printf ("got checksum\n"); // test !!

				if (retcode==0)
					{
					if (writetime>0) 
						{ 
						fprintf (outfile," %4.4X %2.2d%2.2d%2.2d %2.2d%2.2d%2.2d  %s\t%ld\t%s\n",
								 chksum,
								 writetime->tm_mday,writetime->tm_mon+1,year,
								 writetime->tm_hour,writetime->tm_min,writetime->tm_sec,
								 shortName,fileinfo.st_size,fullName);
						}
					else
						{ 
						fprintf (outfile," %4.4X %2.2d%2.2d%2.2d %2.2d%2.2d%2.2d  %s\t%ld\t%s\n",
								 chksum,
								 0,0,0,
								 0,0,0,
								 shortName,fileinfo.st_size,fullName);
						}
					}
				else
					{
					if (writetime>0) 
						{ 
						fprintf (outfile," ---- %2.2d%2.2d%2.2d %2.2d%2.2d%2.2d  %s\t%ld\t%s\n",
							 writetime->tm_mday,writetime->tm_mon+1,year,
							 writetime->tm_hour,writetime->tm_min,writetime->tm_sec,
							 shortName,fileinfo.st_size,fullName);
						}
					else
						{ 
						fprintf (outfile," ---- %2.2d%2.2d%2.2d %2.2d%2.2d%2.2d  %s\t%ld\t%s\n",
								 0,0,0,
								 0,0,0,
								 shortName,fileinfo.st_size,fullName);
						}
					}
				}
			}
		
		direntry = readdir (dir);
		}
	closedir (dir);
	}
else
	{
	fprintf (stdout,"Fehler: konnte directory nicht Oeffnen\n");
	}
/*
printf ("%-60.60s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b","");
*/
printf ("\n");
return (errnum);
}

//**************************************************************************

