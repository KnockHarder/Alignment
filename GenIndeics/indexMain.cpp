#include "firIndex.h"
#include "secIndex.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

char	FirNor[FILENAME_MAX], FirRev[FILENAME_MAX];
char	SecNor[FILENAME_MAX], SecRev[FILENAME_MAX];

char	MidNor[FILENAME_MAX], MidRev[FILENAME_MAX];

const char*   getFileName(const char* fullPath);

void	genNames(const char* ref_fname);

void	saveNames(FILE *fp);

int	main(int argc, char** argv)
{/*{{{*/
  // check arguments
	if (argc != 3)
	{/*{{{*/
		printf("Usage: Command  referfile  namesfile\n");
		return -1;
	}/*}}}*/

  // declear variables
	FILE    *referf, *namesf, *midf;/*{{{*/
	referf = namesf = midf = 0;/*}}}*/

  //check arguments
//  if (fopen_s(&referf, argv[1], "rt") != 0  ||  referf == NULL)
    if ( ( referf = fopen( argv[1], "rt") )  ==  0 )
	{/*{{{*/
		printf("File Open Error: Can`t open file '%s' successfully!\n", argv[1]);
		return -1;
	}/*}}}*/

//  if (fopen_s(&namesf, argv[2], "wt") != 0  ||  namesf == NULL)
    if ( ( namesf = fopen( argv[2], "wt") )  ==  0 )
	{/*{{{*/
		printf("File Open Error: Can`t open file '%s' successfully!\n", argv[2]);
		return -1;
	}/*}}}*/

  // generate files names
	genNames(argv[1]);
  
  // generate indices
	  // fir indices, get mid files 
	FIR::set_fir(referf, MidNor, MidRev);
	
	  // forward direction sec indices, get final files 
//  if (fopen_s(&midf, MidNor, "rb") != 0  ||  midf  == NULL)
    if ( ( midf = fopen( MidNor, "rb") )  ==  0 )
	{/*{{{*/
		printf("File Open Error : Can`t open generated file '%s' successfully!\n", MidNor);
		return false;
	}/*}}}*/

	SEC::set_sec(midf, FirNor, SecNor);
	
	  // backward direction sec indices, get final files 
	fclose(midf);
    midf = 0;
//  if (fopen_s(&midf, MidRev, "rb") != 0  ||  midf == NULL)
    if ( ( midf = fopen( MidRev, "rb") )  ==  0 )
	{/*{{{*/
		printf("File Open Error : Can`t open generated file '%s' successfully!\n", MidNor);
		return false;
	}/*}}}*/
	SEC::set_sec(midf, FirRev, SecRev);

  // save names
	saveNames(namesf);

  // close files 
	fclose(referf);/*{{{*/
	fclose(namesf);
	fclose(midf);
    referf = namesf = midf = 0;
/*}}}*/
	return 0;
}/*}}}*/

void	genNames(const char* ref_fname)
{/*{{{*/
	memset(FirNor, 0, FILENAME_MAX);	memset(FirRev, 0, FILENAME_MAX);
	memset(SecNor, 0, FILENAME_MAX);	memset(SecRev, 0, FILENAME_MAX);
	memset(MidNor, 0, FILENAME_MAX);	memset(MidRev, 0, FILENAME_MAX);
	
	char	preFix[FILENAME_MAX];

  // cpy the file name
	char	filename[FILENAME_MAX];
//  strcpy_s(filename, FILENAME_MAX, getFileName(ref_fname));
    strcpy(filename, getFileName(ref_fname));

  // mid files names
	memset(preFix, 0, FILENAME_MAX);
//  strcat_s(preFix, FILENAME_MAX, TempFilePath);
    strcat(preFix, TempFilePath);

//  strcat_s(MidNor, FILENAME_MAX, preFix);	strcat_s(MidNor, FILENAME_MAX, "mid_nor");
//  strcat_s(MidRev, FILENAME_MAX, preFix);	strcat_s(MidRev, FILENAME_MAX, "mid_rev");
    strcat(MidNor, preFix);	strcat(MidNor, "mid_nor");
    strcat(MidRev, preFix);	strcat(MidRev, "mid_rev");

  // index files names
	memset(preFix, 0, FILENAME_MAX);
/*
	strcat_s(preFix, FILENAME_MAX, IndicesPath);
	strcat_s(preFix, FILENAME_MAX, strtok_s(filename, ".", &contex));
	strcat_s(preFix, FILENAME_MAX, "/");
	strcat_s(preFix, FILENAME_MAX, strtok_s(filename, ".", &contex));

	strcat_s(FirNor, FILENAME_MAX, preFix);	strcat_s(FirNor, FILENAME_MAX, "_fir_nor");
	strcat_s(SecNor, FILENAME_MAX, preFix);	strcat_s(SecNor, FILENAME_MAX, "_sec_nor");
	strcat_s(SecRev, FILENAME_MAX, preFix);	strcat_s(SecRev, FILENAME_MAX, "_sec_rev");
	strcat_s(FirRev, FILENAME_MAX, preFix);	strcat_s(FirRev, FILENAME_MAX, "_fir_rev");
*/

	strcat(preFix, IndicesPath);
	strcat(preFix, strtok(filename, "."));
	strcat(preFix, "/");
    
    if( access( preFix, F_OK ) == -1 )
        if( mkdir( preFix, S_IRWXU )  == -1 )
            printf( "Failed to create dirctory - %s\n", preFix );

	strcat(preFix, strtok(filename, "."));

	strcat(FirNor, preFix);	strcat(FirNor, "_fir_nor");
	strcat(SecNor, preFix);	strcat(SecNor, "_sec_nor");
	strcat(SecRev, preFix);	strcat(SecRev, "_sec_rev");
	strcat(FirRev, preFix);	strcat(FirRev, "_fir_rev");
}/*}}}*/

void	saveNames(FILE *fp)
{/*{{{*/
	fputs(FirNor, fp);	fputs("\n", fp);	;fputs(FirRev, fp);	fputs("\n",  fp);
	fputs(SecNor, fp);	fputs("\n", fp);	;fputs(SecRev, fp);	fputs("\n",  fp);
}/*}}}*/

const char*	getFileName( const char* fullPath)
{/*{{{*/
	char	*fname = 0, *rst = 0,
		fullname[FILENAME_MAX];

//  strcpy_s(fullname, FILENAME_MAX, fullPath);
    strcpy(fullname, fullPath);

//  fname = strtok_s(fullname, "/\\", &temp);
	rst = fname = strtok(fullname, "/\\");

	while ( fname = strtok(0, "/\\") )
        rst = fname;

	return	rst;

}/*}}}*/
