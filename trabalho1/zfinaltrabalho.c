/* TRABALHO 1: Jackson Willian Silva Agostinho - 20172BSI0335 */ 

#if defined(_WIN32) || defined(__CYGWIN__)
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
    int totalLaboratorios;
    int totalInfectados;    
    int capacidadeDaBancada;      
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
    int totalLaboratorios;
    int totalInfectados;    
    int capacidadeDaBancada;        
} infectado_t; 


/* realiza os prints das informações do infectado, labotório e tipo de insumo */
void print_tipo_insumo(insumo_t tipo_insumo);
void print_laboratorio(int id, insumo_t tipo_insumo);
void print_infectado(int id, insumo_t tipo_insumo);


void indicesProdutosFaltantes(produto_t * bancada, int capacidadeDaBancada, insumo_t insumoDoInfectado, int * indiceProduto1, int * indiceProduto2) {
    
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
    i = (rand() % capacidadeDaBancada);

    while ( (*indiceProduto1 == -1 || *indiceProduto2 == -1) && (produtoVerificados <= capacidadeDaBancada) ) {
        
        if (sem_getvalue(&(bancada[i].total), &totalProduto) == 0) {
            if ((totalProduto>0) && (bancada[i].tipo == insumo1Tipo) && (*indiceProduto1 == -1)) {
                *indiceProduto1 = i;
            } else if ((totalProduto>0) && (bancada[i].tipo == insumo2Tipo) && (*indiceProduto2 == -1)) {
                *indiceProduto2 = i;
            }

            i++;
            i = (i == capacidadeDaBancada) ? 0 : i;
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
    int tempo = 0.6;

    while (continuarOperando == true) {
        
        totalProduto1=0;
        totalProduto2=0;
        
        if ((sem_getvalue(&(laboratorio->produto1->total), &totalProduto1)== 0) 
            && (sem_getvalue(&(laboratorio->produto2->total), &totalProduto2) == 0)) {

            if ((totalProduto1 == 0) && (totalProduto2 == 0) ) {

                pthread_mutex_lock(laboratorio->bancadaMutex);

                if ((sem_post(&(laboratorio->produto1->total)) == 0) && (sem_post(&(laboratorio->produto2->total)) == 0)) {
                    laboratorio->ciclosAtual++;
                    // printf("\nLAB %d produziu %d vezes", laboratorio->id, laboratorio->ciclosAtual);
                    if (laboratorio->ciclosAtual == laboratorio->ciclosMinimos) {
                        // printf("\nLAB %d atingiu objetivo", laboratorio->id);
                        sem_post(laboratorio->atingiramObjetivo);
                    }
                }

                pthread_mutex_unlock(laboratorio->bancadaMutex);

                #if defined(_WIN32) || defined(__CYGWIN__)
                Sleep(tempo * 1000);
                #else
                sleep(tempo);
                #endif
            } 
        }

        if (sem_getvalue(laboratorio->atingiramObjetivo, &atingiramObjetivo) == 0) {
            if (atingiramObjetivo == (laboratorio->totalLaboratorios + laboratorio->totalInfectados)) {
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
        
        indicesProdutosFaltantes(infectado->bancada, infectado->capacidadeDaBancada, infectado->insumoInfinito, &indiceProduto1, &indiceProduto2);

        if ((indiceProduto1 != -1) && (indiceProduto2 != -1)) {
        
            pthread_mutex_lock(infectado->bancadaMutex);

            indiceProduto1 = -1;
            indiceProduto2 = -1;

            indicesProdutosFaltantes(infectado->bancada, infectado->capacidadeDaBancada, infectado->insumoInfinito, &indiceProduto1, &indiceProduto2);

            if ((indiceProduto1 != -1) && (indiceProduto2 != -1)) {

                if ((sem_wait(&(infectado->bancada[indiceProduto1].total)) == 0)
                    && (sem_wait(&(infectado->bancada[indiceProduto2].total)) == 0)) {
                    infectado->ciclosAtual++;
                    // printf("\nINF %d fez a vacina %d vezes", infectado->id, infectado->ciclosAtual);
                    if (infectado->ciclosAtual == infectado->ciclosMinimos) {
                        // printf("\nINF %d atingiu objetivo", infectado->id);
                        sem_post(infectado->atingiramObjetivo);
                    }
                }
            }

            pthread_mutex_unlock(infectado->bancadaMutex);
        } 

        if (sem_getvalue(infectado->atingiramObjetivo, &atingiramObjetivo) == 0) {
            if (atingiramObjetivo == (infectado->totalLaboratorios + infectado->totalInfectados)) {
                continuarOperando = false;
            }
        }
    }

    return NULL;
}



int main(int argc, char** argv) {

    /* DECLARAÇÃO DAS VARIÁVEIS */
    int i, ciclosMinimos, capacidadeDaBancada, totalLaboratorios, totalInfectados, totalInsumo;
    produto_t *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    pthread_mutex_t bancadaMutex;
    sem_t atingiramObjetivo;  


    /* VALIDAÇÃO DE ENTRADAS */
    if ( argc != 2 ) {
        printf("\nA quantidade de argumentos é inválida!\n");
        printf("\nO único argumento esperado é o número mínimo de vezes que cada um deve realizar seu objetivo primordial.\n\n");
        return -1;
    } else if ( atoi(argv[1]) < 1) {
        printf("\nO valor do argumento é inválido!\n\n");
        return -1;
    }   
 
    /* INICIALIZAÇÃO DAS VARIÁVEIS */    
    ciclosMinimos = atoi(argv[1]);
    capacidadeDaBancada = 6;
    totalLaboratorios = 3;
    totalInfectados = 3;
    totalInsumo = 3;
    laboratorios = malloc(sizeof(laboratorio_t) * totalLaboratorios);
    bancada =  malloc(sizeof(produto_t) * capacidadeDaBancada);                
    infectados = malloc(sizeof(infectado_t) * totalInfectados);                            
    pthread_mutex_init(&bancadaMutex, NULL);
    sem_init(&atingiramObjetivo, 0, 0);

    /* INICIALIZA BANCADA */
    for (i=0; i < (capacidadeDaBancada); i++){
        sem_init(&(bancada[i].total), 0, 0);
        bancada[i].tipo = (insumo_t) i % totalInsumo;
    }

    /* INICIALIZA LABORATÓRIOS */
    int produto = 0;
    for (i=0; i < totalLaboratorios; i++){
        laboratorios[i].id = i+1;
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].ciclosAtual = 0;
        laboratorios[i].bancadaMutex = &bancadaMutex;
        laboratorios[i].atingiramObjetivo = &atingiramObjetivo;
        laboratorios[i].produto1 = &(bancada[produto]);
        laboratorios[i].capacidadeDaBancada = capacidadeDaBancada;
        laboratorios[i].totalLaboratorios = totalLaboratorios;
        laboratorios[i].totalInfectados = totalInfectados;
        produto++;
        laboratorios[i].produto2 = &(bancada[produto]);
        produto++;
        // print_laboratorio(laboratorios[i].id, laboratorios[i].produto1->tipo);
        // print_laboratorio(laboratorios[i].id, laboratorios[i].produto2->tipo);
        // printf("\n");
    }


    /* INICIALIZA INFECTADOS */
    for (i=0; i < totalInfectados; i++){
        infectados[i].id = i+1;
        infectados[i].bancada = bancada;             
        infectados[i].ciclosMinimos = ciclosMinimos;       
        infectados[i].ciclosAtual = 0;         
        infectados[i].bancadaMutex = &bancadaMutex;
        infectados[i].atingiramObjetivo = &atingiramObjetivo;
        infectados[i].insumoInfinito = (insumo_t) i; 
        infectados[i].capacidadeDaBancada = capacidadeDaBancada;
        infectados[i].totalLaboratorios = totalLaboratorios;
        infectados[i].totalInfectados = totalInfectados;        
        // print_infectado(infectados[i].id, infectados[i].insumoInfinito);
        // printf("\n\n");
    }
    
    /* EXECUTA AS THREADS */   
    for (i = 0; i < totalLaboratorios; i++) {
        pthread_create(&(laboratorios[i].thread), NULL, f_laboratorio, &(laboratorios[i]));
    }

    for (i = 0; i < totalInfectados; i++) {
        pthread_create(&(infectados[i].thread), NULL, f_infectado, &(infectados[i]));
    }


    /* ESPERA AS THREADS TERMINAREM */    
    for (i = 0; i < totalLaboratorios; i++) {
        pthread_join(laboratorios[i].thread, NULL);
    }

    for (i = 0; i < totalInfectados; i++) {
        pthread_join(infectados[i].thread, NULL);
    }

    /* APRESENTA O RESULTADO */
    for (i = 0; i < totalLaboratorios; i++) {
        printf("Laboratorio %d: %d\n", laboratorios[i].id, laboratorios[i].ciclosAtual); 
    }
    for (i = 0; i < totalInfectados; i++) {
        printf("Infectado %d: %d\n", infectados[i].id, infectados[i].ciclosAtual); 
    }

    /* DESTRÓI MEMÓRIA ALOCADA */
    for (i=0; i < (capacidadeDaBancada); i++){
        sem_destroy(&(bancada[i].total));
    }
    pthread_mutex_destroy(&bancadaMutex);
    sem_destroy(&atingiramObjetivo);
    free(laboratorios); 
    free(infectados); 
    free(bancada); 

    return 0;
}



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
