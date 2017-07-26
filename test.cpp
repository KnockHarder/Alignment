/*
    file used to test someidea
*/

#include "firIndex.h"
#include "Utility.h"

typedef    struct
{
    RAW_DATA    *data;
    int        count;
    int        size;
}    RAW_BUFF;

const char*       mids[4] = { "mid0", "mid1", "mid2", "mid3" };
const char*       rsts[4] = { "rst0", "rst1", "rst2", "rst3" };
RAW_BUFF    rbuff = { 0, 0, 0 };

void    recovery( FILE *midfp, FILE *rstfp, int flag );
const char*    getSeq(const uint64_t   &data, int flag);
char*   toString(uint64_t  data);

int main( int argc, char** argv )
{/*{{{*/
    // check arguments count
    /*{{{*/
    if( argc != 2 )
    {
        printf( "Usage: cmd referfile \n");
        exit( -1 );
    }
    /*}}}*/

    // declear variables
    FILE    *refer = 0;
    FILE    *midfp = 0, *rstfp = 0;

    // check refer file
    /*{{{*/
    if( ( refer = fopen( argv[1], "rb" ) )  ==  0 )
    {
        printf( "File Read Error: Can`t open file - %s\n", argv[1] );
        exit( -1 );
    }
    /*}}}*/

    // recall function to get mid-files
    FIR::set_fir( refer, mids );

    // recover the sequences
    for( int i = 0; i < 4; i ++ )
    {
        midfp = fopen( mids[i], "rb" );
        rstfp = fopen( rsts[i], "wt" );
        
        recovery( midfp, rstfp, i );
    
        fclose( midfp );
        fclose( rstfp );
        midfp = rstfp = NULL;
    }
}/*}}}*/

void    recovery( FILE *midfp, FILE *rstfp, int flag )
{/*{{{*/
    // declear variables
    RECORD*     fir_index = (RECORD *)malloc(sizeof(RECORD) * DEGREE);
    uint64_t    data;
    const char* seq;

	// get first index from the file
    if (fread(fir_index, sizeof(RECORD), DEGREE, midfp) != DEGREE)
    {
        printf("File Error:The first-index file is ont ready! -- %d\n", flag);
        exit(-1);
    }

    // read data and recover
    /*{{{*/
    for (uint64_t i = 0; i < DEGREE; i++)
    {
        //  get the number of the sequence with the same first index number
        rbuff.count = fir_index[i].num - fir_index[i].start;

        //  read these sequences into the rebuff
        /*{{{*/
        if (rbuff.count > 0)
        {
            if (rbuff.size < rbuff.count)
            {
                free(rbuff.data);
                (rbuff.data) = (RAW_DATA*)malloc(sizeof(RAW_DATA) * rbuff.count);
                rbuff.size = rbuff.count;
            }

            if (fread((rbuff.data), sizeof(RAW_DATA), rbuff.count, midfp) != rbuff.count)
            {
                printf("File Error:The first-index file is ont applicable! -- %d/%lu\n", flag, i);
                exit(-1);
            }
        }
        /*}}}*/

        // recover data and save to file
        /*{{{*/
        for( int j = 0; j < rbuff.count; j ++ )
        {
            data = i << 48;
            data += rbuff.data[j];
            fputs( getSeq( data, flag ), rstfp );
            fputs( "\n", rstfp );
        }
        /*}}}*/
    }
    /*}}}*/
}/*}}}*/

const char*    getSeq(const uint64_t   &data, int flag)
{/*{{{*/

    // declears variables and init
    /*{{{*/
    char*   refstr = (char*)malloc(sizeof(char) * (MAXNT + 1));
    char*   temp;
    int     length;

    int     pad = (data >> SIGNBITS) & SIGNMASK;
    int     reserv = data & SIGNMASK;
    /*}}}*/

    // get the hist sequence
    /*{{{*/

    // recovery for complementary sequence
    /*{{{*/
    if( flag / 2 == 1 )
    {
        temp = toString( data ^ COMPMASK );
    }
    else
        temp = toString(data);
    /*}}}*/

    // get the hit sequence and get it`s length
    /*{{{*/
    if (pad < 16) // padded or normal
    {
        length = MAXNT - pad - reserv;
    //  strncpy_s(refstr, (MAXNT * 2 + 10), temp + pad, length);
        strncpy(refstr, temp + pad, length);
        refstr[length] = '\0';
    }
    else    // shrinked
    {
        pad = pad - 16;
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

    return refstr;
}/*}}}*/

char*   toString(uint64_t  data)
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

