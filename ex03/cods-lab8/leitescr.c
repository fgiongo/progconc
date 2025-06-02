/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: leitores e escritores usando semáforos */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define L 3
#define E 3

#define NTHREADS L+E

// Variaveis globais
sem_t leitura, escrita; //semaforos para sincronizar a ordem de execucao das threads
int num_leitores=0; //numero de leitores lendo

float c, f=23.7; //variaveis auxiliares

//funcao executada pelos leitores
void *Leitor (void *threadid) {
  int tid =  * (int*) threadid, i;
  while(1) {
     fprintf(stderr, "Leitor: %d quer ler!\n", tid);
     //inicia a leitura
     sem_wait(&leitura); //exclusao mutua entre leitores
     if(num_leitores == 0) { //nao ha leitores lendo, verifica se pode ler...
        fprintf(stderr, "Leitor: %d verifica escrita!\n",  tid);
        sem_wait(&escrita); //bloqueia caso algum escritor esteja escrevendo
     }
     num_leitores++;  //registra que ha mais um leitor lendo
     sem_post(&leitura); //libera a exclusao mutua entre leitores

     //executa a leitura...
     fprintf(stderr, "Leitor: %d esta lendo!\n", tid);
     for(i=1;i<1000000; i++) c=7.5*c - f;

     //termina a leitura
     sem_wait(&leitura); 
     num_leitores--; //registra que nao esta mais lendo 
     if(num_leitores==0) {
        sem_post(&escrita); //o ultimo leitor a sair, libera a escrita
     }
     fprintf(stderr, "Leitor: %d terminou de ler!\n",  tid);
     sem_post(&leitura);
     sleep(1);
  }
  pthread_exit(NULL);
}

//funcao executada pelos escritores
void *Escritor (void *threadid) {
  int tid = *(int*) threadid, i;
  while(1) {
     fprintf(stderr, "Escritor: %d quer escrever!\n", tid);
     //inicia a escrita
     sem_wait(&escrita);

     //SC
     fprintf(stderr, "Escritor: %d esta escrevendo!\n", tid);
     for(i=1;i<1000000; i++) c=c*7.5 -f;
     fprintf(stderr, "Escritor: %d terminou de escrever!\n", tid);
   
     //termina a escrita
     sem_post(&escrita);
     sleep(1);
  }
  pthread_exit(NULL);
}

//funcao principal
int main(int argc, char *argv[]) {
  pthread_t tid[L+E];
  int t, id[L+E];
  
  //inicia os semaforos
  sem_init(&leitura, 0, 1);
  sem_init(&escrita, 0, 1);

  //cria as threads
  for (t=0; t<E; t++) {
     id[t]=t;
     if (pthread_create(&tid[t], NULL, Escritor, (void *)&id[t])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  }
  for (t=L; t<NTHREADS; t++) {
     id[t]=t-L;
     if (pthread_create(&tid[t], NULL, Leitor, (void *)&id[t])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
  }  
  //--espera todas as threads terminarem
  for (t=0; t<NTHREADS; t++) {
    if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
  } 
  pthread_exit(NULL);
}
