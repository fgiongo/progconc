/* Disciplina: Programação Concorrente */
/* Prof.: Silvana Rossetto */
/* Produtor/consumidor usando semaforo binario e contador */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>

#define PRODUTORES 1
#define MAX (PRODUTORES+CONSUMIDORES)
#define N 5 //numero de elementos no buffer

// Variaveis globais
sem_t slotCheio, slotVazio;  // semaforos para sincronizacao por condicao
sem_t mutexGeral; //semaforo geral de sincronizacao por exclusao mutua
sem_t mutexContagens;

int *Buffer; //buffer compartilhado
int *Contagens;
int nInteiros;
int nThreads;

//imprime o buffer
void printBuffer(int buffer[], int tam) {
    for(int i=0;i<tam;i++) 
        printf("%d ", buffer[i]); 
    puts("");
}

//funcao para inserir um elemento no buffer
void Insere (int item, int id) {
    static int in=0;
    sem_wait(&slotVazio); //aguarda slot vazio para inserir
    sem_wait(&mutexGeral); //exclusao mutua entre produtores
    Buffer[in] = item; 
    in = (in + 1) % N;
    printf("Prod[%d]: inseriu %d\n", id, item);
    printBuffer(Buffer, N); //para log apenas
    sem_post(&mutexGeral);
    sem_post(&slotCheio); //sinaliza um slot cheio
}

//funcao para retirar  um elemento no buffer
int Retira (int id) {
    int item;
    static int out=0;
    sem_wait(&slotCheio); //aguarda slot cheio para retirar
    sem_wait(&mutexGeral); //exclusao mutua entre consumidores
    item = Buffer[out];
    Buffer[out] = 0;
    out = (out + 1) % N;
    printf("Cons[%d]: retirou %d\n", id, item);
    printBuffer(Buffer, N); //para log apenas
    sem_post(&mutexGeral);
    sem_post(&slotVazio); //sinaliza um slot vazio
    return item;
}

//----------------------------------------------------------
// Produtor 
void *produtor(void * arg) {
    static int count = 1;
    int id = *(int *)(arg); 
    free(arg);
    while(count <= nInteiros) {
        //produz alguma coisa....
        Insere(count, id); //insere o proximo item
        count++;
    }
    for (int i = 0; i < nThreads; ++i) {
        Insere(EOF, id);
    }
    pthread_exit(NULL);
}

int ehPrimo(int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 ==  0) return  0;
    for (int i = 3; i < sqrt(n) + 1; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

//----------------------------------------------------------
// Consumidor
void *consumidor(void * arg) {
    int item, id = *(int *)(arg);
    free(arg);
    while(1) {
        item = Retira(id); //retira o proximo item
        if (item == EOF) {
            break;
        }

        //consome o item....
        if (ehPrimo(item)) {
            Contagens[id - 1]++;
        }
    }
    pthread_exit(NULL);
}

//--------------------------------------------------------------
// Funcao principal
int main(int argc, char **argv) {
    int i, *id;


    // Parse cmdl args
    if (argc != 4) {
        printf("Uso: %s <N> <M> <C>\n", argv[0]);
        exit(1);
    }
    nInteiros = atoi(argv[1]);
    int bufferSize = atoi(argv[2]);
    int CONSUMIDORES = atoi(argv[3]);
    nThreads = CONSUMIDORES + 1;

    if (nInteiros <= 0 || bufferSize <= 0 || CONSUMIDORES <= 0) {
        printf("Uso: %s <N> <M> <C> com N, M, C > 0\n", argv[0]);
        exit(1);
    }

    Buffer = malloc(sizeof(int) * bufferSize);
    Contagens = malloc(sizeof(int) * nThreads);
    for (int i = 0; i < nThreads; ++i) {
        Contagens[i] = 0;
    }
    pthread_t *tid = malloc(sizeof(pthread_t) * nThreads);

    // Inicia os semaforos
    sem_init(&mutexGeral, 0, 1);
    sem_init(&slotCheio, 0, 0);
    sem_init(&slotVazio, 0, N);

    // Inicia os threads produtores
    for(i=0;i<PRODUTORES;i++) {
        id = (int *) malloc(sizeof(int));
        *id = i+1;
        if (pthread_create(&tid[i], NULL, produtor, (void *) (id))) {
            printf("Erro na criacao do thread produtor\n");
            exit(1);
        }
    }
    // Inicia os threads consumidores
    for(i=0;i<CONSUMIDORES;i++) {
        id = (int *) malloc(sizeof(int));
        *id = i+1;
        if (pthread_create(&tid[PRODUTORES+i], NULL, consumidor, (void *) (id))) {
            printf("Erro na criacao do thread produtor\n");
            exit(1);
        }
    }

    for (i = 0; i < nThreads; ++i) {
        void *ret;
        pthread_join(tid[i], &ret);
    }
    printf("Primos encontrados por cada thread:\n");
    int max = 0;
    int soma = 0;
    int ganhador;
    for (i = 0; i < nThreads - 1; ++i) {
        soma += Contagens[i];
        if (Contagens[i] > max) {
            max = Contagens[i];
            ganhador = i + 1;
        }
        printf("%d, ", Contagens[i]);
    }
    printf("%d\n", Contagens[i]);
    printf("Soma: %d\n", soma);
    printf("Ganhadora: thread %d\n", ganhador);
    pthread_exit(NULL);
}        
