#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// apelida o tipo de dado para soma_t
typedef unsigned long long soma_t;

// define a estrutura de um bloco de trabalho
typedef struct {

    pthread_t threadID;
    int id;
    int indiceInicio;
    int indiceFim;
    soma_t somaParcial;
    int *vetor;

} blocoTrabalho;


// funcao de soma usa pela threads
void *fSomarVetorThread(void *argumento) {  

    // declara variavel e inicializa variavel
    blocoTrabalho * bloco = (blocoTrabalho *) argumento;

    // realiza a soma
    for(int i = bloco->indiceInicio; i < bloco->indiceFim; i++){
        bloco->somaParcial += bloco->vetor[i];
    }

    return NULL;
}

// funcao principal
int main(int argc, char **argv){

    // declara variavel
    int i, quantIndicesDoVetorPorThread, resto, tamanhoVetor, quantThreads, *vetor;
    soma_t somaTotal;
    clock_t clocksInicio, clocksFim;
    pthread_t *threads;
    blocoTrabalho *blocos;

    // inicializa a variavel
    tamanhoVetor = atoi(argv[1]); 
    quantThreads = atoi(argv[2]); 
    threads = malloc( sizeof(pthread_t) * quantThreads );
    blocos = malloc( sizeof(blocoTrabalho) * quantThreads );
    vetor = malloc(sizeof(int) * tamanhoVetor);
    quantIndicesDoVetorPorThread = tamanhoVetor / quantThreads;
    resto = tamanhoVetor % quantThreads;

    // inicializa o vetor
    for (i = 0; i < tamanhoVetor; i++){
        vetor[i] = rand() % 100000;
    }

    // inicio da soma serial
    somaTotal = 0; 
    clocksInicio = clock();
    for (i = 0; i < tamanhoVetor; i++){
        somaTotal += vetor[i];
    }
    clocksFim = clock(); 

    printf("\nA soma do numeros do vetor [serial]: %llu\nClocks decorridos %ld\n", somaTotal, (clocksFim - clocksInicio) );

    // inicio da soma paralela
    for(i=0; i < quantThreads; i++){
        blocos[i].id = i;
        blocos[i].indiceInicio = i * quantIndicesDoVetorPorThread;
        blocos[i].indiceFim = blocos[i].indiceInicio + quantIndicesDoVetorPorThread;
        blocos[i].vetor = vetor;
        blocos[i].somaParcial = 0;
    }
    blocos[quantThreads-1].indiceFim += resto;

    // cria as threads
    clocksInicio = clock();
    for(i=0; i<quantThreads; i++ ){
        pthread_create(&(blocos[i].threadID), NULL, fSomarVetorThread, &(blocos[i]) );
    }

    // realiza a soma
    somaTotal = 0;
    for(i=0; i<quantThreads; i++){
        pthread_join(blocos[i].threadID, NULL);
        somaTotal += blocos[i].somaParcial;
    }
    clocksFim = clock(); 
   
    printf("\nA soma do numeros do vetor [paral.]: %llu\nClocks decorridos %ld\n\n", somaTotal, (clocksFim - clocksInicio) );

    // libera memoria dinamicamente alocada
    free(vetor);
    free(threads);
    free(blocos);   

    return 0;
}