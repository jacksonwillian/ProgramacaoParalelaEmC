/* TRABALHO 1: Jackson Willian Silva Agostinho - 20172BSI0335 */ 

#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TOTAL_INFECTADOS 3
#define TOTAL_LABORATORIOS 3
#define TOTAL_PRODUTO_BANCADA 6
#define TOTAL_INSUMOS 3

typedef enum {
    false = 0,
    true = 1
} bool;

typedef enum { 
    Virus = 0,
    Injecao = 1,
    Elementox = 2 
} insumo_t;

typedef struct {
    sem_t quantidade;
    insumo_t tipo;
} produto_t;

typedef struct {
    pthread_t thread;
    int id;
    int ciclosMinimos;
    int ciclosAtual;                              
    produto_t *produto1;
    produto_t *produto2;
    pthread_mutex_t *bancadaMutex;
    sem_t *concluiram;             
} laboratorio_t;

typedef struct {
    pthread_t thread;
    int id;
    int ciclosMinimos;    
    int ciclosAtual;                               
    insumo_t insumoInfinito;  
    produto_t *bancada;
    pthread_mutex_t *bancadaMutex;
    sem_t *concluiram;      
} infectado_t; 

void print_tipo_insumo(insumo_t tipo_insumo) {
    switch(tipo_insumo) {
        case Virus: 
            printf("virus");
        break;
        case Injecao: 
            printf("injecao");
        break;
        case Elementox: 
            printf("elementox");
        break;
    }
}

void print_laboratorio(int id, insumo_t tipo_insumo) {
    printf("\nLAB %d Tem ", id);
    print_tipo_insumo(tipo_insumo);
}

void print_infectado(int id, insumo_t tipo_insumo) {
    printf("\nINF %d Tem ", id);
    print_tipo_insumo(tipo_insumo);
}


void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    bool continuarOperando = true;
    int quantidade1 = 0;
    int quantidade2 = 0;
    int concluiram;

    while (continuarOperando == true) {
        
        quantidade1=0;
        quantidade2=0;
        
        if ((sem_getvalue(&(laboratorio->produto1->quantidade), &quantidade1)== 0) 
            && (sem_getvalue(&(laboratorio->produto2->quantidade), &quantidade2) == 0)) {

            if ((quantidade1 == 0) && (quantidade2 == 0) ) {

                pthread_mutex_lock(laboratorio->bancadaMutex);

                if ( (sem_post(&(laboratorio->produto1->quantidade)) != 0) 
                    || (sem_post(&(laboratorio->produto2->quantidade)) != 0)) {
                    printf("\nLAB %d erro ao produzir", laboratorio->id);
                } else {

                    laboratorio->ciclosAtual++;

                    printf("\nLAB %d produziu %d vezes", laboratorio->id, laboratorio->ciclosAtual);

                    if (laboratorio->ciclosAtual == laboratorio->ciclosMinimos) {
                        printf("\nLAB %d atingiu objetivo", laboratorio->id);
                        sem_post(laboratorio->concluiram);
                    }
                }
                pthread_mutex_unlock(laboratorio->bancadaMutex);

                sleep(0.8);

            } else {
                sleep(0.3);
            }
        }

        if (sem_getvalue(laboratorio->concluiram, &concluiram) == 0) {
            if (concluiram == (TOTAL_LABORATORIOS + TOTAL_INFECTADOS)) {
                continuarOperando = false;
            }
        }
    }
    
    return NULL;
}


