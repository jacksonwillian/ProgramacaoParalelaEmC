/* TRABALHO 1: Jackson Willian Silva Agostinho - 20172BSI0335 */

#include <Windows.h>
#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/* MODO_DEBUG definido com valor 0 desativa os prints de debug, e definido com valor 1 ativa os prints de debug */
#define MODO_DEBUG 1           


typedef enum {
    false = 0,
    true = 1
} bool;


typedef struct {
    pthread_t thread;
    int id;
    sem_t* barbeiroLiberado;
    sem_t* barbeirosAcordado;
    sem_t* barbeirosAtendeuCliente;
    int quantMinimaClientes;
    int clientesAtendidos;
    sem_t* totalBarbeirosLiberados;
    sem_t* totalAtingiramObjetivo;
    int totalBarbeiros;
} barbeiro_t;


typedef struct {
    pthread_t thread;
    int id;
    sem_t* barbeirosLiberado;
    sem_t* cadeiraEspera;
    sem_t* barbeirosAcordado;
    sem_t* barbeirosAtendeuCliente;
    sem_t* totalBarbeirosLiberados;
    sem_t* totalAtingiramObjetivo;
    int totalBarbeiros;
} cliente_t;


typedef struct TNo {	
	cliente_t cliente;    // elemento
	struct TNo *prox;     // nó posterior
} TNo, * PNo;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, idCliente;
    barbeiro_t* barbeiros;
    cliente_t* cliente;
    sem_t* barbeirosLiberado, * barbeirosAcordado, * barbeirosAtendeuCliente;
    sem_t totalBarbeirosLiberados;
    sem_t cadeiraEspera;
    sem_t totalAtingiramObjetivo;
    sem_t totalClientesVisitaramBarbearia;
   
   
    // validar entradas
    if (argc != 4) {
        printf("\nA quantidade de argumentos na linha de comando eh invalida!\n");
        return -1;
    }
    else if (atoi(argv[1]) < 1 || atoi(argv[2]) < 1 || atoi(argv[3]) < 1) {
        printf("\nO valor do argumento eh invalido!\n\n");
        return -1;
    }

    // inicializar variaveis
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros = malloc(sizeof(barbeiro_t) * quantBarbeiros);
    barbeirosLiberado = malloc(sizeof(sem_t) * quantBarbeiros);
    barbeirosAcordado = malloc(sizeof(sem_t) * quantBarbeiros);
    barbeirosAtendeuCliente = malloc(sizeof(sem_t) * quantBarbeiros);
    cliente =  malloc(sizeof(cliente_t)); 

    if (sem_init(&cadeiraEspera, 0, quantCadeirasEspera) != 0) {      /* a barbearia abre com todas cadeiras de espera livres */
        printf("\nErro ao inicializar semaforo\n");
        return -2;
    }
    if (sem_init(&totalAtingiramObjetivo, 0, 0) != 0) {               /* contagem de barbeiros que atingiram objetivo  */
        printf("\nErro ao inicializar semaforo\n");
        return -3;
    }
    if (sem_init(&totalBarbeirosLiberados, 0, quantBarbeiros) != 0) {  /* contagem de barbeiros liberados (livres), isto eh, os barbeiros que podem atender algum cliente se for acordado */
        printf("\nErro ao inicializar semaforo\n");
        return -4;
    }

    if (sem_init(&totalClientesVisitaramBarbearia, 0, 0) != 0) {         /* contagem de cliente dentro da barbearia */
        printf("\nErro ao inicializar semaforo\n");
        return -6;
    }



    // inicializa barbeiros
    for (i = 0; i < quantBarbeiros; i++) {
        barbeiros[i].id = i;
        barbeiros[i].quantMinimaClientes = quantMinimaClientes;
        barbeiros[i].clientesAtendidos = 0;
        barbeiros[i].totalAtingiramObjetivo = &totalAtingiramObjetivo;
        barbeiros[i].totalBarbeiros = quantBarbeiros;
        barbeiros[i].barbeiroLiberado = &(barbeirosLiberado[i]);
        barbeiros[i].barbeirosAcordado = &(barbeirosAcordado[i]);
        barbeiros[i].barbeirosAtendeuCliente = &(barbeirosAtendeuCliente[i]);
        barbeiros[i].totalBarbeirosLiberados = &totalBarbeirosLiberados;

        if (sem_init(&(barbeirosLiberado[i]), 0, 1) != 0) {            /* define se um barbeiro especifico estah livre (1) ou ocupado (0) */
            printf("\nErro ao inicializar semaforo\n");
            return -9;
        }
        if (sem_init(&(barbeirosAcordado[i]), 0, 0) != 0) {            /* define se um barbeiro especifico estah acordado (1) ou dormindo (0) */
            printf("\nErro ao inicializar semaforo\n");
            return -10;
        }
        if (sem_init(&(barbeirosAtendeuCliente[i]), 0, 0) != 0) {      /* define se um barbeiro especifico terminou o atendimento ao cliente (1) ou ainda vai terminar (0) */
            printf("\nErro ao inicializar semaforo\n");
            return -11;
        }
    }

    // cria barbeiros
    for (i = 0; i < quantBarbeiros; i++) {
        if (pthread_create(&(barbeiros[i].thread), NULL, f_barbeiro, &(barbeiros[i])) != 0) {
            printf("\nErro ao criar thread barbeiro %d\n", i);
            return -12;
        }
    }

    // cria cliente
    atingiramObjetivo = 0;
    idCliente = 0;
    
    PNo noInical, noAuxiliar1, noAuxiliar2; 
	noInical = (PNo) malloc(sizeof(TNo)); 
    noAuxiliar1 = noInical;
    noAuxiliar1->prox = NULL;

    noAuxiliar1->cliente.id = idCliente;
    noAuxiliar1->cliente.barbeirosLiberado = barbeirosLiberado;
    noAuxiliar1->cliente.cadeiraEspera = &cadeiraEspera;
    noAuxiliar1->cliente.barbeirosAcordado = barbeirosAcordado;
    noAuxiliar1->cliente.barbeirosAtendeuCliente = barbeirosAtendeuCliente;
    noAuxiliar1->cliente.totalBarbeiros = quantBarbeiros;
    noAuxiliar1->cliente.totalBarbeirosLiberados = &totalBarbeirosLiberados;
    noAuxiliar1->cliente.totalAtingiramObjetivo = &totalAtingiramObjetivo;

    if (pthread_create(&(noAuxiliar1->cliente.thread), NULL, f_cliente, &(noAuxiliar1->cliente)) != 0) {
        printf("\nErro ao criar thread cliente %d\n", noAuxiliar1->cliente.id);
        return -13;
    }

    if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
        atingiramObjetivo = 0;
    }

    while (atingiramObjetivo < quantBarbeiros) {

        idCliente++;
        noAuxiliar2 = (PNo) malloc(sizeof(TNo)); 
        noAuxiliar2->prox = NULL;

        noAuxiliar2->cliente.id = idCliente;
        noAuxiliar2->cliente.barbeirosLiberado = barbeirosLiberado;
        noAuxiliar2->cliente.cadeiraEspera = &cadeiraEspera;
        noAuxiliar2->cliente.barbeirosAcordado = barbeirosAcordado;
        noAuxiliar2->cliente.barbeirosAtendeuCliente = barbeirosAtendeuCliente;
        noAuxiliar2->cliente.totalBarbeiros = quantBarbeiros;
        noAuxiliar2->cliente.totalBarbeirosLiberados = &totalBarbeirosLiberados;
        noAuxiliar2->cliente.totalAtingiramObjetivo = &totalAtingiramObjetivo;

        noAuxiliar1->prox = noAuxiliar2;
        noAuxiliar1 = noAuxiliar2;

        if (pthread_create(&(noAuxiliar2->cliente.thread), NULL, f_cliente, &(noAuxiliar2->cliente)) != 0) {
            printf("\nErro ao criar thread cliente %d\n", noAuxiliar2->cliente.id);
            return -13;
        } 

        if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
            atingiramObjetivo = 0;
        }
    }
    
    noAuxiliar1 = NULL;
    noAuxiliar2 = NULL;
    noAuxiliar1 = noInical;
    while (noAuxiliar1 != NULL) {
        if (pthread_join(noAuxiliar1->cliente.thread, NULL) != 0) {
            printf("\nErro ao unir thread cliente %d\n", noAuxiliar1->cliente.id);
            return -16;
        }
        noAuxiliar2 = noAuxiliar1->prox;
        free(noAuxiliar1);
        noAuxiliar1 = noAuxiliar2;
    }

    // exibi o babeiro
    for (i = 0; i < quantBarbeiros; i++) {
        printf("barbeiro %d atendeu %d clientes\n", barbeiros[i].id, barbeiros[i].clientesAtendidos);
    }

    // cancela threads babeiro
    // https://wiki.sei.cmu.edu/confluence/display/c/POS44-C.+Do+not+use+signals+to+terminate+threads
    for (i = 0; i < quantBarbeiros; i++) {
        if (pthread_cancel(barbeiros[i].thread) != 0) {
            printf("\nErro ao cancelar thread barbeiro %d\n", i);
            return -15;
        }
    }

    for (i = 0; i < quantBarbeiros; i++) {
        if (pthread_join(barbeiros[i].thread, NULL) != 0) {
            printf("\nErro ao unir thread barbeiro %d\n", i);
            return -16;
        }
    }

    /* libera memoria */

    for(i = 0; i < quantBarbeiros; i++) {
        sem_destroy(&(barbeirosLiberado[i]));
        sem_destroy(&(barbeirosAcordado[i]));
        sem_destroy(&(barbeirosAtendeuCliente[i]));
    }

    sem_destroy(&totalBarbeirosLiberados);
    sem_destroy(&totalAtingiramObjetivo);
    free(barbeiros);
    free(barbeirosLiberado);
    free(barbeirosAcordado);
    free(barbeirosAtendeuCliente);
    free(cliente);
    
    return 0;
}



