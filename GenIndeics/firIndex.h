#pragma	once
#ifndef    FIRST_INDEX
#define    FIRST_INDEX

#include "Utility.h"


namespace   FIR
{
	extern	RECORD                 *fir_index;
	extern	RECORD                 *fir_reverse;
	extern	MY_VECTOR<RAW_DATA>    *groups, *regrou;
	extern	uint16_t            index_num;
	extern	RAW_DATA            new_item;

	void	fir_init();

	void    fir_end();

	void    add_expand(uint64_t data, int length, int max_pad, MY_VECTOR<RAW_DATA> *grou, RECORD* index);

	void    add_shrink(uint64_t data, int length, int max_cut, MY_VECTOR<RAW_DATA> *grou, RECORD* index);

	void    add_data(uint64_t data, uint64_t reverdata, int length);

	void    set_fir(FILE *referf, char const *nor_name, char const *rev_name);
};

#endif
