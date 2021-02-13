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
    int *atingiramObjetivo;                               
    produto_t *produto1;
    produto_t *produto2;
    pthread_mutex_t *bancadaMutex;             
} laboratorio_t;

typedef struct {
    pthread_t thread;
    int id;
    int ciclosMinimos;    
    int ciclosAtual;    
    int *atingiramObjetivo;                                
    insumo_t insumoInfinito;  
    produto_t *bancada;
    pthread_mutex_t *bancadaMutex;    
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
    printf("\nLAB %d Disponibiliza ", id);
    print_tipo_insumo(tipo_insumo);
}


void print_infectado(int id, insumo_t tipo_insumo) {
    printf("\nINF %d Tem ", id);
    print_tipo_insumo(tipo_insumo);
}



/* laboratorios produzem os insumos */
void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    printf("\nLAB %d Abriu", laboratorio->id);

    bool continuarOperando = true;
    int quantidade1 = 0;
    int quantidade2 = 0;
    bool errorSemaforo = false;
    bool esperar;

    while (continuarOperando == true) {
        
        errorSemaforo = false;

        quantidade1=0;
        quantidade2=0;
        
        if ((sem_getvalue(&(laboratorio->produto1->quantidade), &quantidade1) != 0) 
            || (sem_getvalue(&(laboratorio->produto2->quantidade), &quantidade2) != 0)) {
            errorSemaforo = true;
            printf("\nLAB %d erro ao verificar semaforo", laboratorio->id);
        }        
        
        if ((errorSemaforo == false) && (quantidade1 == 0) && (quantidade2 == 0) ) {

            esperar = false;

            pthread_mutex_lock(laboratorio->bancadaMutex);

            if ( (*laboratorio->atingiramObjetivo) == (TOTAL_INFECTADOS + TOTAL_LABORATORIOS) ) {
                continuarOperando = false;
            } else {

                if ( (sem_post(&(laboratorio->produto1->quantidade)) != 0) 
                    || (sem_post(&(laboratorio->produto2->quantidade)) != 0)) {
                    printf("\nLAB %d erro ao produzir", laboratorio->id);
                } else {

                    laboratorio->ciclosAtual++;

                    printf("\nLAB %d produziu %d vezes", laboratorio->id, laboratorio->ciclosAtual);

                    if (laboratorio->ciclosAtual == laboratorio->ciclosMinimos) {
                        (*laboratorio->atingiramObjetivo)++;
                        printf("\nLAB %d alcançou objetivo", laboratorio->id);
                    }

                    esperar = true;

                }
            }

            pthread_mutex_unlock(laboratorio->bancadaMutex);

        } else {
            // printf("\nLAB %d não usou a bancada", laboratorio->id);
        }

        if(esperar==true) {
            esperar = false;
            sleep(1 + rand() % 3);
        }

        sleep(1 + rand() % 3);

    }
    
    return NULL;
}


