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
    sem_t * barbeirosAcordado;
    sem_t * barbeirosAtendeuCliente;
    int quantMinimaClientes;
    int clientesAtendidos;
    sem_t * totalBarbeirosLiberados;
    sem_t * totalAtingiramObjetivo;
    int totalBarbeiros;
} barbeiro_t;


typedef struct {
    pthread_t thread;
    sem_t * barbeirosLiberado;
    sem_t * cadeiraEspera;
    sem_t * barbeirosAcordado;
    sem_t * barbeirosAtendeuCliente;    
    sem_t * totalBarbeirosLiberados;
    sem_t * totalAtingiramObjetivo;
    pthread_mutex_t * mutexClienteID;
    sem_t * barbeariaFechou;    
    sem_t * totalClientesDentroBarbearia;    
    int totalBarbeiros;
} cliente_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {
    // a soma do total de clientes atentidos deve ser um tipo long

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo;
    barbeiro_t * barbeiros;
    cliente_t * cliente;
    sem_t  * barbeirosLiberado, * barbeirosAcordado, * barbeirosAtendeuCliente;
    sem_t totalBarbeirosLiberados;
    sem_t cadeiraEspera;
    sem_t totalAtingiramObjetivo;
    sem_t barbeariaFechou;
    sem_t totalClientesDentroBarbearia;
    pthread_mutex_t mutexClienteID;


    // validar entradas


    // inicializar VARIAVEIS
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros =  malloc(sizeof(barbeiro_t) * quantBarbeiros); 
    barbeirosLiberado =  malloc(sizeof(sem_t) * quantBarbeiros); 
    barbeirosAcordado =  malloc(sizeof(sem_t) * quantBarbeiros); 
    barbeirosAtendeuCliente =  malloc(sizeof(sem_t) * quantBarbeiros); 
    cliente =  malloc(sizeof(cliente_t)); 
    sem_init(&cadeiraEspera, 0, quantCadeirasEspera);  /* todas cadeiras de espera começam livres */
    sem_init(&totalAtingiramObjetivo, 0, 0);
    sem_init(&totalBarbeirosLiberados, 0, quantBarbeiros); /* todos barbeiros começam liberados */
    sem_init(&barbeariaFechou, 0, 0);
    sem_init(&totalClientesDentroBarbearia, 0, 0);
    pthread_mutex_init(&mutexClienteID, NULL);
   

    // inicializa barbeiros
    for (i=0; i < quantBarbeiros; i++){
        barbeiros[i].id = i;
        barbeiros[i].quantMinimaClientes = quantMinimaClientes;
        barbeiros[i].clientesAtendidos = 0;
        barbeiros[i].totalAtingiramObjetivo = &totalAtingiramObjetivo;
        barbeiros[i].totalBarbeiros = quantBarbeiros;
        sem_init(&(barbeirosLiberado[i]), 0, 1);
        sem_init(&(barbeirosAcordado[i]), 0, 0);
        sem_init(&(barbeirosAtendeuCliente[i]), 0, 0);
        barbeiros[i].barbeiroLiberado = &(barbeirosLiberado[i]);
        barbeiros[i].barbeirosAcordado = &(barbeirosAcordado[i]);
        barbeiros[i].barbeirosAtendeuCliente = &(barbeirosAtendeuCliente[i]);
        barbeiros[i].totalBarbeirosLiberados = &totalBarbeirosLiberados;
    }

    // inicializa cliente
    cliente->barbeirosLiberado = barbeirosLiberado;
    cliente->cadeiraEspera = &cadeiraEspera;
    cliente->barbeirosAcordado = barbeirosAcordado;
    cliente->barbeirosAtendeuCliente = barbeirosAtendeuCliente;
    cliente->totalBarbeiros = quantBarbeiros;
    cliente->mutexClienteID = &mutexClienteID;
    cliente->totalBarbeirosLiberados = &totalBarbeirosLiberados;
    cliente->totalAtingiramObjetivo = &totalAtingiramObjetivo;
    cliente->barbeariaFechou = &barbeariaFechou;
    cliente->totalClientesDentroBarbearia = &totalClientesDentroBarbearia;


    // cria barbeiros
    for (i=0; i < quantBarbeiros; i++){
        pthread_create(&(barbeiros[i].thread), NULL, f_barbeiro, &(barbeiros[i]));
    }

    // cria cliente
    // contando semaforo
    atingiramObjetivo = 0;
    while(atingiramObjetivo < quantBarbeiros) {

        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif

        pthread_create(&(cliente->thread), NULL, f_cliente, cliente);

        if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
            atingiramObjetivo = -1;
        } 
    }

    // espera o sinal que avisa que ultimo cliente foi atentido
    printf("Espera ultimo cliente ser atendido\n");
    sem_wait(&barbeariaFechou);


    // join no babeiro
    for (i = 0; i < quantBarbeiros; i++) {
        printf("barbeiro %d atendeu %d clientes\n", barbeiros[i].id, barbeiros[i].clientesAtendidos); 
    }

    return 0;
}



