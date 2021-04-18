/* TRABALHO 2: Jackson Willian Silva Agostinho - 20172BSI0335 - jacksonwillianjbv@gmail.com */

// #include <Windows.h>
#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/* MODO_DEBUG definido com valor 0 desativa os prints de debug, e definido com valor 1 ativa os prints de debug */
#define MODO_DEBUG 0           


typedef enum {
    false = 0,
    true = 1
} bool;


 struct cliente_t;
 struct NoCliente;
 struct cadeira_t;
 struct barbeiro_t;
 struct barbearia_t;


typedef struct cliente_t {
    pthread_t thread;
    int id;
} cliente_t;


/* usado para criar uma lista dinamica de clientes */
typedef struct NoCliente {	
	struct cliente_t cliente;
    struct barbearia_t * barbearia;
	struct NoCliente * prox;
} NoCliente, * PNoCliente;


typedef struct cadeira_t {
    sem_t situacao;
    PNoCliente pCliente;   /* referencia para o ultimo cliente que ocupou a cadeira */
} cadeira_t;


typedef struct barbeiro_t {
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


typedef struct barbearia_t {
    sem_t* barbeirosLiberado;
    sem_t* barbeirosAcordado;
    sem_t* barbeirosAtendeuCliente;
    sem_t* totalBarbeirosLiberados;
    sem_t* totalAtingiramObjetivo;    
    struct cadeira_t* cadeiraEspera;
    int totalBarbeiros;
    int totalCadeirasEspera;
} barbearia_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, idCliente;
    barbeiro_t* barbeiros;
    barbearia_t * barbearia;
    cadeira_t * cadeirasEspera;
    sem_t* barbeirosLiberado, * barbeirosAcordado, * barbeirosAtendeuCliente;
    sem_t  totalBarbeirosLiberados, totalAtingiramObjetivo;
    PNoCliente noInicial, noAuxiliar1, noAuxiliar2; 
   
    /* validar entradas */
    if (argc != 4) {
        printf("\nA quantidade de argumentos na linha de comando eh invalida!\n");
        return -1;
    }
    else if (atoi(argv[1]) < 1 || atoi(argv[2]) < 1 || atoi(argv[3]) < 1) {
        printf("\nO valor do argumento eh invalido!\n\n");
        return -1;
    }

    /* inicializar variaveis */
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros = malloc(sizeof(barbeiro_t) * quantBarbeiros);
    barbeirosLiberado = malloc(sizeof(sem_t) * quantBarbeiros);
    barbeirosAcordado = malloc(sizeof(sem_t) * quantBarbeiros);
    barbeirosAtendeuCliente = malloc(sizeof(sem_t) * quantBarbeiros);
    barbearia = malloc(sizeof(barbearia_t));
    cadeirasEspera = malloc(sizeof(cadeira_t) * quantCadeirasEspera);


    for (i=0; i < quantCadeirasEspera; i++) {
        if (sem_init(&(cadeirasEspera[i].situacao), 0, 1) != 0) {                    /* define a situacao de uma cadeira como livre (1) ou ocupada (0) */
            printf("\nErro ao inicializar cadeirasEspera de espera \n");
        }
    }

    if (sem_init(&totalAtingiramObjetivo, 0, 0) != 0) {               /* contagem de barbeiros que atingiram objetivo  */
        printf("\nErro ao inicializar semaforo\n");
        return -3;
    }
    if (sem_init(&totalBarbeirosLiberados, 0, quantBarbeiros) != 0) {  /* contagem de barbeiros liberados (livres), isto eh, os barbeiros que podem atender algum cliente se for acordado */
        printf("\nErro ao inicializar semaforo\n");
        return -4;
    }


    /* inicializa barbeiros */
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

    /* cria barbeiros */
    for (i = 0; i < quantBarbeiros; i++) {
        if (pthread_create(&(barbeiros[i].thread), NULL, f_barbeiro, &(barbeiros[i])) != 0) {
            printf("\nErro ao criar thread barbeiro %d\n", i);
            // nao retornar
        }
    }

    /* cria clientes enquanto todos os barbeiros ainda nao atingiram o objetivo */
    atingiramObjetivo = 0;
    idCliente = 0;

    barbearia->barbeirosLiberado = barbeirosLiberado;
    barbearia->cadeiraEspera = cadeirasEspera;
    barbearia->barbeirosAcordado = barbeirosAcordado;
    barbearia->barbeirosAtendeuCliente = barbeirosAtendeuCliente;
    barbearia->totalBarbeirosLiberados = &totalBarbeirosLiberados;
    barbearia->totalAtingiramObjetivo = &totalAtingiramObjetivo;
    barbearia->totalBarbeiros = quantBarbeiros;    
    barbearia->totalCadeirasEspera = quantCadeirasEspera;    

	noInicial = (PNoCliente) malloc(sizeof(NoCliente)); 
    noAuxiliar1 = noInicial;
    noAuxiliar1->prox = NULL;

    noAuxiliar1->cliente.id = idCliente;
    noAuxiliar1->barbearia = barbearia;

    if (pthread_create(&(noAuxiliar1->cliente.thread), NULL, f_cliente, &(noAuxiliar1->cliente)) != 0) {
        printf("\nErro ao criar thread cliente %d\n", noAuxiliar1->cliente.id);
    }

    if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
        atingiramObjetivo = 0;
    }

    while (atingiramObjetivo < quantBarbeiros) {

        idCliente++;
        noAuxiliar2 = (PNoCliente) malloc(sizeof(NoCliente)); 
        noAuxiliar2->prox = NULL;

        noAuxiliar2->cliente.id = idCliente;
        noAuxiliar2->barbearia = barbearia;

        noAuxiliar1->prox = noAuxiliar2;
        noAuxiliar1 = noAuxiliar2;

        #if MODO_DEBUG
        printf("cria cliente %d\n", idCliente);
        #endif

        if (pthread_create(&(noAuxiliar2->cliente.thread), NULL, f_cliente, &(noAuxiliar2->cliente)) != 0) {
            printf("\nErro ao criar thread cliente %d\n", noAuxiliar2->cliente.id);
        } 

        if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
            atingiramObjetivo = 0;
        }
    }
    

    /* aguarda todas as threads clientes terminarem e depois libera memoria */
    // noAuxiliar1 = NULL;
    // noAuxiliar2 = NULL;
    // noAuxiliar1 = noInicial;
    // while (noAuxiliar1 != NULL) {
    //     if (pthread_join(noAuxiliar1->cliente.thread, NULL) != 0) {
    //         printf("\nErro ao unir thread cliente %d\n", noAuxiliar1->cliente.id);
    //         return -16;
    //     }
    //     noAuxiliar2 = noAuxiliar1->prox;
    //     free(noAuxiliar1);
    //     noAuxiliar1 = noAuxiliar2;
    // }

    /* aguarda cliente da cadeira de espera */
    for (i = 0; i < quantCadeirasEspera; i++) {
        if (pthread_join(cadeirasEspera[i].pCliente->cliente.thread, NULL) != 0) {
            printf("\nErro ao unir thread cliente %d\n", cadeirasEspera[i].pCliente->cliente.id);
            // return -16;
        }    
    }


    /* exibi a quantidade de clientes que cada barbeiro atendeu */
    for (i = 0; i < quantBarbeiros; i++) {
        printf("barbeiro %d atendeu %d clientes\n", barbeiros[i].id, barbeiros[i].clientesAtendidos);
    }

    
    /* cancela threads barbeiro */
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

    printf("sdsasa\n");

    /* libera memoria */
    // for(i = 0; i < quantBarbeiros; i++) {
    //     sem_destroy(&(barbeirosLiberado[i]));
    //     sem_destroy(&(barbeirosAcordado[i]));
    //     sem_destroy(&(barbeirosAtendeuCliente[i]));
    // }
    // sem_destroy(&totalBarbeirosLiberados);
    // sem_destroy(&totalAtingiramObjetivo);
    // free(barbeiros);
    // free(barbearia);
    // free(barbeirosLiberado);
    // free(barbeirosAcordado);
    // free(barbeirosAtendeuCliente);
    
    pthread_exit(0);
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

        // Sleep(10); // barbeiro estah atendendo cliente

        barbeiro->clientesAtendidos++;

        if (barbeiro->clientesAtendidos == barbeiro->quantMinimaClientes) {
            #if MODO_DEBUG
            printf("barbeiro %d atingiu seu objetivo!\n", barbeiro->id);
            #endif
            sem_post(barbeiro->totalAtingiramObjetivo);
        }

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

    PNoCliente pCliente = (PNoCliente)argumento;

    bool clienteAtendido = false;
    int i, barbeirosVerificados, cadeiraID, quantAtingiramObjetivo;
    
    /* ocupa cadeira de espera se alguma estiver livre, e pega o ID da cadeira */
    cadeiraID = -1;
    for(i=0; i < pCliente->barbearia->totalCadeirasEspera; i++) {
        if(sem_trywait(&(pCliente->barbearia->cadeiraEspera[i].situacao)) == 0) {
            cadeiraID = i;
            break;
        }
    }

    if (cadeiraID != -1) {

        /* cliente ocupou a cadeira */
        pCliente->barbearia->cadeiraEspera[cadeiraID].pCliente = pCliente; // ultimo cliente que ocupou a cadeira

        #if MODO_DEBUG
        printf("cliente %d ocupou a cadeira %d\n", cadeiraID);
        #endif

        while (clienteAtendido == false) {

            sem_wait(pCliente->barbearia->totalBarbeirosLiberados); /* caso não tenha barbeiros livres o cliente vai esperar aqui */

            srand(time(NULL)); // muda semente do rand() 
            i = (rand() % pCliente->barbearia->totalBarbeiros); // indice inicial
            barbeirosVerificados = 0;

            while (barbeirosVerificados < pCliente->barbearia->totalBarbeiros) {

                if (sem_trywait(&(pCliente->barbearia->barbeirosLiberado[i])) == 0) { /* vai ao encontro do barbeiro livre */
                    #if MODO_DEBUG
                    printf("cliente %d acorda barbeiro %d \n", pCliente->cliente->id, i);
                    #endif
                    sem_post(&(pCliente->barbearia->cadeiraEspera[cadeiraID].situacao));  /* libera cadeira de espera */
                    sem_post(&(pCliente->barbearia->barbeirosAcordado[i])); /* acorda barbeiro */
                    #if MODO_DEBUG
                    printf("cliente %d estah esperando o barbeiro %d \n", pCliente->cliente->id, i);
                    #endif
                    sem_wait(&(pCliente->barbearia->barbeirosAtendeuCliente[i])); /* cliente espera, na cadeira do barbeiro, o fim do atendimento */
                    clienteAtendido = true;
                    #if MODO_DEBUG
                    printf("cliente %d foi atendido pelo barbeiro %d \n", pCliente->cliente->id, i);
                    #endif
                    break;
                } // fim if

                i++;
                i = (i >= pCliente->barbearia->totalBarbeiros) ? 0 : i;
                barbeirosVerificados++;

            } // fim while

        } // fim while

        #if MODO_DEBUG
        printf("cliente %d saiu\n", pCliente->cliente->id);
        #endif

    }
    else {
        #if MODO_DEBUG
        printf("cliente %d nao entrou\n", pCliente->cliente->id);
        #endif
    }
    
    return NULL;
}