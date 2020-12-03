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
    int* vetor;
    int quantThreads;
    int* quantBloqueadas;
    struct timeval* tempoInicial;
    sem_t* semaforoBarreira1;
    sem_t* semaforoBarreira2;
} blocoTrabalho;


void* fSomarVetorThread(void* argumento) {
    blocoTrabalho* bloco = (blocoTrabalho*)argumento;
    soma_t somaParcial = 0;

    sem_wait(bloco->semaforoBarreira1);
    *(bloco->quantBloqueadas) += 1;
    if (*(bloco->quantBloqueadas) == bloco->quantThreads) {
        sem_post(bloco->semaforoBarreira1);
        gettimeofday(bloco->tempoInicial, 0);
        while (*(bloco->quantBloqueadas) > 0) {
            sem_post(bloco->semaforoBarreira2);
            *(bloco->quantBloqueadas) -= 1;
        }
    }
    else {
        sem_post(bloco->semaforoBarreira1);
        sem_wait(bloco->semaforoBarreira2);
    }

    for (int i = bloco->indiceInicio; i < bloco->indiceFim; i++) {
        somaParcial += bloco->vetor[i];
    }
    bloco->somaParcial = somaParcial;
    
    return argumento;
}


int main(int argc, char** argv) {

    int i, quantIndicesDoVetorPorThread, resto, tamanhoVetor, quantThreads, quantBloqueadas, * vetor;
    soma_t somaTotal;
    blocoTrabalho* blocos;
    struct timeval tempoInicial, tempoFinal;
    double tempoTotal;
    sem_t semaforoBarreira1, semaforoBarreira2;

    tamanhoVetor = atoi(argv[1]);
    quantThreads = atoi(argv[2]);
    blocos = malloc(sizeof(blocoTrabalho) * quantThreads);
    vetor = malloc(sizeof(int) * tamanhoVetor);
    quantIndicesDoVetorPorThread = tamanhoVetor / quantThreads;
    resto = tamanhoVetor % quantThreads;
    somaTotal = 0;
    quantBloqueadas = 0;
    sem_init(&semaforoBarreira1, 0, 1);
    sem_init(&semaforoBarreira2, 0, 0);


    for (i = 0; i < tamanhoVetor; i++) {
        vetor[i] = rand() % 1000000000;
    }

    for (i = 0; i < quantThreads; i++) {
        blocos[i].indiceInicio = i * quantIndicesDoVetorPorThread;
        blocos[i].indiceFim = blocos[i].indiceInicio + quantIndicesDoVetorPorThread;
        blocos[i].vetor = vetor;
        blocos[i].quantThreads = quantThreads;
        blocos[i].quantBloqueadas = &quantBloqueadas;
        blocos[i].tempoInicial = &tempoInicial;
        blocos[i].semaforoBarreira1 = &semaforoBarreira1;
        blocos[i].semaforoBarreira2 = &semaforoBarreira2;
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
    sem_destroy(&semaforoBarreira1);
    sem_destroy(&semaforoBarreira2);

    return 0;
}