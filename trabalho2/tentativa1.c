/* TRABALHO 1: Jackson Willian Silva Agostinho - 20172BSI0335 */ 

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

// ultimo id
int CLIENTE_ULTIMO_ID = 0;

typedef enum {
    false = 0,
    true = 1
} bool;


typedef struct {
    pthread_t thread;
    int id;    
    sem_t * barbeiroLiberado;
    sem_t * barbeirosDormindo;
    int quantMinimaClientes;
    int clientesAtendidos;
    sem_t * totalBarbeirosLiberados;
    sem_t * totalAtingiramObjetivo;
    int totalBarbeiros;
    double tempoCorteCabelo;
} barbeiro_t;


typedef struct {
    pthread_t thread;
    sem_t * barbeirosLiberado;
    sem_t * cadeiraEspera;
    sem_t * barbeirosDormindo;
    int totalBarbeiros;
    pthread_mutex_t *mutexClienteID;
    sem_t * totalBarbeirosLiberados;
    double tempoCorteCabelo;
} cliente_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {
    // a soma do total de clientes atentidos deve ser um tipo long

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, tempoEspera;
    barbeiro_t * barbeiros;
    cliente_t * cliente;
    sem_t  * barbeirosLiberado, * barbeirosDormindo;
    sem_t totalBarbeirosLiberados;
    sem_t cadeiraEspera;
    sem_t totalAtingiramObjetivo;
    pthread_mutex_t mutexClienteID;
    double tempoCorteCabelo;

    // validar entradas


    // inicializar VARIAVEIS
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros =  malloc(sizeof(barbeiro_t) * quantBarbeiros); 
    barbeirosLiberado =  malloc(sizeof(sem_t) * quantBarbeiros); 
    barbeirosDormindo =  malloc(sizeof(sem_t) * quantBarbeiros); 
    cliente =  malloc(sizeof(cliente_t)); 
    sem_init(&cadeiraEspera, 0, quantCadeirasEspera);  /* todas cadeiras de espera começam livres */
    sem_init(&totalAtingiramObjetivo, 0, 0);
    sem_init(&totalBarbeirosLiberados, 0, quantBarbeiros); /* todos barbeiros começam liberados */
    pthread_mutex_init(&mutexClienteID, NULL);
    tempoCorteCabelo = 1;
   

    // inicializa barbeiros
    for (i=0; i < quantBarbeiros; i++){
        barbeiros[i].id = i;
        barbeiros[i].quantMinimaClientes = quantMinimaClientes;
        barbeiros[i].clientesAtendidos = 0;
        barbeiros[i].totalAtingiramObjetivo = &totalAtingiramObjetivo;
        barbeiros[i].totalBarbeiros = quantBarbeiros;
        sem_init(&(barbeirosLiberado[i]), 0, 1);
        sem_init(&(barbeirosDormindo[i]), 0, 0);
        barbeiros[i].barbeiroLiberado = &(barbeirosLiberado[i]);
        barbeiros[i].barbeirosDormindo = &(barbeirosDormindo[i]);
        barbeiros[i].totalBarbeirosLiberados = &totalBarbeirosLiberados;
        barbeiros[i].tempoCorteCabelo = tempoCorteCabelo;
    }

    // inicializa cliente
    cliente->barbeirosLiberado = barbeirosLiberado;
    cliente->cadeiraEspera = &cadeiraEspera;
    cliente->barbeirosDormindo = barbeirosDormindo;
    cliente->totalBarbeiros = quantBarbeiros;
    cliente->mutexClienteID = &mutexClienteID;
    cliente->totalBarbeirosLiberados = &totalBarbeirosLiberados;
    cliente->tempoCorteCabelo = tempoCorteCabelo;


    // cria barbeiros
    for (i=0; i < quantBarbeiros; i++){
        pthread_create(&(barbeiros[i].thread), NULL, f_barbeiro, &(barbeiros[i]));
    }

    // cria cliente
    // contando semaforo
    atingiramObjetivo = 0;
    tempoEspera = 1;
    while(atingiramObjetivo < quantBarbeiros) {

        pthread_create(&(cliente->thread), NULL, f_cliente, cliente);

        if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
            atingiramObjetivo = -1;
        }
        
        #ifdef _WIN32
        Sleep(tempoEspera * 1000);
        #else
        sleep(tempoEspera);
        #endif
    }

    // join no babeiro
    for (i = 0; i < quantBarbeiros; i++) {
        pthread_join(barbeiros[i].thread, NULL);
        printf("barbeiro %d atendeu %d clientes\n", barbeiros[i].id, barbeiros[i].clientesAtendidos); 
    }

    return 0;
}



void* f_barbeiro(void* argumento) {

    barbeiro_t *barbeiro = (barbeiro_t *)argumento;

    printf("barbeiro id %d entrou\n", barbeiro->id);

    while (true) {
        sem_wait(barbeiro->barbeirosDormindo); /* barbeiro estah dormindo */
        printf("barbeiro %d atendeu um cliente! \n", barbeiro->id);
        barbeiro->clientesAtendidos++;
        #ifdef _WIN32
        Sleep(barbeiro->tempoCorteCabelo * 1000);
        #else
        sleep(barbeiro->tempoCorteCabelo);
        #endif
        sem_post(barbeiro->barbeiroLiberado); /* barbeiro estah livre do cliente */
        sem_post(barbeiro->totalBarbeirosLiberados); /* incrementa total barbeiros liberados */
    }
    
    return NULL;
}


void* f_cliente(void* argumento) {

    cliente_t *cliente = (cliente_t *)argumento;

    bool clienteAtendido = false;
    int clienteID;

    // ID do cliente
    pthread_mutex_lock(cliente->mutexClienteID);
    clienteID = CLIENTE_ULTIMO_ID;
    CLIENTE_ULTIMO_ID++;
    pthread_mutex_unlock(cliente->mutexClienteID);

    if (sem_trywait(cliente->cadeiraEspera) == 0) { /* ocupa cadeira de espera se estiver livre */
        
        printf("cliente %d entrou\n", clienteID);
        
        while (clienteAtendido == false) {
            
            sem_wait(cliente->totalBarbeirosLiberados);

            int i = (rand() % cliente->totalBarbeiros);
            int barbeirosVerificados = 0;
            
            while (barbeirosVerificados < cliente->totalBarbeiros) {

                if (sem_trywait(&(cliente->barbeirosLiberado[i])) == 0) { /* vai até o barbeiro se ele estiver livre */
                    sem_post(cliente->cadeiraEspera);  /* libera cadeira de espera */
                    sem_post(&(cliente->barbeirosDormindo[i])); /* acorda barbeiro */
                    clienteAtendido = true;
                    #ifdef _WIN32
                    Sleep(cliente->tempoCorteCabelo * 1000);
                    #else
                    sleep(cliente->tempoCorteCabelo);
                    #endif
                    printf("clinte %d atendido pelo barbeiro %d \n", clienteID, i);
                    break;
                }

                i++;
                i = (i == cliente->totalBarbeiros)? 0: i;
                barbeirosVerificados++;
            }
            
        }

        printf("cliente %d saiu\n", clienteID);

    } else {
        printf("cliente %d não entrou\n", clienteID);
    }

    return NULL;
}