void* f_barbeiro(void* argumento) {

    barbeiro_t* barbeiro = (barbeiro_t*)argumento;

    #if MODO_DEBUG
    printf("barbeiro id %d entrou\n", barbeiro->id);
    #endif

    while (true) {
        sem_wait(barbeiro->barbeirosAcordado); /* barbeiro estah dormindo */

        #if MODO_DEBUG
        printf("barbeiro %d acordou!\n", barbeiro->id);
        #endif

        barbeiro->clientesAtendidos++;

        if (barbeiro->clientesAtendidos == barbeiro->quantMinimaClientes) {
            #if MODO_DEBUG
            printf("barbeiro %d atingiu seu objetivo!\n", barbeiro->id);
            #endif
            sem_post(barbeiro->totalAtingiramObjetivo);
        }

        Sleep(50); // barbeiro atendendo cliente

        sem_post(barbeiro->barbeirosAtendeuCliente); /* barbeiro terminou de atender o cliente */

        #if MODO_DEBUG
        printf("barbeiro %d atendeu um cliente! \n", barbeiro->id);
        #endif

        sem_post(barbeiro->barbeiroLiberado); /* barbeiro estah livre */
        sem_post(barbeiro->totalBarbeirosLiberados); /* incrementa total barbeiros liberados */

    }

    return NULL;
}


