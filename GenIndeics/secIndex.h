#pragma	once
#ifndef    SEC_INDEX
#define    SEC_INDEX

#include "Utility.h"



namespace   SEC
{

	typedef    struct
	{
		RAW_DATA    *data;
		int        count;
		int        size;
	}    RAW_BUFF;

	extern RECORD    *fir_index;
	extern uint64_t  *start_loc;
	extern RECORD    *sec_index;
	extern RAW_BUFF                rbuff;
	extern MY_VECTOR<uint32_t>     *sec_data;

	void    sec_init();

	void    sec_clean();

	bool    u32_cmp(const uint32_t &a, const uint32_t &b);

	void    set_sec(FILE    *mid_file, char const * fir_name, char const * sec_name);
};

#endif