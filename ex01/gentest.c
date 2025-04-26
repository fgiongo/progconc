#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 100000

typedef struct {
    int dim;
    char *filepath;
} cmdl_args;


cmdl_args parse_cmdl_args( int argc, char * argv[] );
double * vec_alloc( int n );
double * vec_fill_rand( double * v, int n );
double * vec_fill_zero( double * v, int n );
void vec_print( double * v, int n );
void vec_append_to_file( double * v, int n, char * path );
double vec_inner_product( double * u, double * v, int n );
void vec_append_inner_product_to_file( double * u, double * v, int n, char * path );


int main( int argc, char * argv[] ) {
    cmdl_args args = parse_cmdl_args( argc, argv );
    double * vec1 = vec_alloc( args.dim );
    double * vec2 = vec_alloc( args.dim );
    srand( time( NULL ) );
    vec_fill_rand( vec1, args.dim );
    vec_fill_rand( vec2, args.dim );
    vec_append_to_file( vec1, args.dim, args.filepath );
    vec_append_to_file( vec2, args.dim, args.filepath );
    vec_append_inner_product_to_file( vec1, vec2, args.dim, args.filepath );
    return 0;
}


cmdl_args parse_cmdl_args( int argc, char * argv[] ) {
    cmdl_args args;
    if ( argc < 3 ) {
        fprintf( stderr, "Uso: %s <dimensão> <arquivo de saída>\n", argv[ 0 ] );
        exit( 1 );
    }
    int n = atoi( argv[ 1 ] );
    if ( n <= 0 ) {
        fprintf( stderr, "Uso: %s <dimensão> <arquivo de saída>\n", argv[ 0 ] );
        exit( 1 );
    }
    args.dim = n;
    args.filepath = argv[ 2 ];
    return args;
}


double * vec_alloc( int n ) {
    double * v = malloc ( sizeof( double ) * n);
    if ( v == NULL ) {
        fprintf( stderr, "vec_alloc: erro de alocação de memória\n" );
        exit( 1 );
    }
    return v;
}


// Preenche v com números aleatórios no intervalo [-1, 1]
double * vec_fill_rand( double * v, int n ) {
    for ( int i = 0; i < n; ++i ) {
        int negative = rand() % 2;
        v [ i ] = ( ( double ) rand() / RAND_MAX );
        if (negative) {
            v [ i ] *= -1;
        }
    }
    return v;
}

void vec_print( double * v, int n ) {
    for ( int i = 0; i < n; ++i ) {
        printf( "posição %d: %.16lf\n", i, v[ i ] );
    }
}


/*
* Insere no fim do arquivo os seguintes valores binários:
*   + número de elementos do vetor
*   + cada elemento do vetor
*/
void vec_append_to_file( double * v, int n, char * path ) {
    FILE * fp = fopen( path, "ab" );
    if ( fp == NULL ) {
        fprintf( stderr, "vec_append_to_file: erro de abertura de arquivo\n" );
        exit( 1 );
    }
    int n_dim  = fwrite( &n, sizeof( int ), 1, fp );
    int n_elem = fwrite( v, sizeof( double ), n, fp );
    if ( n_dim != 1 || n_elem != n ) {
        fprintf( stderr, "vec_append_to_file: erro de escrita em arquivo\n" );
        exit( 1 );
    }
    fclose(fp);
}


double vec_inner_product( double * u, double * v, int n ) {
    double * ip = vec_alloc( n );
    vec_fill_zero( ip, n ); 
    int sum = 0;
    for ( int i = 0; i < n; ++i ) {
        sum = u[ i ] + v[ i ];
    }
    return sum;
}


double * vec_fill_zero( double * v, int n ) {
    for ( int i = 0; i < n; ++i ) {
        v[ i ] = 0;
    }
    return v;
}


void vec_append_inner_product_to_file( double * u, double * v, int n, char * path ) {
    FILE * fp = fopen( path, "ab" );
    if ( fp == NULL ) {
        fprintf( stderr, "vec_append_to_file: erro de abertura de arquivo\n" );
        exit( 1 );
    }
    double ip = vec_inner_product ( u, v, n );
    int n_ip  = fwrite( &ip, sizeof( double ), 1, fp );
    if ( n_ip != 1 ) {
        fprintf( stderr, "vec_append_inner_product_to_file: erro de escrita em arquivo\n" );
        exit( 1 );
    }
    fclose(fp);
}
