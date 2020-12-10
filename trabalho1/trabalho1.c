#include <pthread.h> 
#include<stdio.h>
#include<stdlib.h>

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
    int ciclosDeProducao;  // incrementa 1 unid. quando renova todo estoque de insumos   
    repositorio_t *repositorio;
} laboratorio_t;


typedef struct {
    pthread_t thread;
    int id;
    int ciclosDeConsumo;  // incrementa 1 unid. quando obtem todos os insumos faltante
    repositorio_t repositorio;
    repositorio_t *bancada; // referencia para os repositorios dos laboratorios
    int quantLaboratorios;
} infectado_t;



// Permite que os laboratorios produzam os insumos
void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    /* IMPRIME AS INFORMACOES DO LABORATORIO */


    return NULL;
}


// Permite que os infectados consumam os insumos
void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;

    return NULL;
}



int main(int argc, char** argv) {

    /* DECLARACAO DAS VARIAVEIS */
    int i, quantMinimaCiclos, quantLaboratorios, quantInfectados, posicaoInsumoInfinito, posicaoInsumoIndisponivel;
    repositorio_t *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    

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
    quantMinimaCiclos = atoi(argv[1]);
    quantLaboratorios = 3;    
    quantInfectados = 3;
    posicaoInsumoIndisponivel = 0;
    posicaoInsumoInfinito = 0;
    laboratorios = malloc(sizeof(laboratorio_t) * quantLaboratorios);
    bancada = malloc(sizeof(repositorio_t) * quantLaboratorios);
    infectados = malloc(sizeof(infectado_t) * quantInfectados);


    /* INICIALIZA LABORATORIOS */
    for (i=0; i < quantLaboratorios; i++){

        laboratorios[i].id = i+1;
        laboratorios[i].ciclosDeProducao = 0;
        laboratorios[i].repositorio = &(bancada[i]);

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
    for (i=0; i < quantInfectados; i++){
        infectados[i].id = i+1;
        infectados[i].ciclosDeConsumo = 0;
        infectados[i].bancada = &(bancada[i]);        
        infectados[i].quantLaboratorios = quantLaboratorios;        

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


    /* EXECUTAS AS THREADS DOS LABORATORIOS */    
    for (i = 0; i < quantLaboratorios; i++) {
        pthread_create(&(laboratorios[i].thread), NULL, f_laboratorio, &(laboratorios[i]));
    }

    for (i = 0; i < quantLaboratorios; i++) {
        pthread_join(laboratorios[i].thread, NULL);
    }


    /* EXECUTAS AS THREADS DOS INFECTADOS */    
    for (i = 0; i < quantInfectados; i++) {
        pthread_create(&(infectados[i].thread), NULL, f_infectado, &(infectados[i]));
    }

    for (i = 0; i < quantInfectados; i++) {
        pthread_join(infectados[i].thread, NULL);
    }


    /* DESTROI MEMORIA ALOCADA */
    
    return 0;
}