#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define MAX_THREADS (128)

typedef struct {
    int n_threads;
    char *filepath;
} cmdl_args;


typedef struct {
    int dim;
    double * v1;
    double * v2;
    double inner_product;
} file_data;


cmdl_args parse_cmdl_args( int argc, char * argv[] );
file_data read_file( char * path );
int int_read ( FILE * fp );
double * vec_read( int n, FILE * fp );
double double_read( FILE * fp );
void vec_print( double * v, int n );
double vec_inner_product( double * u, double * v, int n );


int main( int argc, char * argv[] ) {
    cmdl_args args = parse_cmdl_args( argc, argv );
    file_data input = read_file( args.filepath );
    if ( args.n_threads > input.dim ) {
        args.n_threads = input.dim;
    }
    pthread_t * tid   = malloc( sizeof( pthread_t ) * args.n_threads );
    thread_arg * t_arg = malloc( sizeof( thread_arg ) * args.n_threads );
    if ( tid == NULL || t_arg == NULL ) {
        fprintf( stderr, "create_threads: memory allocation error\n" );
        exit( 1 );
    }
    for ( int i = 0; i < n_threads; ++i ) {
        t_arg[ i ].index = i;
        t_arg[ i ].v1    = v1;
        t_arg[ i ].v2    = v2;
        t_arg[ i ].dim   = dim;
        int err = pthread_create( &tid[i], NULL, inner_product_worker, ( void * ) &t_arg[ i ] );
        if ( err != 0 ) {
            fprintf( stderr, "thread creation error\n" );
            exit( 1 );
        }
    }
    return 0;
}


cmdl_args parse_cmdl_args( int argc, char * argv[] ) {
    cmdl_args args;
    if ( argc < 3 ) {
        fprintf( stderr, "Uso: %s <número de threads> <arquivo de entrada>\n", argv[ 0 ] );
        exit( 1 );
    }
    int n = atoi( argv[ 1 ] );
    if ( n <= 0 ) {
        fprintf( stderr, "Número de threads deve ser maior do que 0\n" );
        exit( 1 );
    }
    if ( n > MAX_THREADS ) {
        n = MAX_THREADS;
    }
    args.n_threads = n;
    args.filepath = argv[ 2 ];
    return args;
}


file_data read_file( char * path ) {
    file_data data;
    FILE * fp = fopen( path, "rb" );
    if ( fp == NULL ) {
        fprintf( stderr, "read_file: erro de abertura de arquivo\n" );
        exit( 1 );
    }
    data.dim           = int_read( fp );
    data.v1            = vec_read( data.dim, fp );
    int n              = int_read( fp );
    data.v2            = vec_read( data.dim, fp );
    data.inner_product = double_read( fp );
    if ( n != data.dim ) {
        fprintf( stderr, "read_file: vetores de tamanhos diferentes\n" );
        exit( 1 );
    }
    fclose(fp);
    return data;
}


int int_read ( FILE * fp ) {
    int n;
    int elem_read = fread(&n, sizeof( int ), 1, fp);
    if ( elem_read != 1 ) {
        fprintf( stderr, "int_read: erro de leitura de arquivo\n" );
        fclose(fp);
        exit( 1 );
    }
    return n;
}


double * vec_read( int n, FILE * fp ) {
    double * vec = malloc( sizeof( double ) * n );
    int elem_read = fread(vec, sizeof( double ), n, fp);
    if ( elem_read != n ) {
        fprintf( stderr, "vec_read: erro de leitura de arquivo\n" );
        fclose(fp);
        exit( 1 );
    }
    return vec;
}


double double_read( FILE * fp ) {
    double f;
    int elem_read = fread(&f, sizeof( double ), 1, fp);
    if ( elem_read != 1 ) {
        fprintf( stderr, "double_read: erro de leitura de arquivo\n" );
        fclose(fp);
        exit( 1 );
    }
    return f;
}


void vec_print( double * v, int n ) {
    for ( int i = 0; i < n; ++i ) {
        printf( "posição %d: %.16lf\n", i, v[ i ] );
    }
}


double vec_inner_product( double * u, double * v, int n ) {
    double sum = 0;
    for ( int i = 0; i < n; ++i ) {
        sum += u[ i ] * v[ i ];
    }
    return sum;
}


