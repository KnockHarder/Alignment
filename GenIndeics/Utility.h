/*************************************
*The coding for the char is A-00,T/U-10,C-01,G-11, case insensitive.
***************************************/
#pragma	once

#ifndef    MY_VECTOR_CLASS
#define    MY_VECTOR_CLASS


#define    DEGREE   65536
#define    MASK48   281474976710655
#define    MASK32   4294967295
#define    MINNT    16      // the vaule of MAXNT minus MINNT can`t be larger than or equal to 16
#define    MAXNT    27      // can`t be changed an larger number
#define    SIGNBITS 5
#define    SIGNMASK     0x1f
#define    BLOCKSIZE    1024
#define    STRLEN       151
#define    MAXUNKNOWN   4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <algorithm>

#define     RAW_DATA     uint64_t

typedef    struct{
	int    start;
	int    num;
}     RECORD;

typedef     struct{
	uint64_t    seq;
	uint32_t    flag;
}   SEQWITHN;

template    <class T>
class    MY_VECTOR
{/*{{{*/
	T    *arry;
	int    count, size;

public:
	MY_VECTOR();

	int     getlength(){ return count; }

	void    push_back(const T &item);

	T*    get(int k);

	void    clear(){ count = 0; }

	int		find(const T &x, int begin, int cmp(const T&, const T&));
	
	size_t	outputf(FILE    *outfile, size_t outfunc(FILE *outfile, const T &x) = NULL);

	void	sort(bool func(const T&, const T&));

	~MY_VECTOR();
};/*}}}*/

extern const char      seq_map[4];

extern const char*		IndicesPath;
extern const char*		TempFilePath;



template <class T>
MY_VECTOR<T>::MY_VECTOR()
{/*{{{*/
	arry = (T *)malloc(sizeof(T) * 16);
	count = 0;
	size = 16;
}/*}}}*/

template <class T>
void    MY_VECTOR<T>::push_back(const T &item)
{/*{{{*/
	if (!(count<size))
	{
		if (size >= (1 << 30))
		{
			printf(" boom !  too much data in VECTOR \n");
			exit(-1);
		}
		T *temp = (T *)malloc(sizeof(T)*size * 2);
		memcpy(temp, arry, sizeof(T)*size);
		free(arry);
		arry = temp;
		size *= 2;
	}
	arry[count] = item;
	count++;
}/*}}}*/

template <class T>
T*    MY_VECTOR<T>::get(int k)
{/*{{{*/
	if (k < 0 || k >= count)
		return NULL;
	else
		return &(arry[k]);
}/*}}}*/

template <class T>
int   MY_VECTOR<T>::find(const T &x, int begin, int cmp(const T&, const T&))
{/*{{{*/
	// required the data has be sorted

	int    mid, end = count - 1;
	if (begin > end)
	{
		return -1;
	}

	while (end > begin)
	{
		mid = (begin + end) / 2;
		if (cmp(arry[mid], x) < 0)
			begin = mid + 1;
		else
			end = mid;
	}
	if (cmp(arry[begin], x) == 0)
		return begin;
	else
		return -1;
}/*}}}*/

template <class T>
size_t    MY_VECTOR<T>::outputf(FILE    *outfile, size_t outfunc(FILE *outfile, const T &x))
{/*{{{*/
	size_t     forreturn = 0;

	if (!outfile)
	{
		printf("VECTOR outputf Error:Can`t find the output file!");
		return    -1;
	}
	if (outfunc == NULL)
		return   fwrite(arry, sizeof(T), count, outfile);
	else
	{
		for (int i = 0; i < count; i++)
			forreturn += outfunc(outfile, arry[i]);
		return  forreturn;
	}
}/*}}}*/

template <class T>
void    MY_VECTOR<T>::sort(bool func(const T&, const T&))
{/*{{{*/
	if (func == NULL)
		printf(" sort func null \n");
	std::sort(arry, arry + count, func);
}/*}}}*/

template <class T>
MY_VECTOR<T>::~MY_VECTOR()
{/*{{{*/
	free(arry);
}/*}}}*/


#endif
