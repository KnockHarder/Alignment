/*
This function will generate 4 indices ( 00, 01, 10, 11 ).
For Nth index, if the binary format of N is 'ab' ( a/b is 0 or 1 ).
When a is 1, the sequences is complementary.
When b is 1, the sequences is in reverse.
*/

#pragma	once
#ifndef    FIRST_INDEX
#define    FIRST_INDEX

#include "Utility.h"

namespace   FIR
{
	extern	RECORD                 *fir_index[4];
	extern	MY_VECTOR<RAW_DATA>    *groups[4];
	extern	uint16_t            index_num;
	extern	RAW_DATA            new_item;

	void	fir_init();

	void    fir_end();

	void    add_expand(uint64_t data, int length, int max_pad, MY_VECTOR<RAW_DATA> *grou, RECORD* index);

	void    add_shrink(uint64_t data, int length, int max_cut, MY_VECTOR<RAW_DATA> *grou, RECORD* index);

    void    add_data(uint64_t data[2], MY_VECTOR<RAW_DATA>    *groups[2], RECORD*   fir_index[2], int length);

	void    set_fir(FILE *referf, const char fnames[4][FILENAME_MAX]);
};

#endif
