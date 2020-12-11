#include <pthread.h> 
#include <semaphore.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

#define QUANT_INFECTADOS 3
#define QUANT_LABORATORIOS 3
#define TAMANHO_REPOSITORIO 3

#define INSUMO_INFINITO -1
#define INSUMO_INDISPONIVEL -1 


typedef struct {
    int virus;
    int injecao;
    int elementoSecreto;
} repositorio_t;


typedef struct {
    pthread_t thread;
    int id;  
    repositorio_t *repositorio;
    int ciclosMinimos;                      // quantidade minima de ciclos de producao
    int *atingiramObjetivo;                 // total que atingiram a quantidade minima de ciclos  
    sem_t *objetivoMutex;           
    pthread_mutex_t *bancadaMutex;             
    pthread_cond_t *condicionalLaboratorio;
    pthread_cond_t *condicionalInfectado;                
} laboratorio_t;


typedef struct {
    pthread_t thread;
    int id;
    repositorio_t repositorio;
    repositorio_t *bancada;                 // referencia para os repositorios dos laboratorios
    int ciclosMinimos;                      // quantidade minima de ciclos de consumo
    int *atingiramObjetivo;                 // total que atingiram a quantidade minima de ciclos 
    sem_t *objetivoMutex;
    pthread_mutex_t *bancadaMutex;    
    pthread_cond_t *condicionalLaboratorio;    
    pthread_cond_t *condicionalInfectado;    
} infectado_t;



// Permite que os laboratorios produzam os insumos
void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    int ciclosAtualProducao = 0;
    int quantEmEstoque = 0;
    int continuarOperando = TRUE;
    
    printf("\nLABORATORIO ID [%d]\n", laboratorio->id); 

    while (continuarOperando == TRUE) {

        pthread_mutex_lock(laboratorio->bancadaMutex);

        quantEmEstoque = 0;

        if (laboratorio->repositorio->virus != INSUMO_INDISPONIVEL) quantEmEstoque += laboratorio->repositorio->virus;
        if (laboratorio->repositorio->injecao != INSUMO_INDISPONIVEL) quantEmEstoque += laboratorio->repositorio->injecao;
        if (laboratorio->repositorio->elementoSecreto != INSUMO_INDISPONIVEL) quantEmEstoque += laboratorio->repositorio->elementoSecreto;
        
        if (quantEmEstoque == 0) {
            if (laboratorio->repositorio->virus != INSUMO_INDISPONIVEL) laboratorio->repositorio->virus = 1;
            if (laboratorio->repositorio->injecao != INSUMO_INDISPONIVEL) laboratorio->repositorio->injecao = 1;
            if (laboratorio->repositorio->elementoSecreto != INSUMO_INDISPONIVEL) laboratorio->repositorio->elementoSecreto = 1;

            ciclosAtualProducao += 1; 

            if (ciclosAtualProducao == laboratorio->ciclosMinimos) *(laboratorio->atingiramObjetivo) += 1; 
           
            if (*(laboratorio->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS)) continuarOperando = FALSE;    
        
            pthread_cond_broadcast(laboratorio->condicionalInfectado);
        }
        
        if (continuarOperando == TRUE) {
            printf("\nLABORATORIO ID [%d] estah aguardando...\n", laboratorio->id);
            while ( pthread_cond_wait (laboratorio->condicionalLaboratorio, laboratorio->bancadaMutex) != 0 );
            printf("\nLABORATORIO ID [%d] recebeu um sinal...\n", laboratorio->id);
        }
                     
        pthread_mutex_unlock(laboratorio->bancadaMutex);
    
        sleep(3);
    }
    
    printf("\nLABORATORIO ID [%d] fechou\n", laboratorio->id);

    pthread_cond_broadcast(laboratorio->condicionalInfectado);

    return NULL;
}


