#include "seqTrans.h"
#include <string.h>

void	SEQ::trans_init()
{/*{{{*/
	groups = new MY_VECTOR<SEQWITHN>[DEGREE];
	revgro = new MY_VECTOR<SEQWITHN>[DEGREE];
}/*}}}*/

void    SEQ::trans_end()
{/*{{{*/
	delete[]groups;
	delete[]revgro;
	groups = NULL;
	revgro = NULL;
}/*}}}*/

bool    SEQ::seq_cmp(const   SEQWITHN &x, const   SEQWITHN &y)
{/*{{{*/
	if (x.seq < y.seq)
		return true;
	else
		return false;
}/*}}}*/

size_t  SEQ::wtf_seqwithn(FILE* outfile, const SEQWITHN &x)   // Write SEQWITHN to file 
{/*{{{*/
	size_t       forreturn = 0;

	forreturn += fwrite(&(x.seq), sizeof(uint64_t), 1, outfile);
	if (x.flag != 0)
		forreturn += fwrite(&(x.flag), sizeof(uint32_t), 1, outfile);

	return forreturn;
}/*}}}*/

size_t  SEQ::rff_seqwithn(FILE* infile, SEQWITHN &x)   // Read SEQWITHN from file
{/*{{{*/
	size_t       forreturn = -1;

	if ((forreturn = fread(&(x.seq), sizeof(uint64_t), 1, infile)) != 1)
	{
		if (forreturn == 0 && feof(infile))
		{
			return  forreturn;
		}
		else
		{
			printf("Read file ERROR: failed to get sequence from file in 'rff_seqwithn'\n");
			trans_end();
			exit(-1);
		}
	}

	// get the flag
	if (((x.seq >> SIGNBITS) & SIGNMASK) != 0)    // the sequence has unknown bases
	{
		if ((forreturn += fread(&(x.flag), sizeof(uint32_t), 1, infile)) != 2)
		{
			printf("Read file ERROR: Failed to get sequence data from file in 'rff_seqwithn'\n");
			trans_end();
			exit(-1);
		}
	}
	else
		x.flag = 0;

	return forreturn;
}/*}}}*/

void    SEQ::save_tofile(const char *nor_fname, const char *rev_fname)
{/*{{{*/
	FILE    *sortf = NULL;

	// save the sorted normal sequences
//  if (fopen_s(&sortf, nor_fname, "wb") != 0  || sortf == NULL)
    if ( ( sortf = fopen( nor_fname, "wb") )  ==  NULL )
	{
		printf("Failed to fopen file \'%s\'\n", nor_fname);
		trans_end();
		exit(-1);
	}

	for (int i = 0; i<DEGREE; i++)
	{
		groups[i].sort(seq_cmp);
		groups[i].outputf(sortf, wtf_seqwithn);
		groups[i].clear();
	}
	fclose(sortf);
	sortf = NULL;

	// save the sorted reverse sequences
//  if (fopen_s(&sortf, rev_fname, "wb") != 0  || sortf == NULL)
    if ( ( sortf = fopen( rev_fname, "wb") )  ==  NULL )
	{
		printf("Failed to fopen file \'%s\'\n", rev_fname);
		trans_end();
		exit(-1);
	}
	for (int i = 0; i<DEGREE; i++)
	{
		revgro[i].sort(seq_cmp);
		revgro[i].outputf(sortf, wtf_seqwithn);
		revgro[i].clear();
	}
	fclose(sortf);
	sortf = NULL;
}/*}}}*/

void    SEQ::fmerger(FILE *firf, FILE *secf, FILE *rstf)
{/*{{{*/
	SEQWITHN     fir_seq, sec_seq;
	FILE    *remainder;
	char    *cpbuff = (char*)malloc(sizeof(char) * 1024);

	// check
	if (!(rff_seqwithn(firf, fir_seq) && rff_seqwithn(secf, sec_seq)))
	{
		printf("File Format ERROR: empty file exits in 'fmerger'\n");
		trans_end();
		exit(-1);
	}

	// meger
	while (!(feof(firf) || feof(secf)))
	{
		if (seq_cmp(fir_seq, sec_seq))
		{
			wtf_seqwithn(rstf, fir_seq);
			rff_seqwithn(firf, fir_seq);
		}
		else
		{
			wtf_seqwithn(rstf, sec_seq);
			rff_seqwithn(secf, sec_seq);
		}
	}

	if (seq_cmp(fir_seq, sec_seq))
	{
		wtf_seqwithn(rstf, fir_seq);
		wtf_seqwithn(rstf, sec_seq);
	}
	else
	{
		wtf_seqwithn(rstf, sec_seq);
		wtf_seqwithn(rstf, fir_seq);
	}

	// take care of the remain data
	if (feof(firf))
		remainder = secf;
	else
		remainder = firf;

	while (!feof(remainder))
		fwrite(cpbuff, sizeof(char), fread(cpbuff, sizeof(char), 1024, remainder), rstf);

	// end
	free(cpbuff);
	cpbuff = NULL;
}/*}}}*/

