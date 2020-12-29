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

#define VIRUS 1
#define INJECAO 2
#define ELEMENTOX 3


typedef struct {
    pthread_t thread;
    int id;  
    int *bancada;
    int indiceInicial;
    int ciclosMinimos;                              
    pthread_mutex_t *bancadaMutex;             
    pthread_cond_t *condicionalLaboratorio;
} laboratorio_t;


typedef struct {
    pthread_t thread;
    int id;
    int *bancada;                 
    int bolsa[TAMANHO_REPOSITORIO];
    int ciclosMinimos;                      
    pthread_mutex_t *bancadaMutex;    
    pthread_cond_t *condicionalLaboratorio;    
} infectado_t; // obs.: poderia guardar o valor do item ele ja tem


int falta_insumo(int * bancada, int posicao) {

    int totalFaltante = 0;

    if (bancada[posicao] == 0) {
        totalFaltante += 1;
    }

    posicao++;
    if (bancada[posicao] == 0) {
        totalFaltante += 1;
    }

    posicao++;
    if (bancada[posicao] == 0) {
        totalFaltante += 1;
    }

    return (totalFaltante == 2) ? 1 : 0;
} 



void mostra_bancada(int * bancada, int labID, int posicaoInicialBancada) {

    // vai para posição do vírus e imprime
    int posicao = posicaoInicialBancada;
    if (bancada[posicao] == -1 ) {
        printf("\n#LAB[%d] diz: 'Na posição=%d da bancada nº%d não produz vírus'\n", labID, posicao, labID);
    } else {
        printf("\n#LAB[%d] diz: 'Na posição=%d da bancada nº%d há %d vírus'\n", labID, posicao, labID, bancada[posicao]);
    }     
    // vai para posição do injecao e imprime       
    posicao++; 
    if (bancada[posicao] == -1 ) {
        printf("#LAB[%d] diz: 'Na posição=%d da bancada nº%d não produz injeção'\n", labID, posicao,labID);
    } else {
        printf("#LAB[%d] diz: 'Na posição=%d da bancada nº%d há %d injeção'\n", labID, posicao, labID, bancada[posicao]);
    }
    // vai para posição do elementoX e imprime
    posicao++; 
    if (bancada[posicao] == -1 ) {
        printf("#LAB[%d] diz: 'Na posição=%d da bancada nº%d não produz elementoX'\n\n", labID, posicao,labID);
    } else {
        printf("#LAB[%d] diz: 'Na posição=%d da bancada nº%d há %d elementoX'\n\n", labID, posicao, labID, bancada[posicao]);
    }

}


void mostra_bolsa(int * bolsa, int infID) {

    // imprime virus na bolsa
    int posicao = 0;
    if (bolsa[posicao] == -1) {
        printf("\n~INF[%d] diz: 'Na posição=%d da bolsa nº%d há infinitos vírus'\n", infID, posicao+1, infID);
    } else {
        printf("\n~INF[%d] diz: 'Na posição=%d da bolsa nº%d há %d vírus'\n", infID, posicao+1, infID, bolsa[posicao]);
    }
    // imprime injecao na bolsa
    posicao++;
    if (bolsa[posicao] == -1) {
        printf("~INF[%d] diz: 'Na posição=%d da bolsa nº%d há infinitos injeção'\n", infID, posicao+1, infID);
    } else {
        printf("~INF[%d] diz: 'Na posição=%d da bolsa nº%d há %d injeção'\n", infID, posicao+1, infID, bolsa[posicao]);
    }
    // imprime elementoX na bolsa
    posicao++;
    if (bolsa[posicao] == -1) {
        printf("~INF[%d] diz: 'Na posição=%d da bolsa nº%d há infinitos elementoX'\n\n", infID, posicao+1, infID);
    } else {
        printf("~INF[%d] diz: 'Na posição=%d da bolsa nº%d há %d elementoX'\n\n", infID, posicao+1, infID, bolsa[posicao]);
    }

}

// Permite que os laboratorios produzam os insumos
void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    int ciclosAtualProducao = 0;
    int quantEmEstoque = 0;
    int continuarOperando = TRUE;
    int posicao=0;

    printf("\n#LAB[%d] diz: 'Abri!'\n", laboratorio->id); 

    while (continuarOperando == TRUE) {

        pthread_mutex_lock(laboratorio->bancadaMutex);

        /*
        * IMPRIME A BANCADA DO LABORATÓRIO
        */

        mostra_bancada(laboratorio->bancada, laboratorio->id, laboratorio->indiceInicial);


        pthread_mutex_unlock(laboratorio->bancadaMutex);
    
        sleep(10 + rand() % 20);

    }
    
    printf("\nLAB[%d] Diz: 'Acabei de fechar :('\n", laboratorio->id);

    return NULL;
}


