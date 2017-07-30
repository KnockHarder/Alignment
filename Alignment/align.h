#ifndef    MY_ALIGN
#define    MY_ALIGN

#include "Utility.h"

#define     MAXFNAME    200
#define     MY_BUFSIZ   10240

namespace   ALI
{
	extern	MY_VECTOR<uint64_t>    refers;  // the references sequences saved as 64-bit or 32-bit integer in a same block
	extern	RECORD       *fir_index;
	extern	RECORD       *sec_index;
	extern	uint64_t     *start_loc;
	extern	char*   fin_fname[2];
	extern	char*   sin_fname[2];

    void    ali_init(char indicesFnames[8][MAXFNAME], char seqFames[2][MAXFNAME]);

	void    ali_end();

	char*   toString(uint64_t  data);

	char*   recoverSeq(const  SEQWITHN  &seq);

	void    readerror(char const * filename);

	int    cmp_seq(const uint64_t &refer, const uint64_t &seq);

    void    writeResult(const uint64_t   &refer, const SEQWITHN    &seq, FILE*  rsf, int flag, int isrev);

	void    alignment(char indices_fnames[8][MAXFNAME], char seq_fname[2][MAXFNAME], const char* rsf_fname);

};

#endif