void    SEQ::itegrate(std::queue< const char* >  fnames, const char *final_fname)
{/*{{{*/
	// declear variables
	FILE     *firf, *secf, *rstf;
	firf = secf = rstf = NULL;
	const char*     name = NULL;
	const char*     usedname;

	char     zerofile[FILENAME_MAX];
//  strcpy_s(zerofile, FILENAME_MAX, TempFilePath); strcat_s(zerofile, FILENAME_MAX, "meger0");
    strcpy(zerofile, TempFilePath); strcat(zerofile, "meger0");

	usedname = zerofile;
	// check
	if (fnames.size()  <  2)
	{
		printf("Context ERROR: number of file parts not checked before 'itegrate'\n");
		exit(-1);
	}

	while (fnames.size() > 1)
	{
		if (fnames.size() == 2)
			usedname = final_fname;

		// open files and check
		name = fnames.front();
   //   fopen_s(&firf, name, "rb");
        firf = fopen( name, "rb" );
		fnames.pop();

		name = fnames.front();
   //   fopen_s(&secf, name, "rb");
        secf = fopen( name, "rb" );
		fnames.pop();

   //   fopen_s(&rstf, usedname, "wb");
		rstf = fopen( usedname, "wb");

		if (firf == NULL || secf == NULL || rstf == NULL)
		{
			printf("File Open ERROR: Can`t open file in 'fmerger'\n");
			trans_end();
			exit(-1);
		}

		// merger
		fmerger(firf, secf, rstf);

		// push new file to the queue
		fclose(firf);
		fclose(secf);
		fclose(rstf);
		firf = secf = rstf = NULL;

		fnames.push(usedname);
		usedname = name;
	}
}/*}}}*/

