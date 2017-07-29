#include "firIndex.h"
#include "secIndex.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

char	FirName[4][FILENAME_MAX];
char	SecName[4][FILENAME_MAX];
char	MidName[4][FILENAME_MAX];

const char*   getFileName(const char* fullPath);

void	genNames(const char* ref_fname);

void	saveNames(FILE *fp);

int	main(int argc, char** argv)
{/*{{{*/

    // check arguments
    /*{{{*/
	if (argc != 3)
	{
		printf("Usage: Command  referfile  namesfile\n");
		return -1;
	}
    /*}}}*/

    // declear variables
    /*{{{*/
	FILE    *referf, *namesf, *midf;
	referf = namesf = midf = 0;
    /*}}}*/

    // check arguments
    // if (fopen_s(&referf, argv[1], "rt") != 0  ||  referf == NULL)
    /*{{{*/
    if ( ( referf = fopen( argv[1], "rt") )  ==  0 )
	{
		printf("File Open Error: Can`t open file '%s' successfully!\n", argv[1]);
		return -1;
	}

    //if (fopen_s(&namesf, argv[2], "wt") != 0  ||  namesf == NULL)
    if ( ( namesf = fopen( argv[2], "wt") )  ==  0 )
	{
		printf("File Open Error: Can`t open file '%s' successfully!\n", argv[2]);
		return -1;
	}
    /*}}}*/ 

    // generate files names
	genNames(argv[1]);
  
    // make indices
    /*{{{*/
    // fir indices, get mid files 
	FIR::set_fir(referf, MidName);
	
    // sec indices, get final files 
    for( int i = 0; i < 4; i ++ )
    {
        // if (fopen_s(&midf, MidNor, "rb") != 0  ||  midf  == NULL)
        if ( ( midf = fopen( MidName[i], "rb") )  ==  0 )
        {
            printf("File Open Error : Can`t open generated file '%s' successfully!\n", MidName[i]);
            return false;
        }

        SEC::set_sec(midf, FirName[i], SecName[i]);

        fclose(midf);
        midf = 0;
    }
    /*}}}*/

    // save names
	saveNames(namesf);

    // close files 
    /*{{{*/
	fclose(referf);
	fclose(namesf);
    referf = namesf = 0;
    /*}}}*/

	return 0;
}/*}}}*/

void	genNames(const char* ref_fname)
{/*{{{*/

    // declear variables
    /*{{{*/
	char	prefix[FILENAME_MAX], postfix[FILENAME_MAX];
	char	filename[FILENAME_MAX];
    /*}}}*/

    // get the file name
    // strcpy_s(filename, FILENAME_MAX, getFileName(ref_fname));
    strcpy(filename, getFileName(ref_fname));

    // get directory
    // strcpy_s(prefix, FILENAME_MAX, TempFilePath);
    strcpy(prefix, TempFilePath);

    // generate midfile names
    /*{{{*/
    // strcpy_s(MidNor, FILENAME_MAX, prefix);	strcat_s(MidNor, FILENAME_MAX, "mid_nor");
    // strcpy_s(MidRev, FILENAME_MAX, prefix);	strcat_s(MidRev, FILENAME_MAX, "mid_rev");
    for( int i = 0; i < 4; i ++ )
    {
        sprintf(postfix, "%d%d", i/2, i%2);

        strcpy(MidName[i], prefix);
        strcat(MidName[i], "mid_");
        strcat(MidName[i], postfix);
    }
    /*}}}*/

    // generate  index-files` names
    /*{{{*/

	// strcpy_s(prefix, FILENAME_MAX, IndicesPath);
	// strcat_s(prefix, FILENAME_MAX, strtok_s(filename, ".", &contex));
	// strcat_s(prefix, FILENAME_MAX, "/");
	// strcat_s(prefix, FILENAME_MAX, strtok_s(filename, ".", &contex));

	// strcat_s(FirNor, FILENAME_MAX, prefix);	strcat_s(FirNor, FILENAME_MAX, "_fir_nor");
	// strcat_s(SecNor, FILENAME_MAX, prefix);	strcat_s(SecNor, FILENAME_MAX, "_sec_nor");
	// strcat_s(SecRev, FILENAME_MAX, prefix);	strcat_s(SecRev, FILENAME_MAX, "_sec_rev");
	// strcat_s(FirRev, FILENAME_MAX, prefix);	strcat_s(FirRev, FILENAME_MAX, "_fir_rev");

	strcpy(prefix, IndicesPath);
	strcat(prefix, strtok(filename, "."));
	strcat(prefix, "/");
    
    // create a directory for the index files
    /*{{{*/
    if( access( prefix, F_OK ) == -1 )
        if( mkdir( prefix, S_IRWXU )  == -1 )
            printf( "Failed to create dirctory - %s\n", prefix );
    /*}}}*/

	strcat(prefix, strtok(filename, "."));

    for( int i = 0; i < 4; i ++ )
    {
        sprintf(postfix, "%d%d", i/2, i%2);
        
	    strcpy(FirName[i], prefix);
        strcat(FirName[i], "_fir_");
        strcat(FirName[i], postfix);
        
	    strcpy(SecName[i], prefix);
        strcat(SecName[i], "_sec_");
        strcat(SecName[i], postfix);
    }
    /*}}}*/

}/*}}}*/

void	saveNames(FILE *fp)
{/*{{{*/
    for( int i = 0; i < 4; i ++ )
    {
        fputs(FirName[i], fp);	
        fputs("\n", fp);
    }

    for( int i = 0; i < 4; i ++ )
    {
        fputs(SecName[i], fp);
        fputs("\n", fp);
    }
}/*}}}*/

const char*	getFileName( const char* fullPath)
{/*{{{*/
	char	*fname = 0, *rst = 0, \
		    fullname[FILENAME_MAX];

    // strcpy_s(fullname, FILENAME_MAX, fullPath);
    strcpy(fullname, fullPath);

    // fname = strtok_s(fullname, "/\\", &temp);
	rst = fname = strtok(fullname, "/\\");

	while ( fname = strtok(0, "/\\") )
        rst = fname;

	return	rst;

}/*}}}*/
