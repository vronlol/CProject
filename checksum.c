#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>

int checkcrc (FILE *infile, FILE *protfile);
int checkit (char *pt, FILE *protfile);
int readfiles (char *dirsname, char *dirlname, FILE *outfile);
int readdirs (char *dirsname, char *dirlname, FILE *outfile, int count);
int getcrc(char *, unsigned long *);
void init(void);
void end(void);

#define FALSE 0
#define TRUE 1
#define DoDebug FALSE


long	fCount;
int		MaxLevel;
int		tMode,verbose,AppendMode, CreateMode;
int		aError,cError,dError,tError,gFiles,uError;
int		SubMode,RMode;
long		fCount;
unsigned long 	lenCount;
int		MaxLevel,MaxMode;


int main (argc,argv)
        int argc;
        char *argv[];

{
    int i;
    char puf[2000], *pt;
    char inname[250];
    char protname[250];
    char dateString[100];
    FILE *protfile;
    FILE *infile;
    long handle;
    struct stat fileinfo;
    struct tm *writetime;
    int year;
    int retcode;
    char outname[250];
    FILE *outfile;
    char oldPath[256], newPath[256];

    tMode = FALSE;
    verbose = FALSE;
    AppendMode = FALSE;
    strcpy(inname, "00lcrc16.crc");
    strcpy(protname, "");
    fCount = 0;
    lenCount = 0;
    MaxLevel = 20;
    aError = 0;
    cError = 0;
    dError = 0;
    uError = 0;
    tError = 0;
    gFiles = 0;
    retcode = 0;
    strcpy(outname, "00lcrc16.crc");
    SubMode = FALSE;
    RMode = FALSE;
    MaxMode = 0;
    CreateMode = FALSE;

    init();

    fprintf(stderr, "checksum V1.0.0\n");
    if (DoDebug) {
        fprintf(stderr, "DEBUGMODE:\n==========\n");
        for (i = 0; i < argc; i++) { fprintf(stderr, "argv[%d] = '%s'\n", i, argv[i]); }
    }

    for (i = 1; i < argc; i++) // define options
    {
        if (*(argv[i]) == '-' || *(argv[i]) == '/') {
            if (DoDebug) { fprintf(stderr, "found option '%s'\n", argv[i]); }
            pt = (argv[i] + 1);

            if (tolower(*pt) == 'q') {
                CreateMode = TRUE;
                if (DoDebug) { printf("set option 'CreateMode'\n"); }
            }

            if (tolower(*pt) == 's') {
                SubMode = TRUE;
                if (DoDebug) { printf("set option 'SubDir'\n"); }
            }

            if (tolower(*pt) == 'r') {
                RMode = TRUE;
                if (DoDebug) { printf("set option 'RMode'\n"); }
            }

            if (tolower(*pt) == 't') {
                tMode = TRUE;
                if (DoDebug) { fprintf(stderr, "set option 'Time-Mode'\n"); }
            }

            if (tolower(*pt) == 'a') {
                AppendMode = TRUE;
                if (DoDebug) { fprintf(stderr, "set option 'AppendMode'\n"); }
            }

            if (tolower(*pt) == 'v') {
                verbose = TRUE;
                if (DoDebug) { fprintf(stderr, "set option 'Verbose'\n"); }
            } else if (tolower(*pt) == 'p') {
                pt = pt + 1;
                strcpy(protname, pt);
                if (strlen(protname) < 1) { strcpy(protname, "00lcrc16.log"); }
                if (DoDebug) { fprintf(stderr, "set option 'Prot-File=%s'\n", protname); }
            } else if (tolower(*pt) == 'c') {
                pt = pt + 1;
                strcpy(inname, pt);
                if (DoDebug) { fprintf(stderr, "found option 'CRC-File=%s'\n", pt); }
            } else if (tolower(*pt) == 'h' || tolower(*pt) == '?') {
                fprintf(stderr, "Usage: checksum [options]\n");
                fprintf(stderr, "       -p[XYZ]  Creates Protocol-File XYZ\n");
                fprintf(stderr, "       -a       AppendMode for Protocol-File\n");
                fprintf(stderr, "       -cXYZ    Checks with File XYZ instead of 00lcrc16.crc\n");
                fprintf(stderr, "       -t       Tests for Modification-Time also\n");
                fprintf(stderr, "       -v       Verbose\n");
                fprintf(stderr, "       -q       Creates Checksum\n");
                fprintf(stderr, "       Usage: -q [options]\n");
                fprintf(stderr, "              -s       Recursive: Also SubDirs\n");
                fprintf(stderr, "              -a       Appends to existing CRC-File\n");
                fprintf(stderr, "              -rROOT   Sets 'ROOT' as starting directory, instead of current\n");
                fprintf(stderr, "              -cOUT    Makes File 'OUT' instead of 00lcrc16.crc\n");
                exit(9);
            }
        }
    }


    if (CreateMode) {
        if (AppendMode)
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
                            fprintf (stderr,"\nERROR: Couldn't find directory %s\n",pt);
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
            { fprintf (stderr,"%ld Files, %.2lf MBytes\n\n",fCount,((double)(lenCount) / 1024.0)); }
            else
            { fprintf (stderr,"%ld Files, %.2lf MBytes\n\n",fCount,((double)(lenCount / 1024) / 1024.0)); }
        }
    } else { //reads file, processes its checksum, logs results. checks for errors, modified/deleted files, outputs a summary report
        infile = fopen(inname, "r");
        if (infile != NULL) {
            if (strlen(protname) > 0) {
                if (AppendMode) { protfile = fopen(protname, "a+"); }
                else { protfile = fopen(protname, "w+"); }
            } else { protfile = stdout; }
            if (protfile != NULL) {
                strcpy(dateString, "???");
                handle = stat(inname, &fileinfo);
                if (handle == 0) {
                    writetime = gmtime(&(fileinfo.st_mtime));

                    year = writetime->tm_year;
                    if (year > 99) { year = year - 100; }
                    sprintf(dateString, "%2.2d.%2.2d.%2.2d. %2.2d:%2.2d", writetime->tm_mday, writetime->tm_mon + 1, year,
                            writetime->tm_hour, writetime->tm_min, writetime->tm_sec);
                }

                if (AppendMode) { fprintf(protfile, "\n******************************************\n\n"); }
                fprintf(protfile, "CHECK (v1.0) with File from %s\n-----------------------------------------------\n",
                        dateString);

                checkcrc(infile, protfile);
                printf("%-60.60s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
                       "");


                fprintf(protfile, "-----------------------------------------------\n");
                if (aError > 0) {
                    fprintf(protfile, "General Errors:   %5d\n", aError);
                }
                fprintf(protfile, "Changed Files:    %5d\n", cError);
                fprintf(protfile, "Deleted Files:    %5d\n", dError);
                if (tError > 0) {
                    fprintf(protfile, "Timestamp-Errors: %5d\n", tError);
                }
                fprintf(protfile, "Cannot Check:     %5d\n", uError);
                fprintf(protfile, "\nCorrect Files:    %5d\n", gFiles);
                fprintf(protfile, "===============================================\n");


                if (protfile != stdout) {
                    fclose(protfile);
                    fprintf(stdout, "-----------------------------------------------\n");
                    if (aError > 0) {
                        fprintf(stdout, "General Errors:   %5d\n", aError);
                    }
                    fprintf(stdout, "Changed Files:    %5d\n", cError);
                    fprintf(stdout, "Deleted Files:    %5d\n", dError);
                    if (tError > 0) {
                        fprintf(stdout, "Timestamp-Errors: %5d\n", tError);
                    }
                    fprintf(stdout, "Cannot Check:     %5d\n", uError);
                    fprintf(stdout, "\nCorrect Files:    %5d\n", gFiles);
                    fprintf(stdout, "===============================================\n");
                }
            }
            fclose(infile);
            if (dError > 0 || tError > 0 || uError > 0) { retcode = 1; }
            if (cError > 0) { retcode = retcode + 2; }
        } else {
            fprintf(stderr, "Couldn't open CRC-File '%s'!\n", inname);
            retcode = 5;
        }
    }

    end();
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
                fprintf (protfile,"Changed: %s (Len:%ld->%ld)\n",fileName,sollLen,istLen);
                cError++;
            }
            else
            {
                if (isOK!=0 && sollLen!=0L)
                {
                    fprintf (protfile,"possibly OK:  %s (CRC couldn't be checked)\n",fileName);
                    uError++;
                }
                else
                {
                    retcode = getcrc (fileName, &istCRC);
                    if (istCRC!=sollCRC && istLen!=0L)
                    {
                        if (retcode!=0)
                        {
                            fprintf (protfile,"possibly OK:  %s (CRC %lX could't be checked right now)\n",fileName,sollCRC);
                            uError++;
                        }
                        else
                        {
                            fprintf (protfile,"Changed: %s (CRC:%lX->%lX)\n",fileName,sollCRC,istCRC);
                            cError++;
                        }
                    }
                    else
                    {
                        if (tMode)
                        {
                            if (strcmp(istDate,sollDate)!=0)
                            {
                                fprintf (protfile,"Changed: %s (Time:%s->%s)\n",fileName,sollDate,istDate);
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
            fprintf (protfile,"Deleted: %s\n",fileName);
            dError++;
        }


        //fprintf (protfile,"SollCRC=%X SollDate='%s' SollLen=%ld  '%s'\n",sollCRC,sollDate,sollLen,fileName);
    }
    else
    {
        if (strlen(inpline)>9)
        {
            fprintf (protfile,"Error:   '%s' not readable\n",inpline);
            aError++;
        }
    }

    return (result);
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
        fprintf (outfile,"/ERROR: Directory '%s' deeper than %d levels !!\n",dirlname,MaxLevel);
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
                            printf ("/ERROR: Couldn't change to '%s'\n",fullName);
                            fprintf (outfile,"/ERROR: Couldn't change to '%s'\n",fullName);
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
        fprintf (stdout,"ERROR: couldn't open directory\n");
    }
/*
printf ("%-60.60s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b","");
*/
    printf ("\n");
    return (errnum);
}

//**************************************************************************
