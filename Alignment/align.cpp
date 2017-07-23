#include "align.h"
#include <string.h>

void    ALI::ali_init(char filenames[6][MAXFNAME])
{/*{{{*/
	FILE	*fp = 0;

	for (int i = 0; i < 6; i++)
	{
	//  if ( fopen_s( &fp, filenames[i], "rb") != 0  ||  fp == NLL)
	    if ( ( fp = fopen( filenames[i], "rb") )  ==  0 )
		{
			printf("Can`t open file \'%s\' correctly\n", filenames[i]);
			exit(-1);
		}
		fclose(fp);
	}

    /*
	strcpy_s(fin_fname[0], sizeof(filenames[0]), filenames[0]); strcpy_s(fin_fname[1], sizeof(filenames[1]), filenames[1]);
	strcpy_s(sin_fname[0], sizeof(filenames[2]), filenames[2]);	strcpy_s(sin_fname[1], sizeof(filenames[3]), filenames[3]);
	strcpy_s(seq_fname[0], sizeof(filenames[4]), filenames[4]);	strcpy_s(seq_fname[1], sizeof(filenames[5]), filenames[5]);
    */
	strcpy(fin_fname[0], filenames[0]); strcpy(fin_fname[1], filenames[1]);
	strcpy(sin_fname[0], filenames[2]);	strcpy(sin_fname[1], filenames[3]);
	strcpy(seq_fname[0], filenames[4]);	strcpy(seq_fname[1], filenames[5]);

	//  variables init
	fir_index = (RECORD*)malloc(sizeof(RECORD) * DEGREE);
	sec_index = (RECORD*)malloc(sizeof(RECORD) * DEGREE);
	start_loc = (uint64_t*)malloc(sizeof(uint64_t) * DEGREE);
	memset(fir_index, 0, sizeof(RECORD) * DEGREE);
	memset(sec_index, 0, sizeof(RECORD) * DEGREE);
	memset(start_loc, 0, sizeof(uint64_t) * DEGREE);
}/*}}}*/

void    ALI::ali_end()
{/*{{{*/
	free(fir_index);
	free(sec_index);
	free(start_loc);
	fir_index = NULL;
	sec_index = NULL;
	start_loc = NULL;
}/*}}}*/

char*   ALI::toString(uint64_t  data)
{/*{{{*/
	char        *rs = (char*)malloc(sizeof(char) * 33);
	unsigned    value;

	for (int i = 0; i < 32; i++)
	{
		value = (data >> (i * 2)) % 4;
		rs[31 - i] = seq_map[value];
	}
	rs[32] = '\0';

	return  rs;
}/*}}}*/

char*   ALI::recoverSeq(const  SEQWITHN  &seq)
{/*{{{*/
	char*   seqstr = (char*)malloc(sizeof(char) * (MAXNT + 1));
	char*   temp;
	int     length = MAXNT - (seq.seq & SIGNMASK);

	int     unknown = (seq.seq >> SIGNBITS) & SIGNMASK;

	temp = toString(seq.seq);
//  strncpy_s(seqstr, MAXNT + 1, temp, length);
    strncpy(seqstr, temp, length);
	seqstr[length] = '\0';

	int     k = 0;
	if (seq.flag != 0)
	{
		if (seq.flag >= (1 << length))
		{
			printf("Error: Sequence %lu is not consistent with Flag %d \n", seq.seq, seq.flag);
			ali_end();
			exit(-1);
		}
		for (int i = 0; i < length; i++)
			if (((seq.flag >> i) & 1) == 1)
			{
				seqstr[i] = 'N';
				k++;
			}
	}

	if (k != unknown)
	{
		printf("Error: Sequence %lu is not consistent with Flag %d \n", seq.seq, seq.flag);
		ali_end();
		exit(-1);
	}

	return seqstr;
}/*}}}*/

void    ALI::readerror(char const * filename)
{/*{{{*/
	printf("Unexpected error happened during reading data from \'%s\'\n", filename);
	ali_end();
	exit(-1);
}/*}}}*/