/* infectados consomem os insumos */
void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;

    bool continuarOperando = true;
    insumo_t insumo1Tipo;
    int insumo1Posicao;
    insumo_t insumo2Tipo;
    int insumo2Posicao;
    int produtoVerificados;
    int i = (rand() % TOTAL_PRODUTO_BANCADA);
    int quantidade;
    int concluiram;


    switch(infectado->insumoInfinito){
        case Virus: {
            insumo1Tipo = Injecao;
            insumo2Tipo = Elementox;
        } break;
        case Injecao: {
            insumo1Tipo = Virus;
            insumo2Tipo = Elementox;
        } break;
        case Elementox: {
            insumo1Tipo = Virus;
            insumo2Tipo = Injecao; 
        } break;
    }
 
    while (continuarOperando == true) {
        
        insumo1Posicao = -1;
        insumo2Posicao = -1;
        
        quantidade = -1;
         
        produtoVerificados = 0;
        
        while ( (insumo1Posicao == -1 || insumo2Posicao == -1) && (produtoVerificados <= TOTAL_PRODUTO_BANCADA) ) {
            
            if (sem_getvalue(&(infectado->bancada[i].quantidade), &quantidade) == 0) {
                if ((quantidade>0) && (infectado->bancada[i].tipo == insumo1Tipo) && (insumo1Posicao == -1)) {
                    insumo1Posicao = i;
                } else if ((quantidade>0) && (infectado->bancada[i].tipo == insumo2Tipo) && (insumo2Posicao == -1)) {
                    insumo2Posicao = i;
                }

                i++;
                i = (i == TOTAL_PRODUTO_BANCADA) ? 0 : i;
                produtoVerificados++;
            }
        
        }


        if ((insumo1Posicao != -1) && (insumo2Posicao != -1)) {
        
            pthread_mutex_lock(infectado->bancadaMutex);

            insumo1Posicao = -1;
            insumo2Posicao = -1;
            
            quantidade = -1;
            
            produtoVerificados = 0;
            

            while ( (insumo1Posicao == -1 || insumo2Posicao == -1) && (produtoVerificados <= TOTAL_PRODUTO_BANCADA) ) {
                    
                if (sem_getvalue(&(infectado->bancada[i].quantidade), &quantidade) == 0) {

                    if ((quantidade>0) && (infectado->bancada[i].tipo == insumo1Tipo) && (insumo1Posicao == -1)) {
                        insumo1Posicao = i;
                    } else if ((quantidade>0) && (infectado->bancada[i].tipo == insumo2Tipo) && (insumo2Posicao == -1)) {
                        insumo2Posicao = i;
                    }

                    produtoVerificados++;
                    i++;
                    i = (i == TOTAL_PRODUTO_BANCADA) ? 0 : i;
                } 
        
            }


            if ((insumo1Posicao != -1) && (insumo2Posicao != -1)) {

                sem_wait(&(infectado->bancada[insumo1Posicao].quantidade));
                sem_wait(&(infectado->bancada[insumo2Posicao].quantidade));

                infectado->ciclosAtual++;

                printf("\nINF %d fez a vacina %d vezes", infectado->id, infectado->ciclosAtual);

                if (infectado->ciclosAtual == infectado->ciclosMinimos) {
                    printf("\nINF %d atingiu objetivo", infectado->id);
                    sem_post(infectado->concluiram);
                }
            }

            pthread_mutex_unlock(infectado->bancadaMutex);

            sleep(0.8);

        } else {
            sleep(0.3);
        }

        if (sem_getvalue(infectado->concluiram, &concluiram) == 0) {
            if (concluiram == (TOTAL_LABORATORIOS + TOTAL_INFECTADOS)) {
                continuarOperando = false;
            }
        }

    }
    
    return NULL;
}



