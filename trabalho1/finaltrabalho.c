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
    sem_t total;
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
    sem_t *atingiramObjetivo;             
} laboratorio_t;

typedef struct {
    pthread_t thread;
    int id;
    int ciclosMinimos;    
    int ciclosAtual;                               
    insumo_t insumoInfinito;  
    produto_t *bancada;
    pthread_mutex_t *bancadaMutex;
    sem_t *atingiramObjetivo;      
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


void indicesProdutosFaltantes(produto_t * bancada, insumo_t insumoDoInfectado, int * indiceProduto1, int * indiceProduto2) {
    
    insumo_t insumo1Tipo;
    insumo_t insumo2Tipo;
    int totalProduto;
    int produtoVerificados;
    int i;

    switch(insumoDoInfectado){
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

    *indiceProduto1 = -1;
    *indiceProduto2 = -1;
    totalProduto = -1;
    produtoVerificados = 0;
    i = (rand() % TOTAL_PRODUTO_BANCADA);

    while ( (*indiceProduto1 == -1 || *indiceProduto2 == -1) && (produtoVerificados <= TOTAL_PRODUTO_BANCADA) ) {
        
        if (sem_getvalue(&(bancada[i].total), &totalProduto) == 0) {
            if ((totalProduto>0) && (bancada[i].tipo == insumo1Tipo) && (*indiceProduto1 == -1)) {
                *indiceProduto1 = i;
            } else if ((totalProduto>0) && (bancada[i].tipo == insumo2Tipo) && (*indiceProduto2 == -1)) {
                *indiceProduto2 = i;
            }

            i++;
            i = (i == TOTAL_PRODUTO_BANCADA) ? 0 : i;
            produtoVerificados++;
        }
    }
}


void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    bool continuarOperando = true;
    int totalProduto1;
    int totalProduto2;
    int atingiramObjetivo;

    while (continuarOperando == true) {
        
        totalProduto1=0;
        totalProduto2=0;
        
        if ((sem_getvalue(&(laboratorio->produto1->total), &totalProduto1)== 0) 
            && (sem_getvalue(&(laboratorio->produto2->total), &totalProduto2) == 0)) {

            if ((totalProduto1 == 0) && (totalProduto2 == 0) ) {

                pthread_mutex_lock(laboratorio->bancadaMutex);

                if ((sem_post(&(laboratorio->produto1->total)) == 0) && (sem_post(&(laboratorio->produto2->total)) == 0)) {
                  
                    laboratorio->ciclosAtual++;

                    printf("\nLAB %d produziu %d vezes", laboratorio->id, laboratorio->ciclosAtual);

                    if (laboratorio->ciclosAtual == laboratorio->ciclosMinimos) {
                        printf("\nLAB %d atingiu objetivo", laboratorio->id);
                        sem_post(laboratorio->atingiramObjetivo);
                    }
                }

                pthread_mutex_unlock(laboratorio->bancadaMutex);

                #ifdef _WIN32
                Sleep(800);
                #else
                sleep(0.8);
                #endif

            } else {
                #ifdef _WIN32
                Sleep(200);
                #else
                sleep(0.2);
                #endif
            }
        }

        if (sem_getvalue(laboratorio->atingiramObjetivo, &atingiramObjetivo) == 0) {
            if (atingiramObjetivo == (TOTAL_LABORATORIOS + TOTAL_INFECTADOS)) {
                continuarOperando = false;
            }
        }
    }

    return NULL;
}


void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;

    bool continuarOperando = true;
    int indiceProduto1;
    int indiceProduto2;
    int atingiramObjetivo;

    while (continuarOperando == true) {

        indiceProduto1 = -1;
        indiceProduto2 = -1;
        
        indicesProdutosFaltantes(infectado->bancada, infectado->insumoInfinito, &indiceProduto1, &indiceProduto2);

        if ((indiceProduto1 != -1) && (indiceProduto2 != -1)) {
        
            pthread_mutex_lock(infectado->bancadaMutex);

            indiceProduto1 = -1;
            indiceProduto2 = -1;

            indicesProdutosFaltantes(infectado->bancada, infectado->insumoInfinito, &indiceProduto1, &indiceProduto2);

            if ((indiceProduto1 != -1) && (indiceProduto2 != -1)) {

                if ((sem_wait(&(infectado->bancada[indiceProduto1].total)) == 0)
                    && (sem_wait(&(infectado->bancada[indiceProduto2].total)) == 0)) {

                    infectado->ciclosAtual++;
                    printf("\nINF %d fez a vacina %d vezes", infectado->id, infectado->ciclosAtual);
                    if (infectado->ciclosAtual == infectado->ciclosMinimos) {
                        printf("\nINF %d atingiu objetivo", infectado->id);
                        sem_post(infectado->atingiramObjetivo);
                    }
                }
            }

            pthread_mutex_unlock(infectado->bancadaMutex);

            #ifdef _WIN32
            Sleep(800);
            #else
            sleep(0.8);
            #endif

        } else {
            #ifdef _WIN32
            Sleep(200);
            #else
            sleep(0.2);
            #endif
        }

        if (sem_getvalue(infectado->atingiramObjetivo, &atingiramObjetivo) == 0) {
            if (atingiramObjetivo == (TOTAL_LABORATORIOS + TOTAL_INFECTADOS)) {
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
    sem_t atingiramObjetivo;  


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
    sem_init(&atingiramObjetivo, 0, 0);

    /* INICIALIZA BANCADA */
    for (i=0; i < (TOTAL_PRODUTO_BANCADA); i++){
        sem_init(&(bancada[i].total), 0, 0);
        bancada[i].tipo = (insumo_t) i % TOTAL_INSUMOS;
    }

    /* INICIALIZA LABORATÓRIOS */
    int produto = 0;
    for (i=0; i < TOTAL_LABORATORIOS; i++){
        laboratorios[i].id = i+1;
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].ciclosAtual = 0;
        laboratorios[i].bancadaMutex = &bancadaMutex;
        laboratorios[i].atingiramObjetivo = &atingiramObjetivo;
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
        infectados[i].atingiramObjetivo = &atingiramObjetivo;
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
        sem_destroy(&(bancada[i].total));
    }
    pthread_mutex_destroy(&bancadaMutex);
    sem_destroy(&atingiramObjetivo);
    free(laboratorios); 
    free(infectados); 
    free(bancada); 

    return 0;
}