/* TRABALHO 2: Jackson Willian Silva Agostinho - 20172BSI0335 - jacksonwillianjbv@gmail.com */


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


/* MODO_DEBUG 
 * definido com valor 0 desativa os prints de debug,
 * definido com valor 1 ativa os prints de debug. 
 */
#define MODO_DEBUG 0      


/* MODO_ERROR
 * definido com valor 0 desativa os prints de erro,
 * definido com valor 1 ativa os prints de erro. 
 */
#define MODO_ERROR 0
  

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
    sem_t barbeariaFechou, totalClientesVisitaramBarbearia;
    pthread_mutex_t mutexClienteID, mutexUltimoCliente;
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
 
    sem_init(&cadeiraEspera, 0, quantCadeirasEspera);    /* a barbearia abre com todas cadeiras de espera livres  */ 
    sem_init(&totalAtingiramObjetivo, 0, 0);   /* contagem de barbeiros que atingiram objetivo  */ 
    sem_init(&totalBarbeirosLiberados, 0, 0);  /* contagem de barbeiros liberados (livres), isto eh, os barbeiros que podem atender algum cliente se for acordado */ 
    sem_init(&barbeariaFechou, 0, 0);    /* sinalizar a main que o programa terminou  */ 
    sem_init(&totalClientesVisitaramBarbearia, 0, 0);  /* contagem de clientes enviados para barbearia  */ 
    pthread_mutex_init(&mutexClienteID, NULL);  /* garantir que apenas um cliente pegue o ID cliente por vez  */ 
    pthread_mutex_init(&mutexUltimoCliente, NULL);  /* garantir que os clientes saiam um por vez para verificar se os barbeiros atingiram o objetivo, e o ultimo sinaliza a main que o programa terminou */ 
    pthread_attr_init(&tattr);  /* inicializa variavel atributo da thread */ 
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);  /* define atributo para detached (recursos podem ser reutilizados a medida que cada thread termina) */ 

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

        sem_init(&(barbeirosLiberado[i]), 0, 0);  /* define se um barbeiro especifico estah livre (1) ou ocupado (0) */  
        sem_init(&(barbeirosAcordado[i]), 0, 0);  /* define se um barbeiro especifico estah acordado (1) ou dormindo (0) */  
        sem_init(&(barbeirosAtendeuCliente[i]), 0, 0);   /* define se um barbeiro especifico terminou o atendimento ao cliente (1) ou ainda vai terminar (0) */  
    }

    /* cria barbeiros */
    for (i = 0; i < quantBarbeiros; i++) {

        while(pthread_create(&(barbeiros[i].thread), NULL, f_barbeiro, &(barbeiros[i])) != 0) {
            #if MODO_ERROR
            printf("\nErro ao criar thread barbeiro %d\n", i);
            #endif   

            #ifdef _WIN32
            Sleep(10);
            #else
            usleep(10);
            #endif
        }
    }

    /* inicializa cliente */
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

    /* cria clientes */
    quantThreadCriadas = 0;
    atingiramObjetivo = 0;
    while (atingiramObjetivo < quantBarbeiros) {
        
        /* incrementa a quantidade de clientes que visitou/ vai visitar a barbearia
         *  para o programa saber que ainda vai ser enviado um novo cliente
         */
        sem_post(&totalClientesVisitaramBarbearia);

        /* thread criada com a propriedade detached, nao precisa realizar join para liberar recurso,
         * os recursos sao liberados automaticamente apos o termino da thread.
         * Obs.: Nao pode realizar o join.
         */
        while (pthread_create(&(cliente->thread), &tattr, f_cliente, cliente) != 0) {
            #if MODO_ERROR
            printf("\nErro ao criar thread cliente %d\n", quantThreadCriadas);
            #endif

            #ifdef _WIN32
            Sleep(10);
            #else
            usleep(10);
            #endif
        } 

        if (sem_getvalue(&totalAtingiramObjetivo, &atingiramObjetivo) != 0) {
            atingiramObjetivo = 0;
        }

        quantThreadCriadas++;
    }


    /* espera o sinal que avisa que ultimo cliente saiu da barbearia e ela fechou */
    sem_wait(&barbeariaFechou);
    

    /* exibi a quantidade de clientes que cada barbeiro atendeu */
    for (i = 0; i < quantBarbeiros; i++) {
        printf("barbeiro %d atendeu %d clientes\n", barbeiros[i].id, barbeiros[i].clientesAtendidos);
    }
    

    /* cria pedido de cancelamento das threads barbeiros */
    for (i = 0; i < quantBarbeiros; i++) {
        while(pthread_cancel(barbeiros[i].thread) != 0) {
            #if MODO_ERROR
            printf("\nErro ao criar pedido de cancelamento da thread barbeiro %d\n", i);
            #endif
            
            #ifdef _WIN32
            Sleep(10);
            #else
            usleep(10);
            #endif
        }
    }


    /* acorda barbeiros para eles cairem no ponto de cancelamento */
    for (i = 0; i < quantBarbeiros; i++) {
        sem_post(barbeiros[i].barbeirosAcordado);
    }


    /* libera recursos dos barbeiros */
    void *status=NULL;
    for (i = 0; i < quantBarbeiros; i++) {

        /* quando uma thread cancelada eh encerrada, uma junção usando pthread_join()
         * obtem PTHREAD_CANCELED como status de saida.
         * https://man7.org/linux/man-pages/man3/pthread_join.3.html
         */
        while(pthread_join(barbeiros[i].thread, &status) != 0) {
            #if MODO_ERROR
            printf("\nErro ao unir thread barbeiro %d\n", i);
            #endif   

            #ifdef _WIN32
            Sleep(10);
            #else
            usleep(10);
            #endif
        }

        #if MODO_DEBUG
        if (status == PTHREAD_CANCELED) {
            printf("thread barbeiro %d cancelada\n", i);
        }
        #endif
    }

    /* destroi variaveis e libera memoria */
    pthread_attr_destroy(&tattr);
    for(i = 0; i < quantBarbeiros; i++) {
        sem_destroy(&(barbeirosLiberado[i]));
        sem_destroy(&(barbeirosAcordado[i]));
        sem_destroy(&(barbeirosAtendeuCliente[i]));
    }
    sem_destroy(&barbeariaFechou);
    sem_destroy(&cadeiraEspera);
    sem_destroy(&totalBarbeirosLiberados);
    sem_destroy(&totalAtingiramObjetivo);
    sem_destroy(&totalClientesVisitaramBarbearia);
    pthread_mutex_destroy(&mutexClienteID);
    pthread_mutex_destroy(&mutexUltimoCliente);
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

    sem_post(barbeiro->barbeiroLiberado); /* barbeiro chegou e estah livre */
    sem_post(barbeiro->totalBarbeirosLiberados); /* incrementa total barbeiros liberados (livres) */

    while (true) {

        sem_wait(barbeiro->barbeirosAcordado); /* barbeiro estah dormindo */

        /* ponto de cancelamento 
         * se nenhum pedido de cancelamento está pendente, então uma chamada para
         * pthread_testcancel() não tem efeito. 
         * https://man7.org/linux/man-pages/man3/pthread_testcancel.3.html 
         */
        pthread_testcancel(); 

        #if MODO_DEBUG
        printf("barbeiro %d acordou e estah atendendo um cliente!\n", barbeiro->id);
        #endif
        
        barbeiro->clientesAtendidos++;

        if (barbeiro->clientesAtendidos == barbeiro->quantMinimaClientes) {
            #if MODO_DEBUG
            printf("barbeiro %d atingiu seu objetivo!\n", barbeiro->id);
            #endif

            sem_post(barbeiro->totalAtingiramObjetivo);
        }

        #if MODO_DEBUG
        printf("barbeiro %d terminou de atender o cliente! \n", barbeiro->id);
        #endif

        sem_post(barbeiro->barbeirosAtendeuCliente); /* barbeiro terminou de atender o cliente */

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
                    printf("cliente %d acorda o barbeiro %d e estah esperando por ele\n", clienteID, i);
                    #endif
                    sem_post(cliente->cadeiraEspera);  /* libera cadeira de espera */
                    sem_post(&(cliente->barbeirosAcordado[i])); /* acorda barbeiro */
                    sem_wait(&(cliente->barbeirosAtendeuCliente[i])); /* cliente espera, na cadeira do barbeiro, o fim do atendimento */
                    clienteAtendido = true;
                    #if MODO_DEBUG
                    printf("cliente %d foi atendido pelo barbeiro %d e jah estah saindo \n", clienteID, i);
                    #endif
                    break;
                } // fim if

                i++;
                i = (i >= cliente->totalBarbeiros) ? 0 : i;
                barbeirosVerificados++;

            } // fim while

        } // fim while

    }
    else {
        #if MODO_DEBUG
        printf("cliente %d nao entrou\n", clienteID);
        #endif
    }

    sem_wait(cliente->totalClientesVisitaramBarbearia);

    /*  mutex para garantir que os clientes saiam um por vez para verificar se os barbeiros atingiram o objetivo,
     *   e o ultimo sinaliza a main que o programa terminou
     */
    pthread_mutex_lock(cliente->mutexUltimoCliente);

    int totalBarbeirosConcluiramObjetivo = 0;
    /* obtem a quantidade de barbeiros que ja atingiram o objetivo */
    while (sem_getvalue(cliente->totalAtingiramObjetivo, &totalBarbeirosConcluiramObjetivo) != 0) {
        #ifdef _WIN32
        Sleep(10);
        #else
        usleep(10);
        #endif
    }

    if (totalBarbeirosConcluiramObjetivo == cliente->totalBarbeiros) {

        int totalClientesVisitaram = 0;
        /* obtem a quantidade de clientes que visitaram/entraram */
        while (sem_getvalue(cliente->totalClientesVisitaramBarbearia, &totalClientesVisitaram) != 0) {
            #ifdef _WIN32
            Sleep(10);
            #else
            usleep(10);
            #endif
        }

        if (totalClientesVisitaram == 0) {
            /* sinalizar na main que saiu o ultimo cliente que entrou/visitou a barbearia apos todos barbeiros atingirem o objetivo */
            sem_post(cliente->barbeariaFechou);
        }
    }

    pthread_mutex_unlock(cliente->mutexUltimoCliente);

    return NULL;
}