// Permite que os infectados consumam os insumos
void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;

    int ciclosAtualConsumo = 0;
    int quantInsumoFaltante = 0;
    int i = 0;
    int continuarOperando = TRUE;
    
    printf("\nINFECTADO ID [%d]\n", infectado->id); 

    while (continuarOperando == TRUE) {
        

        // consumir em par

        pthread_mutex_lock(infectado->bancadaMutex);

        if (infectado->repositorio.virus != INSUMO_INFINITO) infectado->repositorio.virus = 0;
        if (infectado->repositorio.injecao != INSUMO_INFINITO) infectado->repositorio.injecao = 0;
        if (infectado->repositorio.elementoSecreto != INSUMO_INFINITO) infectado->repositorio.elementoSecreto = 0;
        
        quantInsumoFaltante = 2;

        i = rand() % QUANT_LABORATORIOS;

        while (quantInsumoFaltante > 0) {

            if ( infectado->bancada[i].virus != INSUMO_INDISPONIVEL && infectado->repositorio.virus == 0 ) {
                infectado->bancada[i].virus = 0;
                infectado->repositorio.virus = 1;
                quantInsumoFaltante -= 1;
            }

            if ( infectado->bancada[i].injecao != INSUMO_INDISPONIVEL && infectado->repositorio.injecao == 0 ) {
                infectado->bancada[i].injecao = 0;
                infectado->repositorio.injecao = 1;
                quantInsumoFaltante -= 1;
            }

            if ( infectado->bancada[i].elementoSecreto != INSUMO_INDISPONIVEL && infectado->repositorio.elementoSecreto == 0 ) {
                infectado->bancada[i].elementoSecreto = 0;
                infectado->repositorio.elementoSecreto = 1;
                quantInsumoFaltante -= 1;
            }            

            i += 1;
            i = i % QUANT_LABORATORIOS; 
            
            printf("\nINFECTADO [%d] esta consumindo no while \n", infectado->id);
        }

        ciclosAtualConsumo += 1;

        if (ciclosAtualConsumo == infectado->ciclosMinimos) *(infectado->atingiramObjetivo) += 1; 
        
        if (*(infectado->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS)) continuarOperando = FALSE;    
    
        pthread_cond_broadcast(infectado->condicionalLaboratorio);
        
        if (continuarOperando == TRUE) {
            printf("\nINFECTADO ID [%d] estah aguardando...\n", infectado->id);
            while ( pthread_cond_wait (infectado->condicionalInfectado, infectado->bancadaMutex) != 0 );
            printf("\nINFECTADO ID [%d] recebeu um sinal...\n", infectado->id);
        }
                     
        pthread_mutex_unlock(infectado->bancadaMutex);
    
        sleep(3);

    }
    
    printf("\nINFECTADO ID [%d] saiu\n", infectado->id);

    pthread_cond_broadcast(infectado->condicionalLaboratorio);

    return NULL;}



