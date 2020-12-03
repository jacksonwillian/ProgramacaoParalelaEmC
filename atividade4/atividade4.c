#include <pthread.h>
#include <semaphore.h> 
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

typedef struct {
    pthread_t thread;
    int id;
    int quantIngerida;
    int capacidadeMaxima;
    sem_t *garfoEsquerdo;
    sem_t *garfoDireito;
} filosofo_t;


void* f_jantar (void* argumento) {

    filosofo_t *filosofo = (filosofo_t *)argumento;

    while (filosofo->quantIngerida < filosofo->capacidadeMaxima){
        printf("\nFilosofo [%d] estah pensando...", filosofo->id);
        if (sem_trywait(filosofo->garfoEsquerdo) == 0 && sem_trywait(filosofo->garfoDireito) == 0){
            printf("\nFilosofo [%d] estah comendo pela %dª vez.", filosofo->id, (filosofo->quantIngerida + 1));
            filosofo->quantIngerida += 1;
            sem_post(filosofo->garfoEsquerdo);
            sem_post(filosofo->garfoDireito);
            sleep(rand() % 20);
        } else {
            printf("\nOoops! O Filosofo [%d] não conseguiu pegar os garfos.", filosofo->id);
        }
        
    }
    printf("\nFilosofo [%d] estah satisfeito e foi embora.", filosofo->id);
    return NULL;
}


int main(int argc, char** argv) {

    /* DECLARACAO DAS VARIAVEIS */
    int i, quantGarfos, quantFilosofo, capacidadeMaxima;
    filosofo_t *filosofos;
    sem_t *garfos;


    /* VALIDACAO DE ENTRADAS */
    if ( argc != 3 ) {
        printf("\nQuantidades de argumentos são inválidos!\n");
        return -1;
    } else if ( atoi(argv[1]) < 2 ||  0 == atoi(argv[2]) ) {
        printf("\nValores de argumentos são inválidos!\n");
        return -1;
    }   


    /* INICIALIZACAO DAS VARIAVEIS */    
    quantFilosofo = atoi(argv[1]);
    capacidadeMaxima = atoi(argv[2]);
    quantGarfos = quantFilosofo;
    garfos = malloc(sizeof(sem_t) * quantGarfos);
    filosofos = malloc(sizeof(filosofo_t) * quantFilosofo);

    for (i = 0; i < quantGarfos; i++){
        sem_init(&(garfos[i]), 0, 1);
    }

    for (i=0; i < quantFilosofo; i++){
        filosofos[i].id = i+1;
        filosofos[i].quantIngerida = 0;
        filosofos[i].capacidadeMaxima = capacidadeMaxima;
        filosofos[i].garfoEsquerdo = &(garfos[i]);      
        filosofos[i].garfoDireito = &(garfos[(i+1)%quantGarfos]);      
    }

    /* MENSAGEM DE BOAS-VINDAS */    
    printf("\n-- JANTAR DOS FILOSOFOS --\n");
    printf("\n%d Filosofos que pode comer até %d veze(s)\n", quantFilosofo, capacidadeMaxima);


    /* EXECUTAS AS THREADS */    
    for (i = 0; i < quantFilosofo; i++) {
        pthread_create(&(filosofos[i].thread), NULL, f_jantar, &(filosofos[i]));
    }

    for (i = 0; i < quantFilosofo; i++) {
        pthread_join(filosofos[i].thread, NULL);
    }


    /* DESTROI SEMAFORO */  
    for (i = 0; i < quantGarfos; i++){
        sem_destroy(&(garfos[i]));
    }
    
    return 0;
}