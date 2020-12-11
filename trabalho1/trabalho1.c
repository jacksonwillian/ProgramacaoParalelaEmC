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
    int ciclosMinimos;              // quantidade minima de ciclos de producao
    int *atingiramObjetivo;         // total que atingiram a quantidade minima de ciclos  
    sem_t *objetivoMutex;     
} laboratorio_t;


typedef struct {
    pthread_t thread;
    int id;
    repositorio_t repositorio;
    repositorio_t *bancada;         // referencia para os repositorios dos laboratorios
    int ciclosMinimos;              // quantidade minima de ciclos de consumo
    int *atingiramObjetivo;         // total que atingiram a quantidade minima de ciclos 
    sem_t *objetivoMutex;
} infectado_t;



// Permite que os laboratorios produzam os insumos
void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;
    int ciclosAtualProducao = 0;
    int continuarOperando = TRUE;
    
    printf("\nLABORATORIO ID [%d]\n", laboratorio->id); 

    while (continuarOperando == TRUE) {
        
        if (ciclosAtualProducao == laboratorio->ciclosMinimos) {
            sem_wait(laboratorio->objetivoMutex); 
            *(laboratorio->atingiramObjetivo) += 1; 
            sem_post(laboratorio->objetivoMutex); 
            printf("\nLABORATORIO ID [%d] atingiu o objetivo! [Minimo %d] [Atual %d]\n", \
                   laboratorio->id, laboratorio->ciclosMinimos, ciclosAtualProducao); 
        } else if (ciclosAtualProducao > laboratorio->ciclosMinimos) {
            sem_wait(laboratorio->objetivoMutex); 
            if (*(laboratorio->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS)) {
                continuarOperando = FALSE;
            } 
            sem_post(laboratorio->objetivoMutex);
        }

        ciclosAtualProducao = ciclosAtualProducao + 1;
        sleep(3);
    }
    
    printf("\nLABORATORIO ID [%d] fechou\n", laboratorio->id);

    return NULL;
}


// Permite que os infectados consumam os insumos
void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;
    int ciclosAtualConsumo = 0;
    int continuarOperando = TRUE;
    
    printf("\nINFECTADO ID [%d]\n", infectado->id); 

    while (continuarOperando == TRUE) {
        
        if (ciclosAtualConsumo == infectado->ciclosMinimos) {
            sem_wait(infectado->objetivoMutex); 
            *(infectado->atingiramObjetivo) += 1; 
            sem_post(infectado->objetivoMutex); 
            printf("\nINFECTADO ID [%d] atingiu o objetivo! [Minimo %d] [Atual %d]\n", \
                   infectado->id, infectado->ciclosMinimos, ciclosAtualConsumo); 
        } else if (ciclosAtualConsumo > infectado->ciclosMinimos) {
            sem_wait(infectado->objetivoMutex); 
            if (*(infectado->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS)) {
                continuarOperando = FALSE;
            } 
            sem_post(infectado->objetivoMutex);
        }
        ciclosAtualConsumo = ciclosAtualConsumo + 1;
        sleep(3);
    }
    
    printf("\nINFECTADO ID [%d] saiu\n", infectado->id);

    return NULL;}



int main(int argc, char** argv) {

    /* DECLARACAO DAS VARIAVEIS */
    int i, ciclosMinimos, atigiramObjetivo, posicaoInsumoInfinito, posicaoInsumoIndisponivel;
    repositorio_t *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    sem_t objetivoMutex;
    

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

    /* INICIALIZA LABORATORIOS */
    for (i=0; i < QUANT_LABORATORIOS; i++){

        laboratorios[i].id = i+1;
        laboratorios[i].repositorio = &(bancada[i]);
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].atingiramObjetivo = &atigiramObjetivo;
        laboratorios[i].objetivoMutex = &objetivoMutex;

        /* INICIALIZA REPOSITORIO */
        bancada[i].virus = (posicaoInsumoIndisponivel == 0) ? INSUMO_INDISPONIVEL : 0;
        bancada[i].injecao = (posicaoInsumoIndisponivel == 1) ? INSUMO_INDISPONIVEL : 0;
        bancada[i].elementoSecreto = (posicaoInsumoIndisponivel == 2) ? INSUMO_INDISPONIVEL : 0;        

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
    return 0;
}