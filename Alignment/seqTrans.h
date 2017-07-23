/********************************************
*The sequence of experiment data is also represented with 64 bits.  The higher 50 or less
*bits store sequence, the lower 12 bits store the information of 'N'.  And each 6 bits stores
*the position of a 'N'.  That means our application can`t satisfy you if your data has sequences
*which contain more than two 'N'.
*********************************************/
#ifndef    SEQ_TRANS
#define    SEQ_TRANS

#include "Utility.h"
#include <queue>

#define     MAXONCE     0x100000

namespace SEQ
{
	extern	MY_VECTOR<SEQWITHN>    *groups;
	extern	MY_VECTOR<SEQWITHN>    *revgro;

	void    trans_init();
	void    trans_end();
	bool    seq_cmp(const   SEQWITHN &x, const   SEQWITHN &y);
	size_t  wtf_seqwithn(FILE* outfile, const SEQWITHN &x);   // Write SEQWITHN to file 
	size_t  rff_seqwithn(FILE* infile, SEQWITHN &x);   // Read SEQWITHN from file
	void    save_tofile(const char *nor_fname, const char *rev_fname);
	void    fmerger(FILE *firf, FILE *secf, FILE *rstf);
	void    itegrate(std::queue< const char* >  fnames, const char *final_fname);
	void    seq_trans(FILE *referf, char const *normal_name, char const * reverse_name);   // nor_name is the file name of the first-seq_index for 
																						//normal sequence, rev_name is for reverse one

};

#endif
