/* TRABALHO 2: Jackson Willian Silva Agostinho - 20172BSI0335 - jacksonwillianjbv@gmail.com */

#define _GNU_SOURCE

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/* MODO_DEBUG definido com valor 0 desativa os prints de debug, e definido com valor 1 ativa os prints de debug */
#define MODO_DEBUG 0           

/* contagem do id dos clientes usado somente para debug */
int CLIENTE_ULTIMO_ID = 0;

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
    int totalBarbeiros;
    sem_t* totalAtingiramObjetivo;
    pthread_mutex_t* mutexClienteID;
    pthread_mutex_t* mutexUltimoCliente;
    sem_t* barbeariaFechou;
    sem_t* totalClientesVisitaramBarbearia;   
} cliente_t;


void* f_barbeiro(void* argumento);

void* f_cliente(void* argumento);


int main(int argc, char** argv) {

    int i, quantBarbeiros, quantCadeirasEspera, quantMinimaClientes, atingiramObjetivo, quantThreadCriadas;
    barbeiro_t* barbeiros;
    cliente_t* cliente;
    sem_t* barbeirosLiberado, * barbeirosAcordado, * barbeirosAtendeuCliente;
    sem_t cadeiraEspera, totalBarbeirosLiberados, totalAtingiramObjetivo;
    pthread_mutex_t mutexClienteID, mutexUltimoCliente;
    sem_t barbeariaFechou, totalClientesVisitaramBarbearia;
    pthread_attr_t tattr;

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
    barbeiros = (barbeiro_t*)malloc(sizeof(barbeiro_t) * quantBarbeiros);
    barbeirosLiberado = (sem_t*)malloc(sizeof(sem_t) * quantBarbeiros);
    barbeirosAcordado =  (sem_t*)malloc(sizeof(sem_t) * quantBarbeiros);
    barbeirosAtendeuCliente =  (sem_t*)malloc(sizeof(sem_t) * quantBarbeiros);
    cliente = (cliente_t*)malloc(sizeof(cliente_t));
 
    while (sem_init(&cadeiraEspera, 0, quantCadeirasEspera) != 0) {      /* a barbearia abre com todas cadeiras de espera livres */
        printf("\nErro ao inicializar semaforo\n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }
    while (sem_init(&totalAtingiramObjetivo, 0, 0) != 0) {               /* contagem de barbeiros que atingiram objetivo  */
        printf("\nErro ao inicializar semaforo\n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }
    while (sem_init(&totalBarbeirosLiberados, 0, quantBarbeiros) != 0) {  /* contagem de barbeiros liberados (livres), isto eh, os barbeiros que podem atender algum cliente se for acordado */
        printf("\nErro ao inicializar semaforo\n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }
    while (sem_init(&barbeariaFechou, 0, 0) != 0) {                       /* sinalizar a main que o programa terminou */
        printf("\nErro ao inicializar semaforo\n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }
    while (sem_init(&totalClientesVisitaramBarbearia, 0, 0) != 0) {         /* contagem de cliente dentro da barbearia */
        printf("\nErro ao inicializar semaforo\n");
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
    }
    while (pthread_mutex_init(&mutexClienteID, NULL) != 0) {             /* garantir que apenas um cliente pegue o ID cliente por vez */
        printf("\nErro ao inicializar mutex\n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }
    while (pthread_mutex_init(&mutexUltimoCliente, NULL) != 0) {         /* garantir que os clientes saiam um por vez para verificar se os barbeiros atingiram o objetivo, e o ultimo sinaliza a main que o programa terminou */
        printf("\nErro ao inicializar mutex\n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }

    /* inicializa variavel atributo thread */
    while (pthread_attr_init(&tattr) != 0) {
        printf("Erro ao inicializar o thread atributo \n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }

    /* define atributo para detached (recursos podem ser reutilizados a medida que cada thread termina) */
    while (pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED) != 0) {
        printf("Erro ao definir atributo da thread \n");
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
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

        while (sem_init(&(barbeirosLiberado[i]), 0, 1) != 0) {            /* define se um barbeiro especifico estah livre (1) ou ocupado (0) */
            printf("\nErro ao inicializar semaforo\n");
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
        }
        while (sem_init(&(barbeirosAcordado[i]), 0, 0) != 0) {            /* define se um barbeiro especifico estah acordado (1) ou dormindo (0) */
            printf("\nErro ao inicializar semaforo\n");
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
        }
        while (sem_init(&(barbeirosAtendeuCliente[i]), 0, 0) != 0) {      /* define se um barbeiro especifico terminou o atendimento ao cliente (1) ou ainda vai terminar (0) */
            printf("\nErro ao inicializar semaforo\n");
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
        }
    }

    /* cria barbeiros */
    for (i = 0; i < quantBarbeiros; i++) {
        while(pthread_create(&(barbeiros[i].thread), NULL, f_barbeiro, &(barbeiros[i])) != 0) {
            printf("\nErro ao criar thread barbeiro %d\n", i);
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
        }
    }


    // inicializa cliente
    cliente->barbeirosLiberado = barbeirosLiberado;
    cliente->cadeiraEspera = &cadeiraEspera;
    cliente->barbeirosAcordado = barbeirosAcordado;
    cliente->barbeirosAtendeuCliente = barbeirosAtendeuCliente;
    cliente->totalBarbeiros = quantBarbeiros;
    cliente->mutexClienteID = &mutexClienteID;
    cliente->mutexUltimoCliente = &mutexUltimoCliente;
    cliente->totalBarbeirosLiberados = &totalBarbeirosLiberados;
    cliente->totalAtingiramObjetivo = &totalAtingiramObjetivo;
    cliente->barbeariaFechou = &barbeariaFechou;
    cliente->totalClientesVisitaramBarbearia = &totalClientesVisitaramBarbearia;

    quantThreadCriadas = 0;
    atingiramObjetivo = 0;
    while (atingiramObjetivo < quantBarbeiros) {
        
        /* incrementa a quantidade de clientes que vai visitar/ visitou a barbearia
           para o programa saber que ainda vai ser enviado um novo cliente
        */
        sem_post(&totalClientesVisitaramBarbearia);

        while (pthread_create(&(cliente->thread), &tattr, f_cliente, cliente) != 0) {
            printf("\nErro ao criar thread cliente %d\n", quantThreadCriadas);
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
        } 

        if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
            atingiramObjetivo = 0;
        }

        quantThreadCriadas++;
    }

    // espera o sinal que avisa que ultimo cliente saiu da barbearia
    sem_wait(&barbeariaFechou);
    

    /* exibi a quantidade de clientes que cada barbeiro atendeu */
    for (i = 0; i < quantBarbeiros; i++) {
        printf("barbeiro %d atendeu %d clientes\n", barbeiros[i].id, barbeiros[i].clientesAtendidos);
    }

    /* cancela threads barbeiro */
    // https://wiki.sei.cmu.edu/confluence/display/c/POS44-C.+Do+not+use+signals+to+terminate+threads
    for (i = 0; i < quantBarbeiros; i++) {
        while(pthread_cancel(barbeiros[i].thread) != 0) {
            printf("\nErro ao cancelar thread barbeiro %d\n", i);
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
        }
    }


    /* ponto de cancelamento */
    for (i = 0; i < quantBarbeiros; i++) {
        while(pthread_tryjoin_np(barbeiros[i].thread, NULL) != 0) {
            pthread_testcancel();
            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif
        }
    }

    /* libera memoria */
    pthread_attr_destroy(&tattr);
    for(i = 0; i < quantBarbeiros; i++) {
        sem_destroy(&(barbeirosLiberado[i]));
        sem_destroy(&(barbeirosAcordado[i]));
        sem_destroy(&(barbeirosAtendeuCliente[i]));
    }
    sem_destroy(&totalBarbeirosLiberados);
    sem_destroy(&totalAtingiramObjetivo);
    free(barbeiros);
    free(cliente);
    free(barbeirosLiberado);
    free(barbeirosAcordado);
    free(barbeirosAtendeuCliente);
    
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

    #if MODO_DEBUG
        int clienteID;
        /* mutex para garantir que apenas um cliente pegue o ID cliente por vez */
        pthread_mutex_lock(cliente->mutexClienteID);
        clienteID = CLIENTE_ULTIMO_ID;
        CLIENTE_ULTIMO_ID++;
        pthread_mutex_unlock(cliente->mutexClienteID);
    #endif  

    if (sem_trywait(cliente->cadeiraEspera) == 0) { /* ocupa cadeira de espera se alguma estiver livre */

        #if MODO_DEBUG
        printf("cliente %d entrou\n", clienteID);
        #endif

        while (clienteAtendido == false) {

            sem_wait(cliente->totalBarbeirosLiberados); /* caso não tenha barbeiros livres o cliente vai esperar aqui */

            srand(time(NULL)); // muda semente do rand() 
            i = (rand() % cliente->totalBarbeiros); // indice inicial
            barbeirosVerificados = 0;

            while (barbeirosVerificados < cliente->totalBarbeiros) {

                if (sem_trywait(&(cliente->barbeirosLiberado[i])) == 0) { /* vai ao encontro do barbeiro livre */
                    #if MODO_DEBUG
                    printf("cliente %d acorda barbeiro %d \n", clienteID, i);
                    #endif
                    sem_post(cliente->cadeiraEspera);  /* libera cadeira de espera */
                    sem_post(&(cliente->barbeirosAcordado[i])); /* acorda barbeiro */
                    #if MODO_DEBUG
                    printf("cliente %d estah esperando o barbeiro %d \n", clienteID, i);
                    #endif
                    sem_wait(&(cliente->barbeirosAtendeuCliente[i])); /* cliente espera, na cadeira do barbeiro, o fim do atendimento */
                    clienteAtendido = true;
                    #if MODO_DEBUG
                    printf("cliente %d foi atendido pelo barbeiro %d \n", clienteID, i);
                    #endif
                    break;
                } // fim if

                i++;
                i = (i >= cliente->totalBarbeiros) ? 0 : i;
                barbeirosVerificados++;

            } // fim while

        } // fim while

        #if MODO_DEBUG
        printf("cliente %d saiu\n", clienteID);
        #endif
    }
    else {
        #if MODO_DEBUG
        printf("cliente %d nao entrou\n", clienteID);
        #endif
    }


    /*
        mutex para garantir que os clientes saiam um por vez para verificar se os barbeiros atingiram o objetivo,
        e o ultimo sinaliza a main que o programa terminou
    */

    sem_wait(cliente->totalClientesVisitaramBarbearia);

    pthread_mutex_lock(cliente->mutexUltimoCliente);

    int totalBarbeirosConcluiramObjetivo = 0;

    /* obtem a quantidade de barbeiros que ja atingiram o objetivo */
    while (true) {

        if (sem_getvalue(cliente->totalAtingiramObjetivo, &totalBarbeirosConcluiramObjetivo) == 0) {
            break;
        }

        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }

    if (totalBarbeirosConcluiramObjetivo == cliente->totalBarbeiros) {

        int totalClientesVisitaram = 0;

        /* obtem a quantidade de clientes que visitaram/entraram */
        while (true) {
            if (sem_getvalue(cliente->totalClientesVisitaramBarbearia, &totalClientesVisitaram) == 0) {
                break;
            }

            #ifdef _WIN32
            Sleep(1000);
            #else
            sleep(1);
            #endif

        }

        if (totalClientesVisitaram == 0) {
            /* sinalizar na main que o ultimo cliente que entrou/visitou da barbearia apos todos barbeiros atingirem o objetivo */
            sem_post(cliente->barbeariaFechou);
        }
    }

    pthread_mutex_unlock(cliente->mutexUltimoCliente);

    return NULL;
}