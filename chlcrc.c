#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int checkcrc (FILE *infile, FILE *protfile);
int checkit (char *pt, FILE *protfile);
int getcrc(char *, unsigned long *);
void init(void);

#define FALSE 0
#define TRUE 1
#define DoDebug FALSE


long	fCount,lenCount;
int		MaxLevel;
int		tMode,verbose,AppMode;
int		aError,cError,dError,tError,gFiles,uError;


int main (argc,argv)
    int argc;
	char *argv[];

{
int			i;
char		puf[2000],*pt;
char		inname[250];
char		protname[250];
char		dateString[100];
FILE		*protfile;
FILE		*infile;
long		handle;
struct		stat fileinfo;
struct		tm *writetime;
int		year;
int		retcode;

tMode = FALSE;
verbose=FALSE;
AppMode=FALSE;
strcpy (inname,"00lcrc16.crc");
strcpy (protname,"");
fCount = 0;
lenCount = 0;
MaxLevel = 20;
aError=0;
cError=0;
dError=0;
uError=0;
tError=0;
gFiles=0;
retcode=0;

init ();

fprintf (stderr,"CHLCRC V1.1.0\n");
if (DoDebug)
	{
	fprintf (stderr,"DEBUGMODE:\n==========\n");
	for (i=0;i<argc;i++)
		{ fprintf (stderr,"argv[%d] = '%s'\n",i,argv[i]); }
	}

for (i=1;i<argc;i++) // define options
	{
	if (*(argv[i])=='-' || *(argv[i])=='/')
		{
		if (DoDebug)
			{ fprintf (stderr,"Option '%s' gefunden\n",argv[i]); }
		pt = (argv[i]+1);

		if (tolower(*pt)=='t')
			{ 
			tMode = TRUE;
			if (DoDebug)
				{ fprintf (stderr,"Option 'Time-Mode' gesetzt\n"); }
			}

		if (tolower(*pt)=='a')
			{ 
			AppMode = TRUE;
			if (DoDebug)
				{ fprintf (stderr,"Option 'AppMode' gesetzt\n"); }
			}

		if (tolower(*pt)=='v')
			{ 
			verbose = TRUE;
			if (DoDebug)
				{ fprintf (stderr,"Option 'Verbose' gesetzt\n"); }
			}
		else if (tolower(*pt)=='p')
			{
			pt = pt + 1;
			strcpy (protname,pt);
			if (strlen(protname)<1)
				{ strcpy (protname,"00lcrc16.log"); }
			if (DoDebug)
				{ fprintf (stderr,"Option 'Prot-File=%s' gesetzt\n",protname); }
			}
		else if (tolower(*pt)=='c')
			{ 
			pt = pt + 1;
			strcpy (inname,pt);
			if (DoDebug)
				{ fprintf (stderr,"Option 'CRC-File=%s' gefunden\n",pt); }
			}
		else if (tolower(*pt)=='h' || tolower(*pt)=='?')
			{
			fprintf (stderr,"Usage: chlcrc [options]\n"); 
			fprintf (stderr,"       -p[XYZ]  Create Protocol-File XYZ\n"); 
			fprintf (stderr,"       -a       Appendmode for Protocol-File\n"); 
			fprintf (stderr,"       -cXYZ    Check with File XYZ instead of 00lcrc16.crc\n"); 
			fprintf (stderr,"       -t       Test for Modification-Tome also\n"); 
			fprintf (stderr,"       -v       Verbose\n");
			exit (9);
			}
		}
	}

//reads file, processes its checksum, logs results. checks for errors, modified/deleted files, outputs a summary report
infile = fopen (inname,"r");
if (infile!=NULL)
	{
	if (strlen(protname)>0)
		{ 
		if (AppMode)
			{ protfile = fopen (protname,"a+");  }
		else
			{ protfile = fopen (protname,"w+");  }
		}
	else
		{ protfile = stdout; }
	if (protfile!=NULL)
		{
		strcpy (dateString,"???");
		handle	 = stat( inname, &fileinfo );
		if (handle==0)
			{
			writetime = gmtime(&(fileinfo.st_mtime));

			year = writetime->tm_year;
			if (year>99)
				{ year = year - 100; }
			sprintf (dateString,"%2.2d.%2.2d.%2.2d. %2.2d:%2.2d",writetime->tm_mday,writetime->tm_mon+1,year,writetime->tm_hour,writetime->tm_min,writetime->tm_sec);
			}

		if (AppMode)
			{ fprintf (protfile,"\n******************************************\n\n"); }
		fprintf (protfile,"CHECK (v1.1) mit Datei vom %s\n-----------------------------------------------\n",dateString);

		checkcrc (infile,protfile);
		printf ("%-60.60s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b","");


		fprintf (protfile,"-----------------------------------------------\n");
		if (aError>0)
			{
			fprintf (protfile,"Allgemeine Fehler:   %5d\n",aError);
			}
		fprintf (protfile,"Veraenderte Dateien: %5d\n",cError);
		fprintf (protfile,"Geloeschte Dateien:  %5d\n",dError);
		if (tError>0)
			{
			fprintf (protfile,"Timestamp-Fehler:    %5d\n",tError);
			}
		fprintf (protfile,"Nicht pruefbar:      %5d\n",uError);
		fprintf (protfile,"\nKorrekte Dateien:    %5d\n",gFiles);
		fprintf (protfile,"===============================================\n");


		if (protfile!=stdout)
			{
			fclose (protfile);
			fprintf (stdout,"-----------------------------------------------\n");
			if (aError>0)
				{
				fprintf (stdout,"Allgemeine Fehler:   %5d\n",aError);
				}
			fprintf (stdout,"Veraenderte Dateien: %5d\n",cError);
			fprintf (stdout,"Geloeschte Dateien:  %5d\n",dError);
			if (tError>0)
				{
				fprintf (stdout,"Timestamp-Fehler:    %5d\n",tError);
				}
			fprintf (stdout,"Nicht pruefbar:      %5d\n",uError);
			fprintf (stdout,"\nKorrekte Dateien:    %5d\n",gFiles);
		        fprintf (stdout,"===============================================\n");
			}
		}
	fclose (infile);
	if (dError>0 || tError>0 || uError>0)
		{ retcode=1; }
	if (cError>0)
		{ retcode=retcode+2; }
	}
else
	{
	fprintf (stderr,"Konnte CRC-Datei '%s' nicht oeffnen!\n",inname);
	retcode=5;
	}


	
	

return (1);
}


