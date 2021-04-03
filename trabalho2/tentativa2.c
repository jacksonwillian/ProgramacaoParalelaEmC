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
    sem_t * totalClienteSemAtender;
    sem_t * cadeirasEspera;
    sem_t * clienteAguardando;
    int totalBarbeiros;
    int totalCadeirasEspera;
} barbeiro_t;


typedef struct {
    pthread_t thread;
    sem_t * quantCadeirasEsperaDesocupada;
    sem_t * totalClienteSemAtender;
    sem_t * cadeirasEspera;
    sem_t * clienteAguardando;
    int totalCadeirasEspera;
    pthread_mutex_t * mutexClienteID;
    pthread_mutex_t * cortouCabelo;
} cliente_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {
    // a soma do total de clientes atentidos deve ser um tipo long

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, tempoEspera;
    barbeiro_t * barbeiros;
    cliente_t * cliente;
    sem_t quantCadeirasEsperaDesocupada; /* representa total de cadeiras de espera desocupadas */
    sem_t totalClienteSemAtender; /* representa total de cadeiras de espera desocupadas */
    sem_t * cadeirasEspera; /* representa cada uma das cadeiras de espera dentro da barbearia */
    sem_t * clienteAguardando; 
    sem_t totalAtingiramObjetivo;
    pthread_mutex_t * cortouCabelo;
    pthread_mutex_t mutexClienteID; /* usado apenas para debug, garantir ID unico do cliente */

    // validar entradas


    // inicializar VARIAVEIS
    quantBarbeiros = atoi(argv[1]);
    quantCadeirasEspera = atoi(argv[2]);
    quantMinimaClientes = atoi(argv[3]);
    barbeiros =  malloc(sizeof(barbeiro_t) * quantBarbeiros);
    cadeirasEspera =  malloc(sizeof(sem_t) * quantCadeirasEspera); 
    clienteAguardando =  malloc(sizeof(sem_t) * quantCadeirasEspera); 
    cortouCabelo =  malloc(sizeof(pthread_mutex_t) * quantCadeirasEspera); 
    cliente =  malloc(sizeof(cliente_t)); 
    sem_init(&quantCadeirasEsperaDesocupada, 0, quantCadeirasEspera);  /* todas cadeiras de espera começam desocupadas */
    sem_init(&totalClienteSemAtender, 0, 0);  /* a barbearia começa com nenhum cliente sem atender */
    sem_init(&totalAtingiramObjetivo, 0, 0);
    pthread_mutex_init(&mutexClienteID, NULL);
    
   
    // inicializa cadeira de espera
    for (i=0; i < quantCadeirasEspera; i++) {
        // cadeiras de espera começam desocupadas, isto eh, tem valor um
        sem_init(&(cadeirasEspera[i]), 0, 1);
        sem_init(&(clienteAguardando[i]), 0, 0);
        pthread_mutex_init(&(cortouCabelo[i]), NULL);
    }

    // inicializa barbeiros
    for (i=0; i < quantBarbeiros; i++){
        barbeiros[i].id = i;
        barbeiros[i].quantMinimaClientes = quantMinimaClientes;
        barbeiros[i].clientesAtendidos = 0;
        barbeiros[i].totalAtingiramObjetivo = &totalAtingiramObjetivo;
        barbeiros[i].totalBarbeiros = quantBarbeiros;
        barbeiros[i].cadeirasEspera = cadeirasEspera;
        barbeiros[i].totalClienteSemAtender = &totalClienteSemAtender;
        barbeiros[i].totalCadeirasEspera = quantCadeirasEspera;
        barbeiros[i].clienteAguardando = clienteAguardando;
    } 

    // inicializa cliente
    cliente->quantCadeirasEsperaDesocupada = &quantCadeirasEsperaDesocupada;
    cliente->totalClienteSemAtender = &totalClienteSemAtender;
    cliente->cadeirasEspera = cadeirasEspera;
    cliente->mutexClienteID = &mutexClienteID;
    cliente->clienteAguardando = clienteAguardando;
    cliente->totalCadeirasEspera = quantCadeirasEspera;
    cliente->cortouCabelo = cortouCabelo;



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
    int i;

    while (true) {
    
        sem_wait(barbeiro->totalClienteSemAtender);
        printf("barbeiro %d estah acordado! \n", barbeiro->id);
        
        i = 0;
        while (true) {

            // printf("barbeiro %d verifica cada cliente! \n", barbeiro->id);

            if (sem_trywait(&(barbeiro->clienteAguardando[i])) == 0) { /* verifica qual cliente não foi atendido */
                printf("barbeiro %d chama cliente da cadeira %d! \n", barbeiro->id, i);
                /* barbeiro chama o cliente que não foi atendido para sentar na sua cadeira */

                #ifdef _WIN32
                Sleep(1 * 1000);
                #else
                sleep(1);
                #endif
                printf("barbeiro %d atendeu um cliente que estava cadeira %d! \n", barbeiro->id, i);
                sem_post(&(barbeiro->cadeirasEspera[i]));  // libera uma das cadeiras de esper a
                barbeiro->clientesAtendidos++;
                break;
            }

            i++;
            i = (i == barbeiro->totalCadeirasEspera) ? 0 : i;
        }

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

        sem_post(cliente->totalClienteSemAtender);

        int i = 0;
        while( true ) {

            if (sem_trywait(&(cliente->cadeirasEspera[i])) == 0) { /* cliente tenta ocupar uma cadeira de espera */
                printf("cliente %d estar aguardando na cadeira %d\n", clienteID, i);
                /* cliente ocupa uma cadeira de espera e sinaliza que não foi atendido */
                sem_post(&(cliente->clienteAguardando[i])); 
                
                /* cliente vai permanecer sentado na cadeira de espera enquanto nao for atendido */   
                
                sem_wait(&(cliente->cadeirasEspera[i]));
                printf("cliente %d que estava na cadeira %d foi atendido! \n", clienteID, i);
                sem_post(&(cliente->cadeirasEspera[i]));


                break;
            }

            i++;
            i = (i == cliente->totalCadeirasEspera) ? 0 : i;
        }

        sem_post(cliente->quantCadeirasEsperaDesocupada);
        printf("cliente %d saiu da barbearia\n", clienteID);

    } else {
        printf("cliente %d não entrou na barbearia!\n", clienteID);
    }

    return NULL;
}