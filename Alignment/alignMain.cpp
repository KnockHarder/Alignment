#include "seqTrans.h"	
#include "align.h"

char	filenames[6][MAXFNAME];

void    fnameformat(char*     filename);
bool	getIndicesNames(const char* namesfile);

int	main(int argc, char** argv)
{
  // check arguments
	if (argc != 4)
	{
		printf("Usage: alignment namesfile seqfile resultfile\n");
		exit(-1);
	}

	if (!getIndicesNames(argv[1]))
	{
		printf("File Read Error: Can`t get file names from '%s' \n", argv[1]);
		exit(-1);
	}

  // start work
	FILE*	seqf = 0;

//  if (fopen_s(&seqf, argv[2], "rt") != 0 || seqf == NULL)
    if ( ( seqf = fopen( argv[2], "rt") )  ==  0 )
	{
		printf("File Read Error : Can`t open file '%s' \n", argv[2] );
		exit(-1);
	}

  // translate sequences and alignment
	SEQ::seq_trans(seqf, filenames[4], filenames[5]);
	ALI::alignment(filenames, argv[3]);

	return 0;
}

bool	getIndicesNames(const char* namesfile)
{
	FILE*	fp = 0;

//  if (fopen_s(&fp, namesfile, "rt") != 0 || fp == 0)
    if ( ( fp = fopen( namesfile, "rt") )  ==  0 )
		return false;

  // get names for index files
	for (int i = 0; i < 4; i++)
	{

		if (fgets(filenames[i], MAXFNAME, fp) == NULL)
			return false;

		fnameformat( filenames[i]);
	}

    /*
	strcpy_s(filenames[4], MAXFNAME, TempFilePath);	strcat_s(filenames[4], MAXFNAME, "seq_nor");
	strcpy_s(filenames[5], MAXFNAME, TempFilePath);	strcat_s(filenames[5], MAXFNAME, "seq_rev");
    */
	strcpy(filenames[4], TempFilePath);	strcat(filenames[4], "seq_nor");
	strcpy(filenames[5], TempFilePath);	strcat(filenames[5], "seq_rev");

	return true;
}

void    fnameformat(char*     filename)
{
	int     end = 0;
	char    ch = filename[0];

	while (end < MAXFNAME  &&  ch != '\n'  &&  ch != '\0')
	{
		end++;
		ch = filename[end];
	}
	filename[end] = '\0';
}