void   SEQ::seq_trans(FILE *referf, char const *normal_name, char const * reverse_name)   // nor_name is the file name of the first-seq_index for normal sequence, rev_name is for reverse one
{/*{{{*/
	// initiate
	printf(" seq_trans start \n");
	trans_init();

	// declear variables
	uint64_t    data;/*{{{*/
	uint32_t    flag;
	uint64_t    revdata;
	uint32_t    revflag;
	uint32_t    index_num;
    SEQWITHN    newitem;

	uint64_t     inc;
	char         seq[STRLEN];
	char         ch;
	uint32_t     count;
	uint32_t     unknown;    //  how many bases 'n/N' a sequence has 

	int     k = 0;
	FILE    *sortf;

	std::queue< const char* >  nor_fnames, rev_fnames;
	char    *nor_serial_fname;
	char    *rev_serial_fname;
	char	serial[FILENAME_MAX];
	int     number = 0;
	int     files = 0;/*}}}*/

	// transform 
	while (!feof(referf))
	{/*{{{*/
		// get one sequence from file and check 
		if (!fgets(seq, STRLEN, referf))/*{{{*/
			break;
		if (seq[0] == '\n' || seq[0] == '\0')
			continue;

		seq[STRLEN - 1] = '\0';/*}}}*/

		// get the number of unknown bases and the length
		unknown = 0;/*{{{*/
		for (count = 0; (ch = seq[count]) >= 'a' && ch <= 'z'  ||  ch >= 'A' && ch <= 'Z' ; count++)
		{
			if (ch == 'N' || ch == 'n')
				unknown++;
		}/*}}}*/

		// check validation
		if (count < MINNT || count > MAXNT)/*{{{*/
		{
			//printf(" The length of \"%s\" is out of range \n", seq );
			continue;
		}

		if (unknown > MAXUNKNOWN)
		{
			printf("Error: unknown bases is more than %d in %s \n", MAXUNKNOWN, seq);
			continue;
		}/*}}}*/

		// get the positions of unknown bases
		k = 0;/*{{{*/
		flag = revflag = 0;
		for (int i = 0; i < count; i++)
			if (seq[i] == 'N' || seq[i] == 'n')
			{
				flag += 1 << i;
				revflag += 1 << (count - 1 - i);
				k++;
			}/*}}}*/

		// check consitency
		if (k != unknown)/*{{{*/
		{
			printf(" The number of unknown base 'N' is confusing \n ");
			exit(-1);
		}/*}}}*/

		// convert the unknown bases to konwn and get the final data
		for (int i = 0; i < (1 << (unknown * 2)); i++)
		{/*{{{*/
			// tranfrom to binary format
			revdata = data = k = 0;/*{{{*/
			for (int j = 0; j < count; j++)
			{
				ch = seq[j];
				if (ch == 'N' || ch == 'n')  // for unknown bases, from low to high bits, tow bits by tow bits of 'i' are used as the value of 'inc'.
				{
					inc = i >> (k * 2);
					inc = inc % 4;
					k++;
				}
				else
					inc = (ch >> 1) & 3;

				data = data << 2;
				data += inc;
				revdata = revdata >> 2;
				revdata += inc << 62;
			}/*}}}*/

			// put into the buckets of normal index
			data = data << (64 - count * 2);/*{{{*/
			data += unknown << SIGNBITS;
			data += MAXNT - count;

			index_num = data >> 48;
			newitem.seq = data;
			newitem.flag = flag;
			groups[index_num].push_back(newitem);/*}}}*/

			// put into the buckets of reverse index
			revdata += unknown << SIGNBITS;/*{{{*/
			revdata += MAXNT - count;

			index_num = revdata >> 48;
			newitem.seq = revdata;
			newitem.flag = revflag;
			revgro[index_num].push_back(newitem);/*}}}*/

			// increase the number for transformed sequences
			number++;
		}/*}}}*/

		// when meet threshold, save to flie
		if (number >= MAXONCE)
		{/*{{{*/
			nor_serial_fname = new char[FILENAME_MAX];	rev_serial_fname = new char[FILENAME_MAX];

        /*
			sprintf_s(serial, FILENAME_MAX, "_part%d", files);
			strcpy_s(nor_serial_fname, FILENAME_MAX, TempFilePath);	strcat_s(nor_serial_fname, FILENAME_MAX, "seq_nor"); strcat_s(nor_serial_fname, FILENAME_MAX, serial);
			strcpy_s(rev_serial_fname, FILENAME_MAX, TempFilePath);	strcat_s(rev_serial_fname, FILENAME_MAX, "seq_rev"); strcat_s(rev_serial_fname, FILENAME_MAX, serial);
        */
			sprintf(serial, "_part%d", files);
			strcpy(nor_serial_fname, TempFilePath);	strcat(nor_serial_fname, "seq_nor"); strcat(nor_serial_fname, serial);
			strcpy(rev_serial_fname, TempFilePath);	strcat(rev_serial_fname, "seq_rev"); strcat(rev_serial_fname, serial);
			
			save_tofile(nor_serial_fname, rev_serial_fname);

			nor_fnames.push(nor_serial_fname);
			rev_fnames.push(rev_serial_fname);

			nor_serial_fname = rev_serial_fname = 0;
			number = 0;
			files++;
		}/*}}}*/
	}/*}}}*/

	// merger the files 
	if (files == 0)/*{{{*/
	{
		save_tofile(normal_name, reverse_name);
	}
	else if (number > 0)
	{
		nor_serial_fname = (char*)malloc(sizeof(char) * FILENAME_MAX);
		rev_serial_fname = (char*)malloc(sizeof(char) * FILENAME_MAX);
		
    /*
		sprintf_s(serial, FILENAME_MAX, "_part%d", files);
		strcpy_s(nor_serial_fname, FILENAME_MAX, TempFilePath);	strcat_s(nor_serial_fname, FILENAME_MAX, "seq_nor"); strcat_s(nor_serial_fname, FILENAME_MAX, serial);
		strcpy_s(rev_serial_fname, FILENAME_MAX, TempFilePath);	strcat_s(rev_serial_fname, FILENAME_MAX, "seq_rev"); strcat_s(rev_serial_fname, FILENAME_MAX, serial);
    */
		sprintf(serial, "_part%d", files);
		strcpy(nor_serial_fname, TempFilePath);	strcat(nor_serial_fname, "seq_nor"); strcat(nor_serial_fname, serial);
		strcpy(rev_serial_fname, TempFilePath);	strcat(rev_serial_fname, "seq_rev"); strcat(rev_serial_fname, serial);
		
		save_tofile(nor_serial_fname, rev_serial_fname);

		nor_fnames.push(nor_serial_fname);
		rev_fnames.push(rev_serial_fname);
		nor_serial_fname = rev_serial_fname = NULL;
		number = 0;
		files++;

		itegrate(nor_fnames, normal_name);
		itegrate(rev_fnames, reverse_name);
	}/*}}}*/

	// end
	trans_end();
	
	printf(" seq_trans end \n");

}/*}}}*/


MY_VECTOR<SEQWITHN>*		SEQ::groups = 0;
MY_VECTOR<SEQWITHN>*		SEQ::revgro = 0;