// Permite que os infectados consumam os insumos
void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;

    int ciclosAtualConsumo = 0;
    int quantInsumoFaltante = 0;
    int totalEstoque = 0;
    int i = 0;
    int k = 0;
    int posicao = 0;
    int continuarOperando = TRUE;
    
    printf("\n~INF[%d] diz: 'cheguei!'\n", infectado->id); 

    while (continuarOperando == TRUE) {
        

        // consumir em par

        pthread_mutex_lock(infectado->bancadaMutex);

        /*
        * IMPRIME A BOLSA DO INFECTADO
        */

        mostra_bolsa(infectado->bolsa, infectado->id);

        i = 1;
        int contador = 0;
        int faltaInsumo = 0;
        int encontrou = 0;
        int terminou = 0;
        int idLab = 0;
        while( encontrou == 0 && terminou == 0) {
            idLab = (i-1)/TAMANHO_REPOSITORIO;
            faltaInsumo = falta_insumo(infectado->bancada, i);
            if (faltaInsumo == 1) {
                printf("~INF[%d] diz: 'O estoque estah vazio no LAB[%d]'\n", infectado->id, idLab);
            }
            i += 3;
            i = (i > infectado->bancada[0]) ? 1 : i;
            contador++;


            if (contador == ((infectado->bancada[0] - 1)/3)) {
                terminou = 1;
            }

        }

        pthread_mutex_unlock(infectado->bancadaMutex);
    
        sleep( 10 + rand() % 20);

    }
    
    printf("\nINFECTADO ID [%d] saiu\n", infectado->id);

    pthread_cond_broadcast(infectado->condicionalLaboratorio);

    return NULL;
    }



int main(int argc, char** argv) {

    /* DECLARACAO DAS VARIAVEIS */
    int i, tamVetor, ciclosMinimos, atigiramObjetivo, infectadoAguardando, posicaoInsumoInfinito, posicaoInsumoIndisponivel;
    int *bancada;
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
    infectadoAguardando = 0;
    posicaoInsumoIndisponivel = 0;
    posicaoInsumoInfinito = 0;
    laboratorios = malloc(sizeof(laboratorio_t) * QUANT_LABORATORIOS);
    tamVetor = ( TAMANHO_REPOSITORIO * QUANT_LABORATORIOS) + 1;
    printf("\nº%d\n", tamVetor);
    bancada =  malloc(sizeof(int) * tamVetor);                // inicializa todos vetor com valor zero
    infectados = malloc(sizeof(infectado_t) * QUANT_INFECTADOS);
    bancada[0] = tamVetor;                                  // Inicializa a primeira posicao do vetor
    sem_init(&objetivoMutex, 0, 1);
    pthread_mutex_init(&bancadaMutex, NULL);
    pthread_cond_init(&condicionalLaboratorio, NULL);
    pthread_cond_init(&condicionalInfectado, NULL);


    /* INICIALIZA LABORATORIOS */
    // for (posicao=1; i < tamVetor; posicao= i + TAMANHO_REPOSITORIO){
    int posicaoInsumo = 1;
    for (i=0; i < QUANT_LABORATORIOS; i++){

        laboratorios[i].id = i+1;
        laboratorios[i].bancada = bancada;
        laboratorios[i].indiceInicial = posicaoInsumo;
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].bancadaMutex = &bancadaMutex;
        laboratorios[i].condicionalLaboratorio = &condicionalLaboratorio;        

        /* INICIALIZA REPOSITORIO */
        for(int k=0; k < TAMANHO_REPOSITORIO; k++) {
            if (k == posicaoInsumoIndisponivel) {
                bancada[posicaoInsumo] = INSUMO_INDISPONIVEL;
            } else {
                bancada[posicaoInsumo] = 1;
            }
            posicaoInsumo++;
        }

        posicaoInsumoIndisponivel++;
        posicaoInsumoIndisponivel = posicaoInsumoIndisponivel % TAMANHO_REPOSITORIO;
    }



    /* INICIALIZA INFECTADOS */
    for (i=0; i < QUANT_INFECTADOS; i++){
        infectados[i].id = i+1;
        infectados[i].bancada = bancada;             
        infectados[i].ciclosMinimos = ciclosMinimos;       
        infectados[i].bancadaMutex = &bancadaMutex;        
        infectados[i].condicionalLaboratorio = &condicionalLaboratorio;        

        /* INICIALIZA REPOSITORIO */
        for(int k=0; k < TAMANHO_REPOSITORIO; k++) {
            if (k == posicaoInsumoInfinito) {
                infectados[i].bolsa[k] = INSUMO_INFINITO;
            } else {
                infectados[i].bolsa[k] = 0;
            }
        }


        posicaoInsumoInfinito++;
        posicaoInsumoInfinito = posicaoInsumoInfinito % TAMANHO_REPOSITORIO;

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