void* f_barbeiro(void* argumento) {

    barbeiro_t *barbeiro = (barbeiro_t *)argumento;
    printf("barbeiro id %d entrou\n", barbeiro->id);

    while (true) {
        sem_wait(barbeiro->barbeirosAcordado); /* barbeiro estah dormindo */
        printf("barbeiro %d acordou!\n", barbeiro->id);

        barbeiro->clientesAtendidos++;

        if (barbeiro->clientesAtendidos == barbeiro->quantMinimaClientes) {
            printf("barbeiro %d atingiu seu objetivo!\n", barbeiro->id);
            sem_post(barbeiro->totalAtingiramObjetivo);
        }

        sem_post(barbeiro->barbeirosAtendeuCliente);
        printf("barbeiro %d atendeu um cliente! \n", barbeiro->id);

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

        sem_post(cliente->totalClientesDentroBarbearia);
        
        printf("cliente %d entrou\n", clienteID);
        
        
        while (clienteAtendido == false) {

            sem_wait(cliente->totalBarbeirosLiberados);

            int i = (rand() % cliente->totalBarbeiros);
            int barbeirosVerificados = 0;
            
            while (barbeirosVerificados < cliente->totalBarbeiros) {

                if (sem_trywait(&(cliente->barbeirosLiberado[i])) == 0) { /* vai até o barbeiro se ele estiver livre */
                    printf("cliente %d acorda barbeiro %d \n", clienteID, i);
                    sem_post(cliente->cadeiraEspera);  /* libera cadeira de espera */
                    sem_post(&(cliente->barbeirosAcordado[i])); /* acorda barbeiro */
                    printf("cliente %d estah esperando o barbeiro %d \n", clienteID, i);
                    sem_wait(&(cliente->barbeirosAtendeuCliente[i])); /* cliente espera ser atendido */
                    clienteAtendido = true;
                    printf("cliente %d foi atendido pelo barbeiro %d \n", clienteID, i);
                    break;
                } // fim if

                i++;
                i = (i == cliente->totalBarbeiros)? 0: i;
                barbeirosVerificados++;

            } // fim while
            
        } // fim while


        sem_wait(cliente->totalClientesDentroBarbearia);

        int esperando = 0;
        while (sem_getvalue(cliente->cadeiraEspera, &esperando) != 0); 
        printf("\nEsperando: %d\n", esperando);

        pthread_mutex_lock(cliente->mutexClienteID);

        int totalBarbeirosConcluiramObjetivo = 0;

        /* enquanto o sem_getvalue nao for bem-sucedido tente novamente */
        while (sem_getvalue(cliente->totalAtingiramObjetivo, &totalBarbeirosConcluiramObjetivo) != 0); 
        
        if (totalBarbeirosConcluiramObjetivo == cliente->totalBarbeiros) {

            int totalClientesNaBarbearia = 0;
            
            /* enquanto o sem_getvalue nao for bem-sucedido tente novamente */
            while (sem_getvalue(cliente->totalClientesDentroBarbearia, &totalClientesNaBarbearia) != 0); 
    
            if (totalClientesNaBarbearia == 0) {
                /* sinalizar na main que o ultimo cliente saiu da barbearia apos todos barbeiros atingirem o objetivo */
                sem_post(cliente->barbeariaFechou);
            }
        }

        pthread_mutex_unlock(cliente->mutexClienteID);

        printf("cliente %d saiu\n", clienteID);

    } else {
        printf("cliente %d não entrou\n", clienteID);
    }

    return NULL;
}