int     ALI::cmp_seq(const uint64_t &refer, const uint64_t &seq)
{/*{{{*/
	int     ref_idle_bits = (refer & SIGNMASK) * 2;
	int     seq_idle_bits = (seq & SIGNMASK) * 2;
	int     max = (ref_idle_bits > seq_idle_bits) ? ref_idle_bits : seq_idle_bits;

	uint64_t     x = refer >> (max + SIGNBITS * 2), y = seq >> (max + SIGNBITS * 2);

	if (x > y)
		return 1;
	if (x == y)
		return 0;
	if (x < y)
		return -1;
}/*}}}*/

void    ALI::writeResult(const uint64_t   &refer, const SEQWITHN    &seq, FILE*  rsf, bool isrev)
{/*{{{*/

    // declears
	char*   refstr = (char*)malloc(sizeof(char) * (MAXNT * 2 + 10));/*{{{*/
	char*   seqstr = (char*)malloc(sizeof(char) * (MAXNT + 1));
	char*   temp;
	int     length;

	int     pad = (refer >> SIGNBITS) & SIGNMASK;
	int     reserv = refer & SIGNMASK;
	temp = toString(refer);/*}}}*/

	if (pad < 16) // padded or normal
	{
		length = MAXNT - pad - reserv;
	//  strncpy_s(refstr, (MAXNT * 2 + 10), temp + pad, length);
	    strncpy(refstr, temp + pad, length);
		refstr[length] = '\0';
	}
	else    // shrinked
	{
		pad = pad - 16;
		length = MAXNT - reserv + pad;
		for (int i = 0; i < pad; i++)
			refstr[i] = temp[MAXNT - pad + i];
		for (int i = 0; i < MAXNT - reserv; i++)
			refstr[pad + i] = temp[i];
		refstr[length] = '\0';
	}

	if (isrev)
	{
		for (int i = 0; i < length; i++)
			temp[i] = refstr[length - 1 - i];
	//  strncpy_s(refstr, (MAXNT * 2 + 10), temp, length);
	    strncpy(refstr, temp, length);
		refstr[length] = '\0';
	}
	free(temp);
	temp = NULL;

	if (length < MINNT || length > MAXNT)
	{
		printf(" length error in 'alignment', ref = %s\n", refstr);
		printf("pad = %d \t reserv = %d \n", pad, reserv);
	}


	temp = recoverSeq(seq);
	length = MAXNT - (seq.seq & SIGNMASK);
//  strncpy_s(seqstr, (MAXNT + 1), temp, length);
    strncpy(seqstr, temp, length);
	seqstr[length] = '\0';

	if (isrev == 1)
	{
		for (int i = 0; i < length; i++)
			temp[i] = seqstr[length - 1 - i];
	//  strncpy_s(seqstr, (MAXNT + 1), temp, length);
	    strncpy(seqstr, temp, length);
		seqstr[length] = '\0';
	}

	free(temp);
	temp = NULL;
	if (length < MINNT || length > MAXNT)
		printf(" length error in 'alignment', seq = %s\n", seqstr);

//  strcat_s(refstr, (MAXNT * 2 + 10), " = ");
//  strcat_s(refstr, (MAXNT * 2 + 10), seqstr);
    strcat(refstr, " = ");
    strcat(refstr, seqstr);
	fputs(refstr, rsf);
	fputs("\r\n", rsf);

	free(refstr);
	free(seqstr);
	refstr = seqstr = NULL;
}/*}}}*/

