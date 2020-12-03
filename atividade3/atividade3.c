#include <pthread.h>
#include <semaphore.h> 
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h> 

typedef long long soma_t;

typedef struct {
    pthread_t thread_id;
    int indiceInicio;
    int indiceFim;
    soma_t somaParcial;
    int *vetor;
    int totalThreads;    
    int *contadorThreads;
    struct timeval *tempoInicial;  
    pthread_mutex_t *mutex;
    pthread_cond_t *condicional;  
} blocoTrabalho;


void* fSomarVetorThread(void* argumento) {
    
    blocoTrabalho* bloco = (blocoTrabalho*)argumento;
    soma_t somaParcial = 0;

    pthread_mutex_lock(bloco->mutex);
    (*bloco->contadorThreads)++;
    if ((*bloco->contadorThreads) == bloco->totalThreads){
        *(bloco->contadorThreads) = 0;
        gettimeofday(bloco->tempoInicial, 0);
        pthread_cond_broadcast(bloco->condicional);
    } else {
        while ( pthread_cond_wait (bloco->condicional, bloco->mutex) != 0 );
    }
    pthread_mutex_unlock(bloco->mutex);


    for (int i = bloco->indiceInicio; i < bloco->indiceFim; i++) {
        somaParcial += bloco->vetor[i];
    }
    bloco->somaParcial = somaParcial;
    
    return argumento;
}


int main(int argc, char** argv) {

    int i, quantIndicesDoVetorPorThread, resto, tamanhoVetor, quantThreads, contadorThreads, * vetor;
    soma_t somaTotal;
    blocoTrabalho* blocos;
    struct timeval tempoInicial, tempoFinal;
    double tempoTotal;
    pthread_mutex_t mutex;
    pthread_cond_t condicional;


    tamanhoVetor = atoi(argv[1]);
    quantThreads = atoi(argv[2]);
    blocos = malloc(sizeof(blocoTrabalho) * quantThreads);
    vetor = malloc(sizeof(int) * tamanhoVetor);
    quantIndicesDoVetorPorThread = tamanhoVetor / quantThreads;
    resto = tamanhoVetor % quantThreads;
    somaTotal = 0;
    contadorThreads = 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condicional, NULL);


    for (i = 0; i < tamanhoVetor; i++) {
        vetor[i] = rand() % 1000000000;
    }

    for (i = 0; i < quantThreads; i++) {
        blocos[i].indiceInicio = i * quantIndicesDoVetorPorThread;
        blocos[i].indiceFim = blocos[i].indiceInicio + quantIndicesDoVetorPorThread;
        blocos[i].vetor = vetor;
        blocos[i].totalThreads = quantThreads;
        blocos[i].contadorThreads = &contadorThreads;
        blocos[i].tempoInicial = &tempoInicial;
        blocos[i].mutex = &mutex;
        blocos[i].condicional = &condicional;
    }
    blocos[quantThreads - 1].indiceFim += resto;

    
    for (i = 0; i < quantThreads; i++) {
        pthread_create(&(blocos[i].thread_id), NULL, fSomarVetorThread, &(blocos[i]));
    }

    void* bloco;
    for (i = 0; i < quantThreads; i++) {
        pthread_join(blocos[i].thread_id, &bloco);
        somaTotal += ((blocoTrabalho*)bloco)->somaParcial;
    }
    gettimeofday(&tempoFinal, 0);

    tempoTotal = (double)(tempoFinal.tv_sec + tempoFinal.tv_usec * 1.e-6) - (tempoInicial.tv_sec + tempoInicial.tv_usec * 1.e-6);
    printf("\nN_Threads %d  Tempo %f  Tamanho_Vetor %d  Soma %lld\n", quantThreads, tempoTotal, tamanhoVetor, somaTotal);

    free(vetor);
    free(blocos);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condicional);

    return 0;
}