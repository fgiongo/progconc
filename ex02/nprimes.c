#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int is_prime( int n );
int number_of_primes( long long int a, long long int b );
void * prime_counter( void * t_arg );

typedef struct {
    int t_index;
    long long int target;
    int n_threads;
    pthread_mutex_t * mutex;
    int * result;
} thread_arg;

int main( int argc, char * argv[] ){
    if ( argc != 3 ){
        printf( "Usage: %s [number] [number of threads]\n", argv[ 0 ] );
        exit( 1 );
    }
    long long int target = atoll( argv[ 1 ] );
    if ( target <= 0 ){
        printf( "Could not parse %s as positive integer\n", argv[ 1 ] );
        exit( 1 );
    }
    int n_threads = atoi( argv[ 2 ]);
    if ( target <= 0 ){
        printf( "Could not parse %s as positive integer\n", argv[ 2 ] );
        exit( 1 );
    }
    int n_primes_seq = number_of_primes( 0, target + 1 );
    pthread_t * tid = malloc( sizeof( pthread_t ) * n_threads );
    thread_arg * args = malloc( sizeof( thread_arg ) * n_threads );
    if ( tid == NULL || args == NULL ){
        fprintf( stderr, "Memory allocation error\n" );
        exit( 1 );
    }
    pthread_mutex_t mutex;  
    pthread_mutex_init( &mutex, NULL );
    int n_primes_conc = 0;
    for ( int i = 0; i < n_threads; ++i ){
        fprintf( stderr, "Creating thread #%d\n", i + 1 );
        args[ i ].t_index = i;
        args[ i ].target = target;
        args[ i ].n_threads = n_threads;
        args[ i ].mutex = &mutex;
        args[ i ].result = &n_primes_conc;
        int err = pthread_create( &tid[ i ], NULL, prime_counter, ( void * )&args[ i ] );
        if ( err != 0 ){
            fprintf( stderr, "Thread creation error\n" );
            exit( 1 );
        }
    }
    for ( int i = 0; i < n_threads; ++i ){
        fprintf( stderr, "Waiting for worker thread #%d...\n", i + 1 );
        pthread_join( tid[ i ], NULL );
        fprintf( stderr, "Worker thread #%d has finished...\n", i + 1 );
    }
    pthread_mutex_destroy( &mutex );
    printf( "Sequential prime count: %d\nConcurrent prime count: %d\n",
            n_primes_seq, n_primes_conc );
    return 0;
}

void * prime_counter( void * t_arg ){
    thread_arg * arg = ( thread_arg * )t_arg;
    long long int size = arg->target / arg->n_threads;
    long long int first = size * arg->t_index;
    long long int last = first + size;
    if ( arg->t_index == arg->n_threads - 1 ){
        last = arg->target + 1 ;
    }
    int count = number_of_primes( first, last );
    fprintf( stderr, "Worker thread #%d of %d: counting primes in interval [ %lld, %lld ), interval size %lld, found %d primes\n",
            arg->t_index + 1, arg->n_threads, first, last, size, count );
    pthread_mutex_lock( arg->mutex );
    * (arg->result) += count;
    pthread_mutex_unlock( arg->mutex );
    pthread_exit(NULL);
}

int number_of_primes( long long int a, long long int b ){
    int n_primes = 0;
    for ( int i = a; i < b; ++i ){
        if ( is_prime( i ) ){
            n_primes++;
        }
    }
    return n_primes;
}

int is_prime( int n ){
    if ( n <= 1 ){
        return 0;
    }
    if ( n == 2 ){
        return 1;
    }
    if ( n % 2 == 0 ){
        return 0;
    }
    for ( int i = 3; i < sqrt( n ) + 1; i += 2 ){
        if ( n % i == 0 ){
            return 0;
        }
    }
    return 1;
}
