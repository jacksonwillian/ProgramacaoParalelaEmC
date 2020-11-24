#include <pthread.h>
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
} blocoTrabalho;


void* fSomarVetorThread(void* argumento) {
    blocoTrabalho* bloco = (blocoTrabalho*)argumento;
    soma_t somaParcial = 0;
    for (int i = bloco->indiceInicio; i < bloco->indiceFim; i++) {
        somaParcial += bloco->vetor[i];
    }
    bloco->somaParcial = somaParcial;
    return argumento;
}


int main(int argc, char** argv) {

    int i, quantIndicesDoVetorPorThread, resto, tamanhoVetor, quantThreads, * vetor;
    soma_t somaTotal;
    blocoTrabalho* blocos;
    struct timeval tempoInicial, tempoFinal;

    tamanhoVetor = atoi(argv[1]);
    quantThreads = atoi(argv[2]);
    blocos = malloc(sizeof(blocoTrabalho) * quantThreads);
    vetor = malloc(sizeof(int) * tamanhoVetor);
    quantIndicesDoVetorPorThread = tamanhoVetor / quantThreads;
    resto = tamanhoVetor % quantThreads;

    for (i = 0; i < tamanhoVetor; i++) {
        vetor[i] = rand() % 1000000000;
    }

    for (i = 0; i < quantThreads; i++) {
        blocos[i].indiceInicio = i * quantIndicesDoVetorPorThread;
        blocos[i].indiceFim = blocos[i].indiceInicio + quantIndicesDoVetorPorThread;
        blocos[i].vetor = vetor;
    }
    blocos[quantThreads - 1].indiceFim += resto;

    gettimeofday(&tempoInicial, 0);
    for (i = 0; i < quantThreads; i++) {
        pthread_create(&(blocos[i].thread_id), NULL, fSomarVetorThread, &(blocos[i]));
    }

    somaTotal = 0;
    void* bloco;
    for (i = 0; i < quantThreads; i++) {
        pthread_join(blocos[i].thread_id, &bloco);
        somaTotal += ((blocoTrabalho*)bloco)->somaParcial;
    }
    gettimeofday(&tempoFinal, 0);    

    printf("\nN_Threads %d  Tempo %f  Tamanho_Vetor %d  Soma %lld\n", quantThreads, (tempoFinal.tv_sec + tempoFinal.tv_usec*1.e-6) - (tempoInicial.tv_sec + tempoInicial.tv_usec*1.e-6), tamanhoVetor, somaTotal);

    free(vetor);
    free(blocos);

    return 0;
}