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
    sem_t * cadeiraBarbeiro;
    int quantMinimaClientes;
    int clientesAtendidos;
    sem_t * totalAtingiramObjetivo;
    int totalBarbeiros;
} barbeiro_t;


typedef struct {
    pthread_t thread;
    sem_t * cadeirasBarbeiro;
    sem_t * cadeiraEspera;
    int totalBarbeiros;
    pthread_mutex_t *mutexClienteID;
    pthread_mutex_t *mutexCadeirasOcupadas;
} cliente_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {
    // a soma do total de clientes atentidos deve ser um tipo long

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, tempoEspera;
    barbeiro_t * barbeiros;
    cliente_t * cliente;
    sem_t  * cadeirasBarbeiro;
    sem_t cadeiraEspera;
    sem_t totalAtingiramObjetivo;
    pthread_mutex_t mutexClienteID;

    // validar entradas


    // inicializar VARIAVEIS
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros =  malloc(sizeof(barbeiro_t) * quantBarbeiros); 
    cadeirasBarbeiro =  malloc(sizeof(sem_t) * quantBarbeiros); 
    cliente =  malloc(sizeof(cliente_t)); 
    sem_init(&cadeiraEspera, 0, quantCadeirasEspera);
    sem_init(&totalAtingiramObjetivo, 0, 0);
    pthread_mutex_init(&mutexClienteID, NULL);
   

    // inicializa barbeiros
    for (i=0; i < quantBarbeiros; i++){
        barbeiros[i].id = i;
        barbeiros[i].quantMinimaClientes = quantMinimaClientes;
        barbeiros[i].clientesAtendidos = 0;
        barbeiros[i].totalAtingiramObjetivo = &totalAtingiramObjetivo;
        barbeiros[i].totalBarbeiros = quantBarbeiros;
        sem_init(&(cadeirasBarbeiro[i]), 0, 0);
        barbeiros[i].cadeiraBarbeiro = &(cadeirasBarbeiro[i]);
    } 

    // inicializa cliente
    cliente->cadeirasBarbeiro = cadeirasBarbeiro;
    cliente->cadeiraEspera = &cadeiraEspera;
    cliente->totalBarbeiros = quantBarbeiros;
    cliente->mutexClienteID = &mutexClienteID;


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


    // liberar memoria

    return 0;
}



void* f_barbeiro(void* argumento) {

    barbeiro_t *barbeiro = (barbeiro_t *)argumento;

    printf("barbeiro id %d entrou\n", barbeiro->id);

    while (true) {
        sem_wait(&barbeiro->cadeiraBarbeiro);
        printf("barbeiro id %d cortou cabelo! \n", barbeiro->id);
        barbeiro->clientesAtendidos++;
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

    printf("cliente %d chegou\n", clienteID);

    if (sem_trywait(cliente->cadeiraEspera) == 0) {
        
        printf("cliente %d entrou\n", clienteID);
        
        while (clienteAtendido == false) {
            
            // recebe sinal mutex

            // todas threads dormem aqui

            // tentar entrar em um indice do barbeiro

            for (int i = 0; i<cliente->totalBarbeiros; i++) {

                if (sem_trywait(&(cliente->cadeirasBarbeiro[i])) == 0) {
                    clienteAtendido = true;
                    printf("clinte %d atendido %d pelo barbeiro %d \n", clienteID, clienteAtendido, i);
                    break;
                }

            }

            // para cliente no mutex


        }

        sem_post(cliente->cadeiraEspera);

        printf("cliente %d saiu\n", clienteID);

        // envia sinal mutex 

    } else {
        printf("cliente %d não entrou\n", clienteID);
    }

    return NULL;
}