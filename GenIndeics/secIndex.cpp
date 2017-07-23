#include "secIndex.h"
#include <string.h>
#include <algorithm>

void    SEC::sec_init()
{/*{{{*/
	fir_index = (RECORD *)malloc(sizeof(RECORD) * DEGREE);
	sec_index = (RECORD *)malloc(sizeof(RECORD) * DEGREE);
	start_loc = (uint64_t*)malloc(sizeof(uint64_t) * (DEGREE + 1));
	memset(fir_index, 0, sizeof(RECORD)*DEGREE);
	memset(sec_index, 0, sizeof(RECORD)*DEGREE);
	memset(start_loc, 0, sizeof(uint64_t) * (DEGREE + 1));

	rbuff.data = NULL;
	rbuff.count = rbuff.size = 0;
	sec_data = new    MY_VECTOR<uint32_t>[DEGREE];
}/*}}}*/

void    SEC::sec_clean()
{/*{{{*/
	free(fir_index);
	free(sec_index);
	free(rbuff.data);
	free(start_loc);
	fir_index = NULL;
	sec_index = NULL;
	(rbuff.data) = NULL;
	rbuff.count = rbuff.size = 0;
	start_loc = NULL;

	delete[]sec_data;
	sec_data = NULL;
}/*}}}*/

bool    SEC::u32_cmp(const uint32_t &a, const uint32_t &b)
{/*{{{*/
	uint32_t     a_idle_bits = (a & SIGNMASK) * 2;
	uint32_t     b_idle_bits = (b & SIGNMASK) * 2;
	uint32_t     max = (a_idle_bits > b_idle_bits) ? a_idle_bits : b_idle_bits;

	uint32_t    x = a >> (max + SIGNBITS * 2), y = b >> (max + SIGNBITS * 2);
	if (x < y)
		return true;
	else if (x == y)
		if (a_idle_bits > b_idle_bits)
			return true;

	return false;
}/*}}}*/

void    SEC::set_sec(FILE    *mid_file, char const * fir_name, char const * sec_name)
{/*{{{*/
	//  initiate
	printf(" set_sec start \n");
	sec_init();

	//  get first index from the file
	if (fread(fir_index, sizeof(RECORD), DEGREE, mid_file) != DEGREE)
	{/*{{{*/
		printf("File Error:The first-index file is ont applicable!\n");
		sec_clean();
		exit(-1);
	}/*}}}*/

	//  open the file and write second index into 
	FILE    *sec_file = 0;

//  if (fopen_s(&sec_file, sec_name, "wb") != 0  ||  sec_file == NULL)
    if ( ( sec_file = fopen(sec_name, "wb") ) == 0 )
	{/*{{{*/
		printf("Failed to open file \'%s\' \n", sec_name);
		sec_clean();
		exit(-1);
	}/*}}}*/

	start_loc[0] = 0;
	uint32_t    a = 0, b = 0;

	for (int i = 0; i < DEGREE; i++)
	{/*{{{*/
		//  get the number of the sequence with the same first index number
		rbuff.count = fir_index[i].num - fir_index[i].start;

		//  read these sequences into the rebuff
		if (rbuff.count > 0)
		{
			if (rbuff.size < rbuff.count)
			{
				free(rbuff.data);
				(rbuff.data) = (RAW_DATA*)malloc(sizeof(RAW_DATA) * rbuff.count);
				rbuff.size = rbuff.count;
			}

			if (fread((rbuff.data), sizeof(RAW_DATA), rbuff.count, mid_file) != rbuff.count)
			{
				printf("File Error:The first-index file is ont applicable!\n");
				sec_clean();
				exit(-1);
			}
		}

		//  init the value of the the next block start location
		start_loc[i + 1] = start_loc[i];  // set init value
		//Warning: the length of arry start_loc must be larger than DEGREE
		//  set the second index and save
		if (rbuff.count <= BLOCKSIZE)
		{
			std::sort(rbuff.data, rbuff.data + rbuff.count);
			fwrite(rbuff.data, sizeof(uint64_t), rbuff.count, sec_file);
			start_loc[i + 1] = start_loc[i + 1] + sizeof(uint64_t) * rbuff.count;
		}
		else
		{
			//  init the second index
			memset(sec_index, 0, sizeof(RECORD) * DEGREE);
			for (int j = 0; j < DEGREE; j++)
				sec_data[j].clear();

			//  set the second index
			for (int j = 0; j < rbuff.count; j++)
			{
				a = (rbuff.data)[j] >> 32;
				b = (rbuff.data)[j] & MASK32;
				sec_data[a].push_back(b);
				sec_index[a].num++;
			}
			for (int j = 1; j<DEGREE; j++)
			{
				sec_index[j].start = sec_index[j - 1].num;
				sec_index[j].num += sec_index[j - 1].num;
			}

			//  save to file
			fwrite(sec_index, sizeof(RECORD), DEGREE, sec_file);
			start_loc[i + 1] += sizeof(RECORD) * DEGREE;
			for (int j = 0; j < DEGREE; j++)
			{
				sec_data[j].sort(u32_cmp);
				sec_data[j].outputf(sec_file);
				start_loc[i + 1] += sizeof(uint32_t) * sec_data[j].getlength();
			}
		}
	}/*}}}*/

	fclose(sec_file);
	//  save the fir index and location information
	FILE*   fir_file = NULL;
	fir_file = fopen(fir_name, "wb");
	if (fir_file == NULL)
	{
		printf("Failed to open file \'%s\' \n", fir_name);
		sec_clean();
		exit(-1);
	}

	fwrite(fir_index, sizeof(RECORD), DEGREE, fir_file);
	fwrite(start_loc, sizeof(uint64_t), DEGREE, fir_file);
	fclose(fir_file);

	sec_clean();
	printf(" sec_sec end \n");
}/*}}}*/

RECORD*		            SEC::fir_index = 0;
uint64_t*	            SEC::start_loc = 0;
RECORD*		            SEC::sec_index = 0;
SEC::RAW_BUFF           SEC::rbuff;
MY_VECTOR<uint32_t>*    SEC::sec_data = 0;