//**************************************************************************

int checkcrc (FILE *infile, FILE *protfile) //reads lines from input file, processes them and writes result in protocol file

{
int		result;
char	*pt,puf[2000],*ppp;

result = 0;
pt = fgets(puf,1999,infile);
while (pt!=NULL)
	{
	for (ppp=pt;*ppp!='\0';ppp++)
		{ if(*ppp=='\\') *ppp='/'; }
	fCount++;

	if ((fCount & 7)==0) { fflush(protfile); }

	if (strlen(pt)>0 &&  (*(pt+strlen(pt)-1)=='\n' || *(pt+strlen(pt)-1)=='\r'))
		{ *(pt+strlen(pt)-1)='\0'; }
	if (strlen(pt)>0 &&  (*(pt+strlen(pt)-1)=='\n' || *(pt+strlen(pt)-1)=='\r'))
		{ *(pt+strlen(pt)-1)='\0'; }

	if (*pt=='/')
		{ fprintf (protfile,">> %s\n",pt+1); aError++; }
	else
		{ result = result | checkit (pt,protfile); }
	pt = fgets(puf,1999,infile);
	}

return (result);
}

//**************************************************************************
//verifies file integrity by checking file existence, modification time, length, and checksum
//logs the results to protfile and categorizes files as changed, deleted, or OK

int checkit (char *inpline, FILE *protfile)

{
int			result,retcode,isOK;
long		handle;
struct		stat fileinfo;
struct		tm *writetime;
int			year;
char		*pt;

unsigned long	istCRC, sollCRC;
char		istDate[40], sollDate[40];
long		istLen, sollLen;
char		fileName[256];

result = 0;
if (strlen(inpline)>21)
	{
	sollCRC=0; isOK = -1;
	if (inpline[1]!='-' && inpline[2]!='-')
	   { sscanf (inpline+1,"%lX",&sollCRC); isOK=0; }
	strncpy (sollDate,inpline+6,13); sollDate[13]='\0';

	pt = inpline+21;
	while (*pt!='\0' && *pt!='\t') { pt++; }
	if (*pt=='\t') { pt++; }
	sscanf (pt,"%ld",&sollLen);

	while (*pt!='\0' && *pt!='\t') { pt++; }
	if (*pt=='\t') { pt++; }
	strcpy (fileName,pt);

	printf ("%8.8ld  %-50.50s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
			    fCount,fileName);

	handle	 = stat(fileName, &fileinfo );
	if (handle==0)
		{
		istLen = fileinfo.st_size;
		writetime = gmtime(&(fileinfo.st_mtime));

		year = writetime->tm_year;
		if (year>99)
			{ year = year - 100; }
		sprintf (istDate,"%2.2d%2.2d%2.2d %2.2d%2.2d%2.2d",writetime->tm_mday,writetime->tm_mon+1,year,writetime->tm_hour,writetime->tm_min,writetime->tm_sec);

		if (istLen!=sollLen)
			{
			fprintf (protfile,"Geaendert: %s (Len:%ld->%ld)\n",fileName,sollLen,istLen); 
			cError++;
			}
		else
			{
			if (isOK!=0 && sollLen!=0L)
				{
				fprintf (protfile,"Evtl. OK:  %s (CRC konnte nicht geprueft werden)\n",fileName); 
				uError++;
				}
			else
				{
				retcode = getcrc (fileName, &istCRC);
				if (istCRC!=sollCRC && istLen!=0L)
					{
					if (retcode!=0)
						{
						fprintf (protfile,"Evtl. OK:  %s (CRC %lX konnte jetzt nicht geprueft werden)\n",fileName,sollCRC); 
						uError++;
						}
					else
						{
						fprintf (protfile,"Geaendert: %s (CRC:%lX->%lX)\n",fileName,sollCRC,istCRC); 
						cError++;
						}
					}
				else
					{
					if (tMode)
						{
						if (strcmp(istDate,sollDate)!=0)
							{
							fprintf (protfile,"Geaendert: %s (Time:%s->%s)\n",fileName,sollDate,istDate); 
							tError++;
							}
						else
							{
							if (verbose)
								{ fprintf (protfile,"OK:        %s\n",fileName,sollDate,istDate); }
							gFiles++;
							}
						}
					else
						{
						if (verbose)
							{ fprintf (protfile,"OK:        %s\n",fileName,sollDate,istDate); }
						gFiles++;
						}
					}
				}
			}

		}
	else
		{
		fprintf (protfile,"Geloescht: %s\n",fileName); 
		dError++;
		}


	//fprintf (protfile,"SollCRC=%X SollDate='%s' SollLen=%ld  '%s'\n",sollCRC,sollDate,sollLen,fileName);
	}
else 
	{
	if (strlen(inpline)>9)
		{
		fprintf (protfile,"Fehler:   '%s' nicht lesbar\n",inpline); 
		aError++;
		}
	}

return (result);
}


//**************************************************************************

