#include "firIndex.h"
#include <string.h>

void    FIR::fir_init()
{
	fir_index = (RECORD *)malloc(sizeof(RECORD) * DEGREE);
	fir_reverse = (RECORD *)malloc(sizeof(RECORD) * DEGREE);
	memset( fir_index, 0, sizeof(RECORD)*DEGREE);
	memset( fir_reverse, 0, sizeof(RECORD)*DEGREE);
	groups = new    MY_VECTOR<RAW_DATA>[DEGREE];
	regrou = new    MY_VECTOR<RAW_DATA>[DEGREE];
}

void    FIR::fir_end()
{
	free(fir_index);
	free(fir_reverse);
	fir_index = NULL;
	fir_reverse = NULL;

	delete[] groups;
	delete[] regrou;
	groups = NULL;
	regrou = NULL;
}

void    FIR::add_expand(uint64_t data, int length, int max_pad, MY_VECTOR<RAW_DATA> *grou, RECORD* index)
{
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
}

void    FIR::add_shrink(uint64_t data, int length, int max_cut, MY_VECTOR<RAW_DATA> *grou, RECORD* index)
{
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
}

void    FIR::add_data(uint64_t data, uint64_t reverdata, int length)
{
	//expand sequence ------------------------------
	add_expand(data, length, (MAXNT - length + 1) / 2, groups, fir_index);
	add_expand(reverdata, length, (MAXNT - length) / 2, regrou, fir_reverse);
	// shrink sequence ------------------------------ 
	add_shrink(data, length, length - MINNT, groups, fir_index);
	add_shrink(reverdata, length, length - MINNT, regrou, fir_reverse);
}

void    FIR::set_fir(FILE *referf, char const *nor_name, char const *rev_name)
// nor_name is the file name of the first-index for normal sequence, rev_name is for reverse one
{
	printf(" fir_index start \n");
	fir_init();

	uint64_t    data, reverdata;
	uint64_t    tdata;
	uint64_t    padder;

	uint64_t     inc;
	char    seq[STRLEN];
	char    ch;
	int     count;


	while (!feof(referf))
	{
		if (!fgets(seq, STRLEN, referf))
		{
			printf("File Read Error?\n");
			break;
		}
		if (seq[0] == '\n' || seq[0] == '\0')
			continue;

		seq[STRLEN - 1] = '\0';
		data = reverdata = 0;

		for (count = 0; !((ch = seq[count]) == '\0' || ch == '\n' || ch == ' '); count++)
		{
			inc = (ch >> 1) & 3;
			data = data << 2;
			data += inc;
		}
		data = data << (64 - count * 2);

		for (int i = 0; i < count; i++)
		{
			inc = (seq[i] >> 1) & 3;
			reverdata = reverdata >> 2;
			reverdata += inc << 62;
		}

		if (count < MINNT || count > MAXNT)
			continue;

		add_data(data, reverdata, count);
	}

	for (int i = 1; i<DEGREE; i++)
	{ 
		fir_index[i].start = fir_index[i - 1].num;
		fir_index[i].num += fir_index[i - 1].num;

		fir_reverse[i].start = fir_reverse[i - 1].num;
		fir_reverse[i].num += fir_reverse[i - 1].num;
	}

	FILE    *normal = 0, *reverse = 0;

    if ( ( normal = fopen( nor_name, "wb" ) )  ==  0 )
	{
		printf("File Open Error:Can`t open file '%s'\n", nor_name);
		fir_end();
		exit(-1);
	}

	if (fwrite( fir_index, sizeof(RECORD), DEGREE, normal) != DEGREE)
	{
		printf("File Writing Error:Can`t write all data to first_index\n");
		fir_end();
		exit(-1);
	}


	for (int i = 0; i<DEGREE; i++)
		groups[i].outputf(normal);
	fclose(normal);

	reverse = fopen(rev_name, "wb");
	if (fwrite( fir_reverse, sizeof(RECORD), DEGREE, reverse) != DEGREE)
	{
		printf("File Writing Error:Can`t write all data to first_index\n");
		fir_end();
		exit(-1);
	}

	for (int i = 0; i<DEGREE; i++)
		regrou[i].outputf(reverse);
	fclose(reverse);

	fir_end();
	printf(" fir_index end \n");
}

RECORD*                 FIR::fir_index = 0;
RECORD*                 FIR::fir_reverse = 0;
MY_VECTOR<RAW_DATA>*    FIR::groups = 0;
MY_VECTOR<RAW_DATA>*	FIR::regrou = 0;
uint16_t			FIR::index_num = 0;
RAW_DATA			FIR::new_item = 0;
