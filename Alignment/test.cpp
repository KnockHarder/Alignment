/*
    file used to test someidea
*/

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// define
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

const char      seq_map[4] = { 'A', 'C', 'T', 'G' };

typedef     struct{
	uint64_t    seq;
	uint32_t    flag;
}   SEQWITHN;

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

uint64_t    transSeq( const char *seq)
{/*{{{*/
	// declear variables
	uint64_t     data;/*{{{*/
	uint64_t     inc;
    uint32_t     flag;
	char         ch;
	uint32_t     count;
	uint32_t     unknown;    //  how many bases 'n/N' a sequence has 

	int     k = 0;/*}}}*/

    // get the number of unknown bases and the length
    unknown = 0;/*{{{*/
    for (count = 0; (ch = seq[count]) != '\0' && ch != '\n' && ch != ' '; count++)
    {
        if (ch == 'N' || ch == 'n')
            unknown++;
    }/*}}}*/

    // check validation
    if (count < MINNT || count > MAXNT)/*{{{*/
    {
        printf(" The length of \"%s\" is out of range \n", seq );
        exit( -1 );
    }

    if (unknown > MAXUNKNOWN)
    {
        printf("Error: unknown bases is more than %d in %s \n", MAXUNKNOWN, seq);
        exit( -1 );
    }/*}}}*/

    // get the positions of unknown bases
    k = 0;/*{{{*/
    flag = 0;
    for (int i = 0; i < count; i++)
        if (seq[i] == 'N' || seq[i] == 'n')
        {
            flag += 1 << i;
            k++;
        }/*}}}*/

    // check consitency
    if (k != unknown)/*{{{*/
    {
        printf(" The number of unknown base 'N' is confusing \n ");
        exit(-1);
    }/*}}}*/

    // convert the unknown bases to konwn and get the final data
    for (int i = 0; i < (1 << (unknown * 2)); i++)
    {/*{{{*/
        // tranfrom to binary format
        data = k = 0;/*{{{*/
        for (int j = 0; j < count; j++)
        {
            ch = seq[j];
            if (ch == 'N' || ch == 'n')  // for unknown bases, from low to high bits, tow bits by tow bits of 'i' are used as the value of 'inc'.
            {
                inc = i >> (k * 2);
                inc = inc % 4;
                k++;
            }
            else
                inc = (ch >> 1) & 3;

            data = data << 2;
            data += inc;
        }/*}}}*/

        // add length information
        data = data << (64 - count * 2);/*{{{*/
        data += unknown << SIGNBITS;
        data += MAXNT - count;/*}}}*/

    }/*}}}*/

    return data;
}/*}}}*/

char*   recoverSeq(const  SEQWITHN  &seq)
{/*{{{*/
	char*   seqstr = (char*)malloc(sizeof(char) * (MAXNT + 1));
	char*   temp;
	int     length = MAXNT - (seq.seq & SIGNMASK);

	int     unknown = (seq.seq >> SIGNBITS) & SIGNMASK;

	temp = toString(seq.seq);
//  strncpy_s(seqstr, MAXNT + 1, temp, length);
    strncpy(seqstr, temp, length);
	seqstr[length] = '\0';

	int     k = 0;
	if (seq.flag != 0)
	{
		if (seq.flag >= (1 << length))
		{
			printf("Error: Sequence %lu is not consistent with Flag %d \n", seq.seq, seq.flag);
			exit(-1);
		}
		for (int i = 0; i < length; i++)
			if (((seq.flag >> i) & 1) == 1)
			{
				seqstr[i] = 'N';
				k++;
			}
	}

	if (k != unknown)
	{
		printf("Error: Sequence %lu is not consistent with Flag %d \n", seq.seq, seq.flag);
		exit(-1);
	}

	return seqstr;
}/*}}}*/

int    main( int argc, char** argv)
{/*{{{*/
    printf( "%d\n", 020 );

    if( argc != 3 )
    {
        printf( "Usage: cmd binfile rstfile\n" );
        exit(0);
    }

	printf(" test start \n");

	//  declear variables
    char        buff[102];/*{{{*/
	FILE        *seqf, *resultf;
    size_t      errorcode;
    seqf = resultf = 0;

	SEQWITHN    seq_data;
    char*       seq;/*}}}*/


	// open files
	if ( ( resultf = fopen( argv[2], "wt") )  ==  0 )/*{{{*/
	{
		printf("Error: Can`t open the file '%s'\n", argv[2] );
		exit(-1);
	}

    if ( ( seqf = fopen( argv[1], "rb") )  ==  0 )
    {
        printf("Open file \'%s\' failed\n", argv[1] );
        exit(-1);
    }/*}}}*/

    //  get the sequence and search it with the index one by one
    while (!feof(seqf))
    {/*{{{*/

        //  read a sequence from file
        if ((errorcode = fread(&(seq_data.seq), sizeof(uint64_t), 1, seqf)) != 1)
        {/*{{{*/
            if (errorcode == 0 && feof(seqf))
                continue;
            
            printf( "Read File Error: Can`t get sequence from file\n" );
            exit( -1 );
        }/*}}}*/

        // get the flag
        if (((seq_data.seq >> SIGNBITS) & SIGNMASK) != 0)    // the sequence has unknown bases
        {/*{{{*/
            if ((errorcode = fread(&(seq_data.flag), sizeof(uint32_t), 1, seqf)) != 1)
            {
                printf( "File Read Error: Can`t get flag from file\n" );
                exit( -1 );
            }
        }
        else
            seq_data.flag = 0;/*}}}*/

        // store seqence 
        seq = recoverSeq( seq_data ); /*{{{*/
        
        sprintf( buff, "%lx", seq_data.seq );
        fputs( buff, resultf );
        fputs( "\n", resultf );
        fputs( seq, resultf );
        fputs( "\n", resultf );/*}}}*/

    }/*}}}*/

    // test recoverone
    fputs( "\nhexadecimal \n", resultf );
    sprintf( buff, "%lx", transSeq( "AAACTGGAACCCTGATACATTACTG" ) );
    fputs( buff, resultf );
    
    fclose( seqf );
    fclose( resultf );
    
	printf(" test end \n");
    return 0;
}/*}}}*/
