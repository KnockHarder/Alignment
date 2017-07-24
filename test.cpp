/*
    file used to test someidea
*/

#include <stdio.h>
#include <stdlib.h>

const char** astrs = 0;
const char** bstrs = 0;

int main( int argc, char** argv )
{/*{{{*/
    astrs = ( const char** ) malloc( sizeof( char* ) * 2 );
    bstrs = ( const char** ) malloc( sizeof( char* ) * 2 );

    astrs[0] = "a first";
    astrs[1] = "a second";
    
    bstrs[0] = "b first";
    bstrs[1] = "b second";
    
    // allowed
    astrs[0] = "a changed";

    // not allowed
    // astrs[0][0] = 'c';

}/*}}}*/
