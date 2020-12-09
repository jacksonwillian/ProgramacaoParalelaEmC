#include <pthread.h>
#include <semaphore.h> 
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

#define BLOQUEADO 0

typedef struct {
    pthread_t thread;
    int id;
    int capacidadeMaxima;
    sem_t *garfoEsquerdo;
    sem_t *garfoDireito;
} filosofo_t;


void* f_jantar (void* argumento) {

    filosofo_t *filosofo = (filosofo_t *)argumento;
    int quantIngerida = 0;
    int garfoEsquerdo = 0;
    int garfoDireito = 0;
    int tempo = 0;

    while (quantIngerida < filosofo->capacidadeMaxima) {

        printf("\nFilosofo [%d] estah pensando...", filosofo->id);

        sleep(tempo + rand() % 5);

        // Se sem_trywait executou com sucesso a operacao de bloqueio do semaforo, entao retorna zero.
        // Senao retorna um valor -1 e o estado do semáforo fica inalterado.
        garfoEsquerdo = sem_trywait(filosofo->garfoEsquerdo);
        garfoDireito = sem_trywait(filosofo->garfoDireito);
        
        if (garfoEsquerdo == BLOQUEADO && garfoDireito == BLOQUEADO){
            printf("\nFilosofo [%d] estah comendo pela %dª vez.", filosofo->id, (quantIngerida + 1));
            quantIngerida += 1;
            tempo = 5; // atribui tempo de esperar maior
        } else {
            printf("\nOoops! O Filosofo [%d] não conseguiu pegar os garfos.", filosofo->id);
            tempo = 0;
        }

        // Se o semaforo esquerdo foi bloqueado, entao ele eh liberado
        if (garfoEsquerdo == BLOQUEADO) {
            sem_post(filosofo->garfoEsquerdo);
        }

        // Se o semaforo direito foi bloqueado, entao ele eh liberado
        if (garfoDireito == BLOQUEADO) {
            sem_post(filosofo->garfoDireito);
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
        printf("\nArgumento 1 eh a quantidade total de filosofos");
        printf("\nArgumento 2 eh a quantidade maxima que cada um pode comer\n");
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