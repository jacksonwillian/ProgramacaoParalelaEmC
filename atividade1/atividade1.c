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
    int *vetor;

} blocoTrabalho;


// funcao de soma usa pela threads
void *fSomarVetorThread(void *argumento) {  

    // declara variavel
    blocoTrabalho * bloco;
    soma_t *somaLocal;

    // inicializa variavel
    bloco = (blocoTrabalho *) argumento;
    somaLocal = malloc(sizeof(soma_t));    
    *somaLocal = 0;

    // realiza a soma
    for(int i = bloco->indiceInicio; i < bloco->indiceFim; i++){
        *somaLocal += bloco->vetor[i];
    }

    // retorna o valor da soma
    pthread_exit( (void*) somaLocal );
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
    somaTotal = 0;

    // inicializa o vetor
    for (i = 0; i < tamanhoVetor; i++){
        vetor[i] = rand() % 100000;
    }

    // inicio da soma serial
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
        void *soma_parcial;
        pthread_join(blocos[i].threadID, &soma_parcial);
        somaTotal += *( (soma_t *)soma_parcial );
        free(soma_parcial);
    }
    clocksFim = clock(); 
   
    printf("\nA soma do numeros do vetor [paral.]: %llu\nClocks decorridos %ld\n\n", somaTotal, (clocksFim - clocksInicio) );

    pthread_exit(NULL);

    // libera memoria dinamicamente alocada
    free(vetor);
    free(threads);
    free(blocos);   

    return 0;
}