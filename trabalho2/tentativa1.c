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

typedef enum {
    false = 0,
    true = 1
} bool;


typedef struct {
    pthread_t thread;
    int id;    
    sem_t cadeiraBarbeiro;
    int quantMinimaClientes;
    int clientesAtendidos;
    sem_t * totalAtingiramObjetivo;
    int totalBarbeiros;
} barbeiro_t;


typedef struct {
    pthread_t thread;
    barbeiro_t * barbeiros;
    sem_t * cadeiraEspera;
} cliente_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {
    // a soma do total de clientes atentidos deve ser um tipo long

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, tempoEspera;
    barbeiro_t * barbeiros;
    cliente_t * cliente;
    sem_t cadeiraEspera;
    sem_t totalAtingiramObjetivo;


    // validar entradas


    // inicializar VARIAVEIS
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros =  malloc(sizeof(barbeiro_t) * quantBarbeiros); 
    cliente =  malloc(sizeof(cliente_t)); 
    sem_init(&cadeiraEspera, 0, quantCadeirasEspera);
    sem_init(&totalAtingiramObjetivo, 0, 0);
   

    // inicializa barbeiros
    for (i=0; i < quantBarbeiros; i++){
        barbeiros[i].id = i;
        barbeiros[i].quantMinimaClientes = quantMinimaClientes;
        barbeiros[i].clientesAtendidos = 0;
        barbeiros[i].totalAtingiramObjetivo = &totalAtingiramObjetivo;
        barbeiros[i].totalBarbeiros = quantBarbeiros;
        sem_init(&(barbeiros[i].cadeiraBarbeiro), 0, 0);
    } 

    // inicializa cliente
    cliente->barbeiros = barbeiros;
    cliente->cadeiraEspera = &cadeiraEspera;


    // cria barbeiros
    for (i=0; i < quantBarbeiros; i++){
        pthread_create(&(barbeiros[i].thread), NULL, f_barbeiro, &(barbeiros[i]));
    }

    // cria cliente
    // contando semaforo
    atingiramObjetivo = 0;
    tempoEspera = 1;
    while(atingiramObjetivo < quantBarbeiros) {

        if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
            atingiramObjetivo = -1;
        }

        pthread_create(&(cliente->thread), NULL, f_cliente, cliente);

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

    return NULL;
}


void* f_cliente(void* argumento) {

    cliente_t *cliente = (cliente_t *)argumento;
    
    printf("cliente chegou\n");
    if (sem_trywait(cliente->cadeiraEspera) == 0) {
        printf("cliente entrou\n");
    
        while (true) {
            

        }

        sem_post(cliente->cadeiraEspera);
        printf("cliente saiu\n");
    } else {
        printf("cliente não entrou\n");
    }

    return NULL;
}