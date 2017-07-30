#include "align.h"
#include <string.h>

void    ALI::ali_init(char indicesFnames[8][MAXFNAME], char seqFames[2][MAXFNAME])
{/*{{{*/
	FILE	*fp = 0;

	for (int i = 0; i < 8; i++)
	{
	    // if ( fopen_s( &fp, filenames[i], "rb") != 0  ||  fp == NLL)
	    if ( ( fp = fopen( indicesFnames[i], "rb") )  ==  0 )
		{
			printf("Can`t open file \'%s\' correctly\n", indicesFnames[i]);
			exit(-1);
		}
		fclose(fp);
	}
	for (int i = 0; i < 2; i++)
	{
	    // if ( fopen_s( &fp, filenames[i], "rb") != 0  ||  fp == NLL)
	    if ( ( fp = fopen( seqFames[i], "rb") )  ==  0 )
		{
			printf("Can`t open file \'%s\' correctly\n", seqFames[i]);
			exit(-1);
		}
		fclose(fp);
	}

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

void    ALI::writeResult(const uint64_t   &refer, const SEQWITHN    &seq, FILE*  rsf, int flag, int isrev)
{/*{{{*/

    // declears variables and init
    /*{{{*/
    char    str[40];
	char*   term = (char*)malloc(sizeof(char) * 200);
	char*   temp = 0;
    int     hitStart , hitEnd, queryStart, queryEnd;
    
	char*   refstr = (char*)malloc(sizeof(char) * (MAXNT * 1));
	char*   seqstr = (char*)malloc(sizeof(char) * (MAXNT + 1));
	int     length;
	int     pad, reserv, k;

    /*}}}*/

    // get the hist sequence and the match part
    /*{{{*/

	pad = (refer >> SIGNBITS) & SIGNMASK;
	reserv = refer & SIGNMASK;
    k = seq.seq & SIGNMASK;


    if( reserv > k )
        hitEnd = queryEnd = MAXNT - reserv;
    else
        hitEnd = queryEnd = MAXNT - k;


    // recovery for complementary sequence
    /*{{{*/
    if( flag / 2 == 1 )
    {
        temp = toString( refer ^ COMPMASK );
    }
    else
        temp = toString(refer);
    /*}}}*/

    // get the hit sequence
    /*{{{*/
	if (pad < 16) // padded or normal
	{
        hitStart = 1;
        hitEnd -= pad;
        queryStart = pad + 1;

		length = MAXNT - pad - reserv;
	//  strncpy_s(refstr, (MAXNT * 2 + 10), temp + pad, length);
	    strncpy(refstr, temp + pad, length);
		refstr[length] = '\0';
	}
	else    // shrinked
	{
		pad = pad - 16;

        hitStart = pad + 1;
        hitEnd += pad;
        queryStart = 1;

		length = MAXNT + pad - reserv;
		for (int i = 0; i < pad; i++)
			refstr[i] = temp[MAXNT - pad + i];
		for (int i = 0; i < MAXNT - reserv; i++)
			refstr[pad + i] = temp[i];
		refstr[length] = '\0';
	}
    /*}}}*/

    // check length
	if (length < MINNT || length > MAXNT)
	{
		printf(" length error in 'alignment', ref = %s\n", refstr);
		printf("pad = %d \t reserv = %d \n", pad, reserv);
	}

    // recover from reverse
    /*{{{*/
	if ( flag % 2 == 1 )    // is reverse
	{
        k = hitStart;
        hitStart = length - hitEnd + 1;
        hitEnd  = length - k + 1;

		for (int i = 0; i < length; i++)
			temp[i] = refstr[length - 1 - i];
	//  strncpy_s(refstr, (MAXNT * 2 + 10), temp, length);
	    strncpy(refstr, temp, length);
		refstr[length] = '\0';
	}
    /*}}}*/

	free(temp);
	temp = NULL;

    /*}}}*/

    // get the query sequence
    /*{{{*/
	temp = recoverSeq(seq);
	length = MAXNT - (seq.seq & SIGNMASK);

    // check length
	if (length < MINNT || length > MAXNT)
		printf(" length error in 'alignment', seq = %s\n", seqstr);

    // strncpy_s(seqstr, (MAXNT + 1), temp, length);
    strncpy(seqstr, temp, length);
	seqstr[length] = '\0';
    
    // recover from reverse of query sequence
    /*{{{*/
	if (isrev == 1)
	{
        k = queryStart;
        queryStart = length - queryEnd + 1;
        queryEnd  = length - k + 1;

		for (int i = 0; i < length; i++)
			temp[i] = seqstr[length - 1 - i];
	//  strncpy_s(seqstr, (MAXNT + 1), temp, length);
	    strncpy(seqstr, temp, length);
		seqstr[length] = '\0';
	}
    /*}}}*/

	free(temp);
	temp = NULL;
    /*}}}*/

    // format output
    /*{{{*/
    if( flag % 2 != isrev )
    {
            k = hitStart;
            hitStart = hitEnd;
            hitEnd = k;
    }

    sprintf( str, "\t%d\t%d\t", hitStart, hitEnd );
    strcpy( term, refstr );
    strcat( term, str );

    sprintf( str, "\t%d\t%d", queryStart, queryEnd );
    strcat( term, seqstr );
    strcat( term, str );

	fputs( term, rsf );
	fputs( "\r\n", rsf );
    /*}}}*/

	free(refstr);
	free(seqstr);
	refstr = seqstr = NULL;
}/*}}}*/

void    ALI::alignment(char indices_fnames[8][MAXFNAME], char seq_fname[2][MAXFNAME], const char* rsf_fname)
{/*{{{*/
	printf(" alignment start \n");
	ali_init(indices_fnames, seq_fname);

	//  declear variables
    /*{{{*/
	FILE        *finf, *sinf, *seqf, *resultf;
	int64_t     fir_num = -1;    // record the first index number 
                                //which related to the second index
	int64_t     sec_num = -1;
	bool        is_int64 = false;       // which block is used, 
                                        //32-bit data or 64-bit data
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
	char        *fout_buff = NULL;
    /*}}}*/

	// Init the file buffs
    /*{{{*/
	fin_buff = (char*)malloc(MY_BUFSIZ);
	fout_buff = (char*)malloc(MY_BUFSIZ);
	if (fin_buff == NULL || fout_buff == NULL)
	{
		printf("Error: Failed to allocate memory \n");
		exit(-1);
	}
    /*}}}*/

	// open the file saved results
    /*{{{*/
    // if (fopen_s(&resultf, rsf_fname, "wt") != 0 || resultf == NULL)
	if ( ( resultf = fopen( rsf_fname, "wt") )  ==  0 )
	{
		printf("Error: Can`t open the file '%s'\n", rsf_fname);
		ali_end();
		exit(-1);
	}
	setvbuf(resultf, fout_buff, _IOFBF, MY_BUFSIZ);
    /*}}}*/

    // Query with 4 kinds of matches, and write the result into file
    /*{{{*/
    for( int i = 0; i < 4; i ++ )
    {
        // prepare the hit index and reverse hit index
        fin_fname[0] = indices_fnames[ 0 ^ i ];
        fin_fname[1] = indices_fnames[ 1 ^ i ];

        sin_fname[0] = indices_fnames[ 4 ^ i ];
        sin_fname[1] = indices_fnames[ 5 ^ i ];

        // alignment on both side
        /*{{{*/
        for (int j = 0; j < 2; j++) 
        {
            fir_num = sec_num = -1;
            is_int64 = false;

            //  open the files 
            /*{{{*/
            //  if (fopen_s(&finf, fin_fname[j], "rb") != 0  ||  finf == NULL)
            if ( ( finf = fopen( fin_fname[j], "rb") )  ==  0 )
            {
                printf("Open file \'%s\' failed\n", fin_fname[j]);
                ali_end();
                exit(-1);
            }

            //  if (fopen_s(&sinf, sin_fname[j], "rb") != 0  || sinf == NULL)
            if ( ( sinf = fopen( sin_fname[j], "rb") )  ==  0 )
            {
                printf("Open file \'%s\' failed\n", sin_fname[j]);
                ali_end();
                exit(-1);
            }
            
            //  if (fopen_s(&seqf, seq_fname[j], "rb") != 0  || seqf == NULL)
            if ( ( seqf = fopen( seq_fname[j], "rb") )  ==  0 )
            {
                printf("Open file \'%s\' failed\n", seq_fname[j]);
                ali_end();
                exit(-1);
            }
            setvbuf(seqf, fin_buff, _IOFBF, MY_BUFSIZ);
            /*}}}*/

            //  read first index and the start location in the file of the index[j] 
            /*{{{*/
            if (fread(fir_index, sizeof(RECORD), DEGREE, finf) != DEGREE)
            {
                printf("File \'%s\' has bad data\n", fin_fname[j]);
                ali_end();
                exit(-1);
            }

            if (fread(start_loc, sizeof(uint64_t), DEGREE, finf) != DEGREE)
            {
                printf("File \'%s\' has bad data\n", fin_fname[j]);
                ali_end();
                exit(-1);
            }
            /*}}}*/

            //  get the sequence and search it with the index one by one
            /*{{{*/
            while (!feof(seqf))
            {
                //  read one sequence from file
                /*{{{*/
                if ((errorcode = fread(&(seq_data.seq), sizeof(uint64_t), 1, seqf)) != 1)
                {
                    if (errorcode == 0 && feof(seqf))
                        continue;
                    readerror(seq_fname[j]);
                }
                /*}}}*/

                // get the flag
                /*{{{*/
                if (((seq_data.seq >> SIGNBITS) & SIGNMASK) != 0)    
                // the sequence has unknown bases
                {
                    if ((errorcode = fread(&(seq_data.flag), sizeof(uint32_t), 1, seqf)) != 1)
                        readerror(seq_fname[j]);
                }
                else
                    seq_data.flag = 0;
                /*}}}*/

                //  check the block being in use, if not matched anymore, refresh it
                /*{{{*/
                // get first index
                index_num = seq_data.seq >> 48;

                if (fir_num != index_num)
                // the block is not matched when check first index
                {
                    // check error case ( the seqences are ordered )
                    if (fir_num > index_num)
                    {
                        printf("fir_index bad order in file '%s'\n", seq_fname[j]);
                        readerror(seq_fname[j]);
                    }

                    // get the right block
                    /*{{{*/
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
                            printf("File Read ERROR: bad format file '%s'\n", sin_fname[j]);
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
                            printf("File Read ERROR: bad format file '%s'\n", sin_fname[j]);
                            ali_end();
                            exit(-1);
                        }

                        refers.clear();
                        for (int k = 0; k < length; k++)
                            refers.push_back(buff_64[k]);

                        free(buff_64);
                        buff_64 = NULL;
                    }
                    /*}}}*/

                    //  init the find_start
                    find_start = 0;
                }
                else if (!is_int64 && (index_num = (seq_data.seq >> 32) & 0xffff) != sec_num)    
                // the block is not matched when check second index
                {
                    // check error case ( the seqences are ordered )
                    if (sec_num > index_num)
                    {
                        printf("sec_index bad order in file '%s'\n", seq_fname[j]);
                        readerror(seq_fname[j]);
                    }

                    //  get the block with the index_num of the second index
                    /*{{{*/
                    fseek(sinf, sizeof(uint32_t) * (sec_index[index_num].start - sec_index[sec_num].num), SEEK_CUR);   // sec_num = -1? That is not a problem
                    sec_num = index_num;

                    length = sec_index[sec_num].num - sec_index[sec_num].start;
                    buff_32 = (uint32_t*)malloc(sizeof(uint32_t) * length);
                    if (fread(buff_32, sizeof(uint32_t), length, sinf) != length)
                    {
                        printf("File Read ERROR: bad format file '%s'\n", sin_fname[j]);
                        ali_end();
                        exit(-1);
                    }

                    refers.clear();
                    for (int k = 0; k < length; k++)
                        refers.push_back(buff_32[k]);

                    free(buff_32);
                    buff_32 = NULL;
                    /*}}}*/

                    find_start = 0;
                }
                /*}}}*/

                // get hits and write into file
                /*{{{*/
                //  find the first matched sequence from the block
                if (!is_int64)
                    position = refers.find(seq_data.seq & MASK32, &find_start, cmp_seq);
                else
                    position = refers.find(seq_data.seq & MASK48, &find_start, cmp_seq);

                // store the position
                dup_position = position;

                //  write result to file, and find the others matched sequence from the block
                while (position != -1)
                {
                    find_start++;

                    if (!is_int64)
                        ref_data = (fir_num << 48) + (sec_num << 32) + *(refers.get(position));
                    else
                        ref_data = (fir_num << 48) + *(refers.get(position));

                    writeResult(ref_data, seq_data, resultf, i ^ j, j);

                    if (!is_int64)
                        position = refers.find(seq_data.seq & MASK32, &find_start, cmp_seq);
                    else
                        position = refers.find(seq_data.seq & MASK48, &find_start, cmp_seq);
                }

                // recover the position
                if ( dup_position != -1)
                    find_start = dup_position;
                /*}}}*/

            }
            /*}}}*/

            fclose(finf);
            fclose(sinf);
            fclose(seqf);
            finf = sinf = seqf = NULL;
        }
        /*}}}*/
    }
    /*}}}*/

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
char*   ALI::fin_fname[2] = { 0, 0 };
char*   ALI::sin_fname[2] = { 0, 0 };
