#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define N_SAMPLES (100)
#define TG_SIZE (2)
#define TC_SIZE (3)

typedef struct {
    int t_index;
    long long int target;
    int n_threads;
    pthread_mutex_t * mutex;
    int * result;
} thread_arg;

typedef struct {
    int n_primes;
    double time;
} sample;

int is_prime( int n );
int number_of_primes( long long int a, long long int b );
void * prime_counter( void * t_arg );
sample new_sample( long long int target, int n_threads );
sample benchmark( long long int target, int n_threads );

int main( void ){
    const long long int targets[ TG_SIZE ] = { 1e3, 1e6 };
    const int thread_counts[ TC_SIZE ] = { 1, 2, 4 };
    sample results[ TC_SIZE ];
    for ( int tg_i = 0; tg_i < TG_SIZE; ++tg_i ){
        char time_filename[ 32 ];
        char speedup_filename[ 32 ];
        char efficiency_filename[ 32 ];
        sprintf( time_filename, "time-%lld.csv", targets[ tg_i ] );
        sprintf( speedup_filename, "speedup-%lld.csv", targets[ tg_i ] );
        sprintf( efficiency_filename, "efficiency-%lld.csv", targets[ tg_i ] );
        FILE * time_fp = fopen( time_filename, "w" );
        FILE * speedup_fp = fopen( speedup_filename, "w" );
        FILE * efficiency_fp = fopen( efficiency_filename, "w" );
        if ( time_fp == NULL || speedup_fp == NULL ||
                efficiency_fp == NULL ){
            fprintf( stderr, "Error opening file\n" );
            exit( 1 );
        }
        for ( int tc_i = 0; tc_i < TC_SIZE; ++tc_i ){
            results[ tc_i ] = benchmark( targets[ tg_i ], thread_counts[ tc_i ] );
            double speedup = results[ 0 ].time / results[ tc_i ].time;
            double efficiency = speedup / thread_counts[ tc_i ];
            fprintf( time_fp, "%lf, %lf\n",
                    ( double )thread_counts[ tc_i ], results[ tc_i ].time );
            fprintf( speedup_fp, "%lf, %lf\n",
                    ( double )thread_counts[ tc_i ], speedup );
            fprintf( efficiency_fp, "%lf, %lf\n",
                    ( double )thread_counts[ tc_i ], efficiency );
        }
        fclose( time_fp );
        fclose( speedup_fp );
        fclose( efficiency_fp );
    }
    return 0;
}

sample benchmark( long long int target, int n_threads ){
    sample s;
    double time = 0;
    for ( int i = 0; i < N_SAMPLES; ++i ){
        s = new_sample( target, n_threads );
        time += s.time;
    }
    double avg = time / N_SAMPLES;
    return ( sample ){ s.n_primes, avg };
}

sample new_sample( long long int target, int n_threads ){
    struct timespec begin, end;
    clock_gettime( CLOCK_MONOTONIC, &begin );
    pthread_t * tid = malloc( sizeof( pthread_t ) * n_threads );
    thread_arg * args = malloc( sizeof( thread_arg ) * n_threads );
    if ( tid == NULL || args == NULL ){
        fprintf( stderr, "Memory allocation error\n" );
        exit( 1 );
    }
    pthread_mutex_t mutex;
    pthread_mutex_init( &mutex, NULL );
    int n_primes = 0;
    for ( int i = 0; i < n_threads; ++i ){
        args[ i ].t_index = i;
        args[ i ].target = target;
        args[ i ].n_threads = n_threads;
        args[ i ].mutex = &mutex;
        args[ i ].result = &n_primes;
        int err = pthread_create( &tid[ i ], NULL, prime_counter, ( void * )&args[ i ] );
        if ( err != 0 ){
            fprintf( stderr, "Thread creation error\n" );
            exit( 1 );
        }
    }
    for ( int i = 0; i < n_threads; ++i ){
        pthread_join( tid[ i ], NULL );
    }
    pthread_mutex_destroy( &mutex );
    free( tid );
    free( args );
    clock_gettime( CLOCK_MONOTONIC, &end );
    double time = ( end.tv_sec - begin.tv_sec ) * 1000.0 +
        ( end.tv_nsec - begin.tv_nsec ) / 1e6;
    return ( sample ){ n_primes, time };
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