int main(int argc, char** argv) {

    /* DECLARACAO DAS VARIAVEIS */
    int i, ciclosMinimos, atigiramObjetivo, posicaoInsumoInfinito, posicaoInsumoIndisponivel;
    repositorio_t *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    sem_t objetivoMutex;
    pthread_mutex_t bancadaMutex;
    pthread_cond_t condicionalLaboratorio, condicionalInfectado;


    /* VALIDACAO DE ENTRADAS */
    if ( argc != 2 ) {
        printf("\nQuantidades de argumentos são inválidos!\n");
        printf("\nPrimeiro argumento eh numero de vezes MINIMO que cada um realizou seu objetivo primordial.\n");
        return -1;
    } else if ( atoi(argv[1]) < 1) {
        printf("\nValor do argumento eh inválido!\n");
        return -1;
    }   
 

    /* INICIALIZACAO DAS VARIAVEIS */    
    ciclosMinimos = atoi(argv[1]);
    atigiramObjetivo = 0;
    posicaoInsumoIndisponivel = 0;
    posicaoInsumoInfinito = 0;
    laboratorios = malloc(sizeof(laboratorio_t) * QUANT_LABORATORIOS);
    bancada = malloc(sizeof(repositorio_t) * QUANT_LABORATORIOS);
    infectados = malloc(sizeof(infectado_t) * QUANT_INFECTADOS);
    sem_init(&objetivoMutex, 0, 1);
    pthread_mutex_init(&bancadaMutex, NULL);
    pthread_cond_init(&condicionalLaboratorio, NULL);
    pthread_cond_init(&condicionalInfectado, NULL);


    /* INICIALIZA LABORATORIOS */
    for (i=0; i < QUANT_LABORATORIOS; i++){

        laboratorios[i].id = i+1;
        laboratorios[i].repositorio = &(bancada[i]);
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].atingiramObjetivo = &atigiramObjetivo;
        laboratorios[i].objetivoMutex = &objetivoMutex;
        laboratorios[i].bancadaMutex = &bancadaMutex;
        laboratorios[i].condicionalLaboratorio = &condicionalLaboratorio;        
        laboratorios[i].condicionalInfectado = &condicionalInfectado;      

        /* INICIALIZA REPOSITORIO */
        bancada[i].virus = (posicaoInsumoIndisponivel == 0) ? INSUMO_INDISPONIVEL : 1;
        bancada[i].injecao = (posicaoInsumoIndisponivel == 1) ? INSUMO_INDISPONIVEL : 1;
        bancada[i].elementoSecreto = (posicaoInsumoIndisponivel == 2) ? INSUMO_INDISPONIVEL : 1;        

        posicaoInsumoIndisponivel++;
        posicaoInsumoIndisponivel = posicaoInsumoIndisponivel % TAMANHO_REPOSITORIO;

        /* IMPRIMIR LABORATORIO */
        printf("\nLABORATORIO ID %d\n", laboratorios[i].id);
        if (laboratorios[i].repositorio->virus != INSUMO_INDISPONIVEL) printf("Fornece virus\n");
        if (laboratorios[i].repositorio->injecao != INSUMO_INDISPONIVEL) printf("Fornece injecao\n");
        if (laboratorios[i].repositorio->elementoSecreto != INSUMO_INDISPONIVEL) printf("Fornece elemento secreto\n");

    }



    /* INICIALIZA INFECTADOS */
    for (i=0; i < QUANT_INFECTADOS; i++){
        infectados[i].id = i+1;
        infectados[i].bancada = &(bancada[i]);             
        infectados[i].ciclosMinimos = ciclosMinimos;
        infectados[i].atingiramObjetivo = &atigiramObjetivo;
        infectados[i].objetivoMutex = &objetivoMutex;        
        infectados[i].bancadaMutex = &bancadaMutex;        
        infectados[i].condicionalLaboratorio = &condicionalLaboratorio;        
        infectados[i].condicionalInfectado = &condicionalInfectado;        

        /* INICIALIZA REPOSITORIO */
        infectados[i].repositorio.virus = (posicaoInsumoInfinito == 0) ? INSUMO_INFINITO : 0;
        infectados[i].repositorio.injecao = (posicaoInsumoInfinito == 1) ? INSUMO_INFINITO : 0;
        infectados[i].repositorio.elementoSecreto = (posicaoInsumoInfinito == 2) ? INSUMO_INFINITO : 0;

        posicaoInsumoInfinito++;
        posicaoInsumoInfinito = posicaoInsumoInfinito % TAMANHO_REPOSITORIO;

        /* IMPRIMIR INFECTADO */
        printf("\nINFECTADO ID %d\n", infectados[i].id);
        if (infectados[i].repositorio.virus == INSUMO_INFINITO) printf("Possui virus\n");
        if (infectados[i].repositorio.injecao == INSUMO_INFINITO) printf("Possui injecao\n");
        if (infectados[i].repositorio.elementoSecreto == INSUMO_INFINITO) printf("Possui elemento secreto\n");
    }


    /* EXECUTAS AS THREADS LABORATORIOS E INFECTADOS */    
    for (i = 0; i < QUANT_LABORATORIOS; i++) {
        pthread_create(&(laboratorios[i].thread), NULL, f_laboratorio, &(laboratorios[i]));
    }
   
    for (i = 0; i < QUANT_INFECTADOS; i++) {
        pthread_create(&(infectados[i].thread), NULL, f_infectado, &(infectados[i]));
    }

    for (i = 0; i < QUANT_LABORATORIOS; i++) {
        pthread_join(laboratorios[i].thread, NULL);
    }

    for (i = 0; i < QUANT_INFECTADOS; i++) {
        pthread_join(infectados[i].thread, NULL);
    }


    /* DESTROI MEMORIA ALOCADA */
    
    // sem_destroy(&objetivoMutex);
    // pthread_mutex_destroy(&bancadaMutex);
    // pthread_cond_destroy(&condicionalLaboratorio);    
    // pthread_cond_destroy(&condicionalInfectado);    

    return 0;
}