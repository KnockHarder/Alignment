#include "firIndex.h"
#include <string.h>

void    FIR::fir_init()
{/*{{{*/
    for( int i = 0; i < 4; i ++ )
    {
        fir_index[i] = (RECORD *)malloc(sizeof(RECORD) * DEGREE);
        memset( fir_index[i], 0, sizeof(RECORD)*DEGREE);
        groups[i] = new    MY_VECTOR<RAW_DATA>[DEGREE];
    }
}/*}}}*/

void    FIR::fir_end()
{/*{{{*/
    for( int i = 0; i < 4; i ++ )
    {
        free(fir_index[i]);
        fir_index[i] = NULL;

        delete[] groups[i];
        groups[i] = NULL;
    }
}/*}}}*/

void    FIR::add_expand(uint64_t data, int length, int max_pad, MY_VECTOR<RAW_DATA> *grou, RECORD* index)
{/*{{{*/
	uint64_t    tdata;    // transformed data
	uint32_t    padder, h_pad, t_reser, inc;

	for (int i = 0; i <= max_pad; i++)
	{
		h_pad = i;
		t_reser = MAXNT - length - i;

		inc = 1 << (i * 2);
		for (int j = 0; j<inc; j++)
		{
			tdata = data >> (i * 2);
			padder = (uint64_t)j << (64 - i * 2);
			tdata += padder;
			tdata += t_reser;
			tdata += h_pad << SIGNBITS;

			index_num = tdata >> 48;
			new_item = tdata & MASK48;  //Only the 48 lower biis are useful now


			grou[index_num].push_back(new_item);
			index[index_num].num++;
		}
	}
}/*}}}*/

void    FIR::add_shrink(uint64_t data, int length, int max_cut, MY_VECTOR<RAW_DATA> *grou, RECORD* index)
{/*{{{*/
	uint64_t    tdata;
	uint32_t    h_pad, t_reser;    // Operation here is cutting the higher 1 ~ max_cut nt of the sequence, so h_pad is negative ( represented as number between 32 ~ 65 )
	uint32_t    shrinked;   // the bits be shrinked

	for (int i = 1; i <= max_cut; i++)
	{
		shrinked = data >> ((32 - i) * 2);
		tdata = data << (i * 2);
		h_pad = 16 + i;  // the fifth bit is flag bit
		t_reser = MAXNT - length + i;

		tdata += t_reser;
		tdata += h_pad << SIGNBITS;
		tdata += shrinked << (SIGNBITS * 2);

		index_num = tdata >> 48;
		new_item = tdata & MASK48;  //Only the 48 lower biis are useful now

		grou[index_num].push_back(new_item);
		index[index_num].num++;
		if (t_reser > MAXNT - MINNT)
			printf("something strange in \n");
	}
}/*}}}*/

void    FIR::add_data(uint64_t data[2], MY_VECTOR<RAW_DATA>    *groups[2],RECORD*   fir_index[2], int length)
{/*{{{*/
    //expand sequence ------------------------------
    add_expand(data[0], length, (MAXNT - length + 1) / 2, groups[0], fir_index[0]);
    add_expand(data[1], length, (MAXNT - length) / 2, groups[1], fir_index[1]);
    
    // shrink sequence ------------------------------ 
    add_shrink(data[0], length, length - MINNT, groups[0], fir_index[0]);
    add_shrink(data[1], length, length - MINNT, groups[1], fir_index[1]);
    
}/*}}}*/

void    FIR::set_fir(FILE *referf, const char fnames[4][FILENAME_MAX])
{/*{{{*/
	printf(" fir_index start \n");
	fir_init();

    // declear variables
    /*{{{*/
    MY_VECTOR<RAW_DATA>    *tempgroups[2]; // one and reverse of it 
	uint64_t    data[2]; // normal and reverse of one sequence
	uint64_t    compleData[2]; // complement of the above two sequence
	uint64_t    tdata;
	uint64_t    padder;

	uint64_t     inc;
	char    seq[STRLEN];
	char    ch;
	int     count;
    /*}}}*/

    // translate
	while (!feof(referf))
	{/*{{{*/
        // read one sequence
        /*{{{*/
		if (!fgets(seq, STRLEN, referf))
		{
			printf("File Read Error?\n");
			continue;
		}
		if (seq[0] == '\n' || seq[0] == '\0')
			continue;

		seq[STRLEN - 1] = '\0';
        /*}}}*/

        // translat sequence and the reverse 
        // then get the complementary two
        /*{{{*/
		data[0] = data[1] = 0;
        compleData[0] = compleData[1] = 0;

		for (count = 0; !((ch = seq[count]) == '\0' || ch == '\n' || ch == ' '); count++)
		{
			inc = (ch >> 1) & 3;
			data[0] = data[0] << 2;
			data[0] += inc;
		}

        // check the length of the sequence
        /*{{{*/
		if (count < MINNT || count > MAXNT)
			continue;
        /*}}}*/

        compleData[0] = ( data[0] ^ COMPMASK ) << (64 - count * 2 );
		data[0] = data[0] << (64 - count * 2);

		for (int i = 0; i < count; i++)
		{
			inc = (seq[i] >> 1) & 3;
			data[1] = data[1] >> 2;
			data[1] += inc << 62;
		}
        compleData[1] = ( data[1] ^ COMPMASK ) >> (64 - count * 2) << (64 - count * 2);

        /*}}}*/

        // put the two translated data into buckets
        // then put complementary of the two translated data into buckets
		add_data(data, groups, fir_index, count);
        add_data(compleData, groups + 2, fir_index + 2, count );

	}/*}}}*/

    // save indices 
    for( int i = 0; i < 4; i ++ )
    {/*{{{*/
        for (int j = 1; j<DEGREE; j++)
        { 
            fir_index[i][j].start = fir_index[i][j - 1].num;
            fir_index[i][j].num += fir_index[i][j - 1].num;
        }

        FILE    *fp = 0;

        if ( ( fp = fopen( fnames[i], "wb" ) )  ==  0 )
        {
            printf("File Open Error:Can`t open file '%s'\n", fnames[i]);
            fir_end();
            exit(-1);
        }

        if (fwrite( fir_index[i], sizeof(RECORD), DEGREE, fp ) != DEGREE)
        {
            printf("File Writing Error:Can`t write all data to first_index\n");
            fir_end();
            exit(-1);
        }

        for (int j = 0; j<DEGREE; j++)
            groups[i][j].outputf( fp );

        fclose( fp );

    }/*}}}*/

	fir_end();
	printf(" fir_index end \n");
}/*}}}*/

RECORD*                 FIR::fir_index[4] = { 0, 0, 0, 0};
MY_VECTOR<RAW_DATA>*    FIR::groups[4] = { 0, 0, 0, 0};
uint16_t			FIR::index_num = 0;
RAW_DATA			FIR::new_item = 0;
