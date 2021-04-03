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
    int quantMinimaClientes;
    int clientesAtendidos;
    sem_t * totalAtingiramObjetivo;
    sem_t * quantCadeirasEsperaDesocupada;
    sem_t * cadeirasEspera;
    sem_t * clienteAguardando;
    int totalBarbeiros;
    int totalCadeirasEspera;
} barbeiro_t;


typedef struct {
    pthread_t thread;
    sem_t * quantCadeirasEsperaDesocupada;
    sem_t * cadeirasEspera;
    sem_t * clienteAguardando;
    int totalCadeirasEspera;
    pthread_mutex_t *mutexClienteID;
} cliente_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {
    // a soma do total de clientes atentidos deve ser um tipo long

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, tempoEspera;
    barbeiro_t * barbeiros;
    cliente_t * cliente;
    sem_t quantCadeirasEsperaDesocupada; /* representa total de cadeiras de espera desocupadas */
    sem_t * cadeirasEspera; /* representa cada uma das cadeiras de espera dentro da barbearia */
    sem_t * clienteAguardando; 
    sem_t totalAtingiramObjetivo;
    pthread_mutex_t mutexClienteID; /* usado apenas para debug, garantir ID unico do cliente */

    // validar entradas


    // inicializar VARIAVEIS
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros =  malloc(sizeof(barbeiro_t) * quantBarbeiros);
    cadeirasEspera =  malloc(sizeof(sem_t) * quantCadeirasEspera); 
    clienteAguardando =  malloc(sizeof(sem_t) * quantCadeirasEspera); 
    cliente =  malloc(sizeof(cliente_t)); 
    sem_init(&quantCadeirasEsperaDesocupada, 0, quantCadeirasEspera);  /* Todas cadeiras de espera começam desocupadas */
    sem_init(&totalAtingiramObjetivo, 0, 0);
    pthread_mutex_init(&mutexClienteID, NULL);
   
    // inicializa cadeira de espera
    for (i=0; i < quantCadeirasEspera; i++) {
        // cadeiras de espera começam desocupadas, isto eh, tem valor um
        sem_init(&(cadeirasEspera[i]), 0, 1);
        sem_init(&(clienteAguardando[i]), 0, 0);
    }

    // inicializa barbeiros
    for (i=0; i < quantBarbeiros; i++){
        barbeiros[i].id = i;
        barbeiros[i].quantMinimaClientes = quantMinimaClientes;
        barbeiros[i].clientesAtendidos = 0;
        barbeiros[i].totalAtingiramObjetivo = &totalAtingiramObjetivo;
        barbeiros[i].totalBarbeiros = quantBarbeiros;
        barbeiros[i].cadeirasEspera = cadeirasEspera;
        barbeiros[i].quantCadeirasEsperaDesocupada = &quantCadeirasEsperaDesocupada;
        barbeiros[i].totalCadeirasEspera = quantCadeirasEspera;
        barbeiros[i].clienteAguardando = clienteAguardando;
    } 

    // inicializa cliente
    cliente->quantCadeirasEsperaDesocupada = &quantCadeirasEsperaDesocupada;
    cliente->cadeirasEspera = cadeirasEspera;
    cliente->mutexClienteID = &mutexClienteID;
    cliente->clienteAguardando = clienteAguardando;
    cliente->totalCadeirasEspera = quantCadeirasEspera;



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
    int tempoEspera = 5;
    while (true) {
        
        if (sem_trywait(barbeiro->quantCadeirasEsperaDesocupada) == 0) { /* barbeiro verifica se ha clientes ocupando cadeiras de espera */
            // semaforo cadeiras de espera
            for (int i = 0; i < barbeiro->totalCadeirasEspera; i++) {

                if (sem_trywait(&(barbeiro->cadeirasEspera[i])) == 0) { /* verifica qual cadeira tem cliente para atender */
                    
                    
                }
            }

            // semaforo cortando cabelo
        } else {
            /* Não há clientes, barbeiro dorme enquanto aguarda */
            sem_wait(barbeiro->quantCadeirasEsperaDesocupada);
        }

        printf("barbeiro %d cortou cabelo! \n", barbeiro->id);
        barbeiro->clientesAtendidos++;
        #ifdef _WIN32
        Sleep(tempoEspera * 1000);
        #else
        sleep(tempoEspera);
        #endif
    }
    

    return NULL;
}


void* f_cliente(void* argumento) {

    cliente_t *cliente = (cliente_t *)argumento;

    int clienteID;

    // ID do cliente usado apenas para debug
    pthread_mutex_lock(cliente->mutexClienteID);
    clienteID = CLIENTE_ULTIMO_ID;
    CLIENTE_ULTIMO_ID++;
    pthread_mutex_unlock(cliente->mutexClienteID);

    if (sem_trywait(cliente->quantCadeirasEsperaDesocupada) == 0) { /* tenta ocupar uma cadeira de espera */
        
        printf("cliente %d entrou\n", clienteID);
        
        for(int i = 0; i < cliente->totalCadeirasEspera; i++) {
            if (sem_trywait(&(cliente->cadeirasEspera[i])) == 0) { /* cliente tenta ocupar uma cadeira de espera */
                /* cliente ocupa uma cadeira de espera e sinaliza que estah aguardando */
                sem_post(&(cliente->clienteAguardando[i])); 
                /* cliente permanece sentado na cadeira de espera enquanto nao eh atendido */   
                sem_wait(&(cliente->cadeirasEspera[i]));
                /* cliente foi chamado para ser atendido e senta na cadeira do barbeiro */
                sem_post(&(cliente->cadeirasEspera[i])); // libera cadeira de espera
                /* cliente estar sendo atendido e aguarda o barbeiro terminar de cortar seu cabelo */
                sem_wait(&(cliente->clienteAguardando[i]));     
                break;
            }
        }

        printf("cliente %d saiu\n", clienteID);

    } else {
        printf("cliente %d não entrou\n", clienteID);
    }

    return NULL;
}