void* f_cliente(void* argumento) {

    cliente_t* cliente = (cliente_t*)argumento;

    bool clienteAtendido = false;
    int i, barbeirosVerificados;


    if (sem_trywait(cliente->cadeiraEspera) == 0) { /* ocupa cadeira de espera se alguma estiver livre */

        #if MODO_DEBUG
        printf("cliente %d entrou\n", cliente->id);
        #endif

        while (clienteAtendido == false) {

            sem_wait(cliente->totalBarbeirosLiberados); /* caso não tenha barbeiros livres o cliente vai esperar aqui */

            srand(time(NULL)); // os valores gerados não se repitam
            i = (rand() % cliente->totalBarbeiros);
            barbeirosVerificados = 0;

            while (barbeirosVerificados < cliente->totalBarbeiros) {

                if (sem_trywait(&(cliente->barbeirosLiberado[i])) == 0) { /* vai ao encontro do barbeiro livre */
                    #if MODO_DEBUG
                    printf("cliente %d acorda barbeiro %d \n", cliente->id, i);
                    #endif
                    sem_post(cliente->cadeiraEspera);  /* libera cadeira de espera */
                    sem_post(&(cliente->barbeirosAcordado[i])); /* acorda barbeiro */
                    #if MODO_DEBUG
                    printf("cliente %d estah esperando o barbeiro %d \n", cliente->id, i);
                    #endif
                    sem_wait(&(cliente->barbeirosAtendeuCliente[i])); /* cliente espera, na cadeira do barbeiro, o fim do atendimento */
                    clienteAtendido = true;
                    #if MODO_DEBUG
                    printf("cliente %d foi atendido pelo barbeiro %d \n", cliente->id, i);
                    #endif
                    break;
                } // fim if

                i++;
                i = (i >= cliente->totalBarbeiros) ? 0 : i;
                barbeirosVerificados++;

            } // fim while

        } // fim while

        #if MODO_DEBUG
        printf("cliente %d saiu\n", cliente->id);
        #endif

    }
    else {
        #if MODO_DEBUG
        printf("cliente %d nao entrou\n", cliente->id);
        #endif
    }

    return NULL;
}