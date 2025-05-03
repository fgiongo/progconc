#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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


typedef struct {
    int t_index;
    int n_threads;
    int v_dim;
    double * v1;
    double * v2;
} thread_arg;


cmdl_args parse_cmdl_args( int argc, char * argv[] );
file_data read_file( char * path );
int int_read ( FILE * fp );
double * vec_read( int n, FILE * fp );
double double_read( FILE * fp );
void vec_print( double * v, int n );
double vec_inner_product( double * u, double * v, int n );
double var_relativa( double c, double s );
void * inner_product_worker( void * a );


int main( int argc, char * argv[] ) {
    cmdl_args args = parse_cmdl_args( argc, argv );
    file_data input = read_file( args.filepath );
    if ( args.n_threads > input.dim ) {
        args.n_threads = input.dim;
    }
    pthread_t * tid   = malloc( sizeof( pthread_t ) * args.n_threads );
    thread_arg * t_arg = malloc( sizeof( thread_arg ) * args.n_threads );
    if ( tid == NULL || t_arg == NULL ) {
        fprintf( stderr, "Memory allocation error, exiting\n" );
        exit( 1 );
    }
    for ( int i = 0; i < args.n_threads; ++i ) {
        fprintf( stderr, "Creating worker thread #%d...\n", i + 1 );
        t_arg[ i ].t_index = i;
        t_arg[ i ].n_threads = args.n_threads;
        t_arg[ i ].v_dim = input.dim;
        t_arg[ i ].v1 = input.v1;
        t_arg[ i ].v2 = input.v2;
        int err = pthread_create( &tid[ i ], NULL, inner_product_worker, ( void * ) &t_arg[ i ] );
        if ( err != 0 ) {
            fprintf( stderr, "Thread creation error, exiting\n" );
            exit( 1 );
        }
    }
    double result = 0;
    for ( int i = 0; i < args.n_threads; ++i ) {
        fprintf( stderr, "Waiting for worker thread #%d...\n", i + 1 );
        double * retval;
        int err = pthread_join( tid[ i ], ( void ** )&retval );
        if ( err != 0 ) {
            fprintf( stderr, "Thread termination error\n" );
            exit( 1 );
        }
        double partial = * retval;
        fprintf( stderr, "Worker #%d has returned %.16lf\n", i + 1, partial );
        result += partial;
    }
    fprintf( stderr, "Final result: %.16lf, expected: %.16lf\n", result, input.inner_product );
    fprintf( stderr, "Var. Relativa: %.16lf\n", var_relativa( result, input.inner_product ) );
    return 0;
}


void * inner_product_worker( void * a ) {
    thread_arg * arg = ( thread_arg * )a;
    double * result = malloc( sizeof( double ) );
    if ( result == NULL ) {
        fprintf( stderr, "Worker #%d: could not allocate memory for result\n", arg->t_index);
    }
    int size = arg->v_dim / arg->n_threads;
    int startpos  = size * arg->t_index;
    int endpos = startpos + size;
    if ( arg->t_index == arg->n_threads - 1 ) {
        endpos = arg->v_dim;
    }
    fprintf( stderr, "Worker #%d of %d: working on vector of size %d, slice size: %d, start pos: %d, end pos: %d\n",
            arg->t_index + 1, arg->n_threads, arg->v_dim, size, startpos, endpos);
    for ( int i = startpos; i < endpos; ++i ) {
        * result += arg->v1[ i ] * arg->v2[ i ];
    }
    pthread_exit( ( void * )result );
}


double var_relativa( double c, double s ) {
    return fabs( ( s - c ) / s );
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
        fclose( fp );
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