/* infectados consomem os insumos */
void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;

    printf("\nINF %d Chegou", infectado->id);

    bool continuarOperando = true;
    
    insumo_t insumo1Tipo;
    int insumo1Posicao;
    insumo_t insumo2Tipo;
    int insumo2Posicao;
    int produtoVerificados;
    int i;
    int quantidade;
    bool esperar;


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
        
        i = rand() % TOTAL_PRODUTO_BANCADA;
        while ( (insumo1Posicao == -1 || insumo2Posicao == -1) && (produtoVerificados < TOTAL_PRODUTO_BANCADA) ) {
            
            if (sem_getvalue(&(infectado->bancada[i].quantidade), &quantidade) == 0) {

                if ((quantidade>0) && (infectado->bancada[i].tipo == insumo1Tipo) && (insumo1Posicao == -1)) {
                    insumo1Posicao = i;
                    // printf("\nINF %d tem insumo 1 do tipo ", infectado->id);
                    // print_tipo_insumo(infectado->bancada[i].tipo);
                    // printf(" na posicao %d", insumo1Posicao);
                } else if ((quantidade>0) && (infectado->bancada[i].tipo == insumo2Tipo) && (insumo2Posicao == -1)) {
                    insumo2Posicao = i;
                    // printf("\nINF %d tem insumo 2 do tipo ", infectado->id);
                    // print_tipo_insumo(infectado->bancada[i].tipo);
                    // printf(" na posicao %d", insumo2Posicao);
                }

                i++;
                i = (i == (TOTAL_PRODUTO_BANCADA - 1) ? 0 : i);
                produtoVerificados++;

            }
          

            
            // sleep(2);
            
        }


        if ((insumo1Posicao != -1) && (insumo2Posicao != -1)) {
        
            esperar = false;

            pthread_mutex_lock(infectado->bancadaMutex);

            if ( (*infectado->atingiramObjetivo) == (TOTAL_INFECTADOS + TOTAL_LABORATORIOS) ) {
                    continuarOperando = false;
            } else {

                insumo1Posicao = -1;
                insumo2Posicao = -1;
                
                quantidade = -1;
                
                produtoVerificados = 0;
                
                i = rand() % TOTAL_PRODUTO_BANCADA;
                while ( (insumo1Posicao == -1 || insumo2Posicao == -1) && (produtoVerificados < TOTAL_PRODUTO_BANCADA) ) {
                        
                    if (sem_getvalue(&(infectado->bancada[i].quantidade), &quantidade) == 0) {

                        if ((quantidade>0) && (infectado->bancada[i].tipo == insumo1Tipo) && (insumo1Posicao == -1)) {
                            insumo1Posicao = i;
                            // printf("\nINF %d tem insumo 1 do tipo ", infectado->id);
                            // print_tipo_insumo(infectado->bancada[i].tipo);
                            // printf(" na posicao %d", insumo1Posicao);
                        } else if ((quantidade>0) && (infectado->bancada[i].tipo == insumo2Tipo) && (insumo2Posicao == -1)) {
                            insumo2Posicao = i;
                            // printf("\nINF %d tem insumo 2 do tipo ", infectado->id);
                            // print_tipo_insumo(infectado->bancada[i].tipo);
                            // printf(" na posicao %d", insumo2Posicao);
                        }

                        produtoVerificados++;
                        i++;
                        i = (i == (TOTAL_PRODUTO_BANCADA - 1) ? 0 : i);
                    }
            
                }


                if ((insumo1Posicao != -1) && (insumo2Posicao != -1)) {

                    sem_wait(&(infectado->bancada[insumo1Posicao].quantidade));
                    sem_wait(&(infectado->bancada[insumo2Posicao].quantidade));

                    infectado->ciclosAtual++;

                    printf("\nINF %d fez a vacina %d vezes", infectado->id, infectado->ciclosAtual);

                    if (infectado->ciclosAtual == infectado->ciclosMinimos) {
                        (*infectado->atingiramObjetivo)++;
                        printf("\nINF %d alcançou objetivo", infectado->id);
                    }

                    esperar = true;

                }

            }

            pthread_mutex_unlock(infectado->bancadaMutex);

        } else {
            printf("\nINF %d não encontrou seus insumos", infectado->id); 
        }

        if(esperar==true) {
            esperar = false;
            sleep(1 + rand() % 3);
        }

        sleep(1 + rand() % 3);
    
    }
    
    return NULL;
}



int main(int argc, char** argv) {

    /* DECLARAÇÃO DAS VARIÁVEIS */
    int i, atingiramObjetivo, ciclosMinimos;
    produto_t *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    pthread_mutex_t bancadaMutex;


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
    atingiramObjetivo = 0;
    laboratorios = malloc(sizeof(laboratorio_t) * TOTAL_LABORATORIOS);
    bancada =  malloc(sizeof(produto_t) * TOTAL_PRODUTO_BANCADA);                
    infectados = malloc(sizeof(infectado_t) * TOTAL_INFECTADOS);                            
    pthread_mutex_init(&bancadaMutex, NULL);



    /* INICIALIZA BANCADA */

    for (i=0; i < (TOTAL_PRODUTO_BANCADA); i++){
        sem_init(&(bancada[i].quantidade), 0, 0);
        bancada[i].tipo = (insumo_t) i % TOTAL_INSUMOS;
    }

    /* INICIALIZA LABORATÓRIOS */

    int quantidade1 = 0;
    int quantidade2 = 0;

    int produto = 0;
    for (i=0; i < TOTAL_LABORATORIOS; i++){

        laboratorios[i].id = i+1;
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].ciclosAtual = 0;
        laboratorios[i].atingiramObjetivo = &atingiramObjetivo;
        laboratorios[i].bancadaMutex = &bancadaMutex;
        laboratorios[i].produto1 = &(bancada[produto]);
        produto++;
        laboratorios[i].produto2 = &(bancada[produto]);
        produto++;

        // ver o valor do semaforo tbm

        if (sem_getvalue(&(laboratorios[i].produto1->quantidade), &quantidade1) == 0){
            printf("\nquantidade1 >>%d", quantidade1);
        }
        if (sem_getvalue(&(laboratorios[i].produto2->quantidade), &quantidade2) == 0){
            printf("\nquantidade2 >>%d", quantidade2);
        }
        print_laboratorio(laboratorios[i].id, laboratorios[i].produto1->tipo);
        print_laboratorio(laboratorios[i].id, laboratorios[i].produto2->tipo);

    }



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



    /* APRESENTA O RESULTADO */

    for (i = 0; i < TOTAL_LABORATORIOS; i++) {
        printf("\n>>> laboratorio %d: %d\n", laboratorios[i].id, laboratorios[i].ciclosAtual); 
    }

    for (i = 0; i < TOTAL_INFECTADOS; i++) {
        printf("\n>>> infectado %d: %d\n", infectados[i].id, infectados[i].ciclosAtual); 
    }



    /* DESTRÓI MEMÓRIA ALOCADA */
    
    pthread_mutex_destroy(&bancadaMutex);
    free(laboratorios); 
    free(infectados); 
    free(bancada); 

    return 0;
}