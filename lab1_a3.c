#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N_THREADS 4

typedef struct {
    int value;
} t_Args;

typedef struct {
    int value;
} t_Ret;

int parse_cmdl_args(int argc, char **argv);
void *increment(void *arg);


int main(int argc, char **argv) {

    // Validar argumentos de linha de comando
    int factor = parse_cmdl_args(argc, argv);

    // Alocar memória para tids
    pthread_t *pid = (pthread_t*)malloc(sizeof(pthread_t)*N_THREADS);
    if (!pid) {
        printf("erro de alocação de memória\n");
        return 3;
    }



    // Criar threads
    for (size_t i = 0; i < N_THREADS; ++i) {
        t_Args *args = (t_Args*)malloc(sizeof(t_Args));
        if (!args) {
            printf("erro de alocação de memória\n");
            return 4;
        }

        args->id = i;
        args->nthreads = N_THREADS;


        int ret = pthread_create(&tid[i], NULL, increment, (void*)args);
        if (ret) {
            printf("erro %d de criação de thread %zu\n", ret, i);
            return 5;
        }
    }

    for (int i = 0; i < N_THREADS; ++i) {
        int ret = pthread_join(tid[i], NULL);
        if (ret) {
            printf("erro %d na espera da thread %zu\n", ret, i);
            return 6;
        }
    }


    return 0;
}


int parse_cmdl_args(int argc, char **argv) {

    if (argc != 2) {
        printf("USO: %s [fator (!= 0)]\n", argv[0]);
        exit(1);
    }

    int factor = atoi(argv[1]);

    if (factor <= 0) {
        printf("USO: %s [fator (!= 0)]\n", argv[0]);
        exit(2);
    }

    return factor;
}