void    ALI::alignment(char filenames[6][MAXFNAME], const char* rsf_fname)
{/*{{{*/
	printf(" alignment start \n");
	ali_init(filenames);

	//  declear variables
	FILE      *finf, *sinf, *seqf, *resultf;/*{{{*/
	int64_t   fir_num = -1;    // record the first index number which related to the second index
	int64_t   sec_num = -1;
	bool        is_int64 = false;       // which block is used, 32-bit data or 64-bit data
	int         length = -1, find_start = 0;
	uint32_t    index_num;

	uint32_t    *buff_32 = NULL;
	uint64_t    *buff_64 = NULL;

	SEQWITHN    seq_data;
	uint64_t    ref_data;
	int         position;
	int			dup_position;
	int         errorcode;

	char        *fin_buff = NULL;
	char        *fout_buff = NULL;/*}}}*/

	// Init the file buffs
	fin_buff = (char*)malloc(MY_BUFSIZ);/*{{{*/
	fout_buff = (char*)malloc(MY_BUFSIZ);
	if (fin_buff == NULL || fout_buff == NULL)
	{
		printf("Error: Failed to allocate memory \n");
		exit(-1);
	}/*}}}*/

	// open the file saved results
//  if (fopen_s(&resultf, rsf_fname, "wt") != 0 || resultf == NULL)
	if ( ( resultf = fopen( rsf_fname, "wt") )  ==  0 )/*{{{*/
	{
		printf("Error: Can`t open the file '%s'\n", rsf_fname);
		ali_end();
		exit(-1);
	}
	setvbuf(resultf, fin_buff, _IOFBF, MY_BUFSIZ);/*}}}*/

	// alignment on both side
	for (int i = 0; i < 1; i++) //for debuggging
	{/*{{{*/
		fir_num = sec_num = -1;
		is_int64 = false;

		//  open the files 
    	//  if (fopen_s(&finf, fin_fname[i], "rb") != 0  ||  finf == NULL)
	    if ( ( finf = fopen( fin_fname[i], "rb") )  ==  0 )
		{/*{{{*/
			printf("Open file \'%s\' failed\n", fin_fname[i]);
			ali_end();
			exit(-1);
		}/*}}}*/

	    //  if (fopen_s(&sinf, sin_fname[i], "rb") != 0  || sinf == NULL)
	    if ( ( sinf = fopen( sin_fname[i], "rb") )  ==  0 )
		{/*{{{*/
			printf("Open file \'%s\' failed\n", sin_fname[i]);
			ali_end();
			exit(-1);
		}/*}}}*/
		
	    //  if (fopen_s(&seqf, seq_fname[i], "rb") != 0  || seqf == NULL)
	    if ( ( seqf = fopen( seq_fname[i], "rb") )  ==  0 )/*{{{*/
		{
			printf("Open file \'%s\' failed\n", seq_fname[i]);
			ali_end();
			exit(-1);
		}
		setvbuf(seqf, fout_buff, _IOFBF, MY_BUFSIZ);/*}}}*/

		//  read first index and the start location in the file of the index[i] 
		if (fread(fir_index, sizeof(RECORD), DEGREE, finf) != DEGREE)/*{{{*/
		{
			printf("File \'%s\' has bad data\n", fin_fname[i]);
			ali_end();
			exit(-1);
		}

		if (fread(start_loc, sizeof(uint64_t), DEGREE, finf) != DEGREE)
		{
			printf("File \'%s\' has bad data\n", fin_fname[i]);
			ali_end();
			exit(-1);
		}
		fclose(finf);
		finf = NULL;/*}}}*/

		//  get the sequence and search it with the index one by one
		while (!feof(seqf))
		{/*{{{*/
			//  read one sequence from file
			// get the sequence
			if ((errorcode = fread(&(seq_data.seq), sizeof(uint64_t), 1, seqf)) != 1)
			{/*{{{*/
				if (errorcode == 0 && feof(seqf))
					continue;
				readerror(seq_fname[i]);
			}/*}}}*/

			// get the flag
			if (((seq_data.seq >> SIGNBITS) & SIGNMASK) != 0)    // the sequence has unknown bases
			{/*{{{*/
				if ((errorcode = fread(&(seq_data.flag), sizeof(uint32_t), 1, seqf)) != 1)
					readerror(seq_fname[i]);
			}/*}}}*/
			else
				seq_data.flag = 0;

			//  check the block being in use, if not matched anymore, refresh it
			index_num = seq_data.seq >> 48;
			if (fir_num != index_num)
			{/*{{{*/
				// check error case ( the seqences are ordered )
				if (fir_num > index_num)
				{
					printf("fir_index bad order in file '%s'\n", seq_fname[i]);
					readerror(seq_fname[i]);
				}

				// the block is not matched, needed to get the right block
				fseek(sinf, start_loc[index_num], SEEK_SET);
				fir_num = index_num;

				if ((length = fir_index[index_num].num - fir_index[index_num].start)  >  BLOCKSIZE)
				{
					is_int64 = false;
					//  this block has second index, get the index
					fread(sec_index, sizeof(RECORD), DEGREE, sinf);
					//  get the block with the index_num of the second index
					index_num = (seq_data.seq >> 32) & 0xffff;
					fseek(sinf, sizeof(uint32_t) * sec_index[index_num].start, SEEK_CUR);
					sec_num = index_num;

					length = sec_index[index_num].num - sec_index[index_num].start;
					buff_32 = (uint32_t*)malloc(sizeof(uint32_t) * length);
					if (fread(buff_32, sizeof(uint32_t), length, sinf) != length)
					{
						printf("File Read ERROR: bad format file '%s'\n", sin_fname[i]);
						ali_end();
						exit(-1);
					}

					refers.clear();
					for (int k = 0; k < length; k++)
						refers.push_back(buff_32[k]);

					free(buff_32);
					buff_32 = NULL;
				}
				else
				{
					is_int64 = true;
					//  this block has no second index, so just get the block directly
					sec_num = -1;
					buff_64 = (uint64_t*)malloc(sizeof(uint64_t) * length);
					if (fread(buff_64, sizeof(uint64_t), length, sinf) != length)
					{
						printf("File Read ERROR: bad format file '%s'\n", sin_fname[i]);
						ali_end();
						exit(-1);
					}

					refers.clear();
					for (int k = 0; k < length; k++)
						refers.push_back(buff_64[k]);

					free(buff_64);
					buff_64 = NULL;
				}

				//  init the find_start
				find_start = 0;
			}/*}}}*/
			else if (!is_int64 && (index_num = (seq_data.seq >> 32) & 0xffff) != sec_num)    //  the block associated with second index is not matched 
			{/*{{{*/
				// check error case ( the seqences are ordered )
				if (sec_num > index_num)
				{
					printf("sec_index bad order in file '%s'\n", seq_fname[i]);
					readerror(seq_fname[i]);
				}

				//  get the block with the index_num of the second index
				fseek(sinf, sizeof(uint32_t) * (sec_index[index_num].start - sec_index[sec_num].num), SEEK_CUR);   // sec_num = -1? That is not a problem
				sec_num = index_num;

				length = sec_index[sec_num].num - sec_index[sec_num].start;
				buff_32 = (uint32_t*)malloc(sizeof(uint32_t) * length);
				if (fread(buff_32, sizeof(uint32_t), length, sinf) != length)
				{
					printf("File Read ERROR: bad format file '%s'\n", sin_fname[i]);
					ali_end();
					exit(-1);
				}

				refers.clear();
				for (int k = 0; k < length; k++)
					refers.push_back(buff_32[k]);

				free(buff_32);
				buff_32 = NULL;

				find_start = 0;
			}/*}}}*/

			//  find the first matched sequence from the block
			if (!is_int64)
				position = refers.find(seq_data.seq & MASK32, &find_start, cmp_seq);
			else
				position = refers.find(seq_data.seq & MASK48, &find_start, cmp_seq);

			// store the position
			dup_position = position;

			//  write result to file, and find the others matched sequence from the block
			while (position != -1)
			{/*{{{*/
				find_start++;

				if (!is_int64)
					ref_data = (fir_num << 48) + (sec_num << 32) + *(refers.get(position));
				else
					ref_data = (fir_num << 48) + *(refers.get(position));

				writeResult(ref_data, seq_data, resultf, i);

				if (!is_int64)
					position = refers.find(seq_data.seq & MASK32, &find_start, cmp_seq);
				else
					position = refers.find(seq_data.seq & MASK48, &find_start, cmp_seq);
			}/*}}}*/

			// recover the position
			if ( dup_position != -1)
				find_start = dup_position;

		}/*}}}*/

		fclose(sinf);
		fclose(seqf);
		sinf = seqf = NULL;
	}/*}}}*/

	// flush the file buff
	fflush(resultf);
	free(fin_buff);
	free(fout_buff);
	fin_buff = fout_buff = NULL;

	ali_end();
	fclose(resultf);
	printf(" alignment end \n");
}/*}}}*/


MY_VECTOR<uint64_t>    ALI::refers;
RECORD*			ALI::fir_index = 0;
RECORD*			ALI::sec_index = 0;
uint64_t*		ALI::start_loc = 0;
char    ALI::fin_fname[2][MAXFNAME];
char    ALI::sin_fname[2][MAXFNAME];
char    ALI::seq_fname[2][MAXFNAME];