int main(int argc, char** argv) {

    /* DECLARAÇÃO DAS VARIÁVEIS */
    int i, ciclosMinimos;
    produto_t *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    pthread_mutex_t bancadaMutex;
    sem_t concluiram;  


    /* VALIDAÇÃO DE ENTRADAS */
    if ( argc != 2 ) {
        printf("\nA quantidade de argumentos é inválida!\n");
        printf("\nO primeiro argumento é o número mínimo de vezes que cada um deve realizar seu objetivo primordial.\n\n");
        return -1;
    } else if ( atoi(argv[1]) < 1) {
        printf("\nO valor do argumento é inválido!\n\n");
        return -1;
    }   
 
    /* INICIALIZAÇÃO DAS VARIÁVEIS */    
    ciclosMinimos = atoi(argv[1]);
    laboratorios = malloc(sizeof(laboratorio_t) * TOTAL_LABORATORIOS);
    bancada =  malloc(sizeof(produto_t) * TOTAL_PRODUTO_BANCADA);                
    infectados = malloc(sizeof(infectado_t) * TOTAL_INFECTADOS);                            
    pthread_mutex_init(&bancadaMutex, NULL);
    sem_init(&concluiram, 0, 0);

    /* INICIALIZA BANCADA */
    for (i=0; i < (TOTAL_PRODUTO_BANCADA); i++){
        sem_init(&(bancada[i].quantidade), 0, 0);
        bancada[i].tipo = (insumo_t) i % TOTAL_INSUMOS;
    }

    /* INICIALIZA LABORATÓRIOS */
    int produto = 0;
    for (i=0; i < TOTAL_LABORATORIOS; i++){
        laboratorios[i].id = i+1;
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].ciclosAtual = 0;
        laboratorios[i].bancadaMutex = &bancadaMutex;
        laboratorios[i].concluiram = &concluiram;
        laboratorios[i].produto1 = &(bancada[produto]);
        produto++;
        laboratorios[i].produto2 = &(bancada[produto]);
        produto++;
        print_laboratorio(laboratorios[i].id, laboratorios[i].produto1->tipo);
        print_laboratorio(laboratorios[i].id, laboratorios[i].produto2->tipo);
    }

    // PULA LINHA
    printf("\n");

    /* INICIALIZA INFECTADOS */
    for (i=0; i < TOTAL_INFECTADOS; i++){
        infectados[i].id = i+1;
        infectados[i].bancada = bancada;             
        infectados[i].ciclosMinimos = ciclosMinimos;       
        infectados[i].ciclosAtual = 0;         
        infectados[i].bancadaMutex = &bancadaMutex;
        infectados[i].concluiram = &concluiram;
        infectados[i].insumoInfinito = (insumo_t) i; 
        print_infectado(infectados[i].id, infectados[i].insumoInfinito);
    }

    // PULA LINHA
    printf("\n");

    /* EXECUTA AS THREADS */   
    for (i = 0; i < TOTAL_LABORATORIOS; i++) {
        pthread_create(&(laboratorios[i].thread), NULL, f_laboratorio, &(laboratorios[i]));
    }

    for (i = 0; i < TOTAL_INFECTADOS; i++) {
        pthread_create(&(infectados[i].thread), NULL, f_infectado, &(infectados[i]));
    }


    /* ESPERA AS THREADS TERMINAREM */    
    for (i = 0; i < TOTAL_LABORATORIOS; i++) {
        pthread_join(laboratorios[i].thread, NULL);
    }

    for (i = 0; i < TOTAL_INFECTADOS; i++) {
        pthread_join(infectados[i].thread, NULL);
    }

    // PULA LINHA
    printf("\n\n");

    /* APRESENTA O RESULTADO */
    for (i = 0; i < TOTAL_LABORATORIOS; i++) {
        printf(">>> Laboratorio %d: %d\n", laboratorios[i].id, laboratorios[i].ciclosAtual); 
    }
    for (i = 0; i < TOTAL_INFECTADOS; i++) {
        printf(">>> Infectado %d: %d\n", infectados[i].id, infectados[i].ciclosAtual); 
    }


    /* DESTRÓI MEMÓRIA ALOCADA */
    for (i=0; i < (TOTAL_PRODUTO_BANCADA); i++){
        sem_destroy(&(bancada[i].quantidade));
    }
    pthread_mutex_destroy(&bancadaMutex);
    sem_destroy(&concluiram);
    free(laboratorios); 
    free(infectados); 
    free(bancada); 

    return 0;
}