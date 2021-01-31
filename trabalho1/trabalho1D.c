#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

#define QUANT_INFECTADOS 3
#define QUANT_LABORATORIOS 3
#define TAMANHO_REPOSITORIO 3

#define INSUMO_INFINITO -1
#define INSUMO_INDISPONIVEL -1


typedef int bool;

typedef struct {
    pthread_t thread;
    int id;  
    int *bancada;
    int indiceInicial;
    int ciclosMinimos;
    int ciclosAtual;
    int *atingiramObjetivo;                               
    pthread_mutex_t *bancadaMutex;             
    pthread_cond_t *laboratorioCondicional;
    pthread_cond_t *infectadoCondicional;
} laboratorio_t;


typedef struct {
    pthread_t thread;
    int id;
    int *bancada;                 
    int bolsa[TAMANHO_REPOSITORIO];
    int ciclosMinimos;    
    int ciclosAtual;    
    int *atingiramObjetivo;                  
    int *contadorBarreira;                  
    pthread_mutex_t *bancadaMutex;    
    pthread_cond_t *laboratorioCondicional;    
    pthread_cond_t *infectadoCondicional;    
} infectado_t; 


/* gera um valor de indice 'aleatorio' para iniciar a busca de insumo no vetor (bancada) */
int gera_indice(int intervalo_max, int x) {
    int numero = 0;
    numero = rand() % intervalo_max;
    return numero - (numero % x) + 1;
}


/* verifica se a bancada do laboratório está sem insumo */
bool bancada_sem_insumos(int * bancada, int posicao) {

    /* capacidade máxima da bancada é 2 insumos por laboratório, desconsiderando o insumo indisponível */
    int itens_na_bancada = 2;


    /* contagem de vírus morto */
    if (bancada[posicao] == 0) {
        itens_na_bancada--;
    }

    /* contagem de injeção */
    posicao++;
    if (bancada[posicao] == 0) {
        itens_na_bancada--;
    }

    /* contagem de elementox */
    posicao++;
    if (bancada[posicao] == 0) {
        itens_na_bancada--;
    }

    return (itens_na_bancada == 0) ? TRUE : FALSE;
} 

/* adiciona os insumos na posição da bancada pertencete a um laboratório */
void repor_bancada(int * bancada, int labID, int posicaoInicialBancada) {

    /* adiciona vírus */
    int posicao = posicaoInicialBancada;
    if (bancada[posicao] != INSUMO_INDISPONIVEL && bancada[posicao] == 0) {
        bancada[posicao] = 1;
    }    

    /* adiciona injeção */
    posicao++; 
    if (bancada[posicao] != INSUMO_INDISPONIVEL && bancada[posicao] == 0) {
        bancada[posicao] = 1;
    }   

    /* adiciona elementoX */
    posicao++; 
    if (bancada[posicao] != INSUMO_INDISPONIVEL && bancada[posicao] == 0) {
        bancada[posicao] = 1;
    }   
}


/* verifica se a bolsa do infectado está cheia de insumos */
bool bolsa_estah_cheia(int * bolsa) {

    /* capacidade máxima da bolsa é 2 insumos, desconsiderando o insumo infinito */
    int capacidade_da_bolsa = 2;

    int total_itens = 0;

    /* contagem de vírus morto */
    if (bolsa[0] > 0) {
       total_itens++;
    }

    /* contagem de injeção */
    if (bolsa[1] > 0) {
        total_itens++;
    }

    /* contagem de elementoX */
    if (bolsa[2] > 0) {
        total_itens++;
    } 

    return (capacidade_da_bolsa == total_itens) ? TRUE : FALSE;
}


/* esvazia a bolsa dos infectados */
void esvaziar_bolsa(int * bolsa) {

    /* esvazia virus */
    if (bolsa[0] != INSUMO_INFINITO) {
        bolsa[0] = 0;
    }

    /* esvazia injecao */
    if (bolsa[1] != INSUMO_INFINITO) {
        bolsa[1] = 0;
    }

    /* esvazia elementoX */
    if (bolsa[2] != INSUMO_INFINITO) {
        bolsa[2] = 0;
    } 
}


/* permite que os laboratorios produzam os insumos */
void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    int quantEmEstoque = 0;
    bool continuarOperando = TRUE;
    bool sem_insumos = FALSE;
    int posicao=0;

    while (continuarOperando == TRUE) {

        pthread_mutex_lock(laboratorio->bancadaMutex);

        if ( (*laboratorio->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS) ) {
            
            /* QUANDO TODOS ATINGIRAM O OBJETIVO */

            /* flag indica se a thread laboratório deve terminar ou continuar a execução */
            continuarOperando = FALSE;  

            /* envia um sinal para acordar todas as threads laboratorio que podem estar dormindo */
            for (int l = 0; l < QUANT_LABORATORIOS; l++) {
                pthread_cond_signal(&(laboratorio->laboratorioCondicional[l]));
            }

            /* envia um sinal para acordar todas as threads infectados que podem estar dormindo */
            pthread_cond_broadcast(laboratorio->infectadoCondicional);

        } else {

            sem_insumos = bancada_sem_insumos(laboratorio->bancada, laboratorio->indiceInicial);

            if (sem_insumos == TRUE) {

                /* QUANDO O LABORATÓRIO REPÕE O ESTOQUE */

                repor_bancada(laboratorio->bancada, laboratorio->id, laboratorio->indiceInicial);
                laboratorio->ciclosAtual += 1;
                if (laboratorio->ciclosAtual == laboratorio->ciclosMinimos) {
                    (*laboratorio->atingiramObjetivo) += 1;
                }

                printf("\n#LAB[%d] restabeleceu o estoque %d vezes.\n", laboratorio->id, laboratorio->ciclosAtual);
            }

            while ( pthread_cond_wait (&(laboratorio->laboratorioCondicional[(laboratorio->id - 1)]), laboratorio->bancadaMutex) != 0 );
        }

        pthread_mutex_unlock(laboratorio->bancadaMutex);
    }
    
    return NULL;
}


// Permite que os infectados consumam os insumos
void* f_infectado (void* argumento) {

    infectado_t *infectado = (infectado_t *)argumento;

    int quantInsumoFaltante = 0;
    int totalEstoque = 0;
    int i = 0;
    int k = 0;
    bool continuarOperando = TRUE;
    int tamanhoVetor = 0;
    bool conseguiu_consumir = FALSE;
    int totalLaboratoriosVisitados = 0;
    bool falta_insumo = FALSE;
    int consumiu = 0;
    bool ja_consumiu = FALSE;
    bool deve_esperar = FALSE;
    bool visitouTodosLabs = FALSE;
    int idLab = 0;
    int posicaoNaBancada = 0;   

    while (continuarOperando == TRUE) {
           
        pthread_mutex_lock(infectado->bancadaMutex);

        totalLaboratoriosVisitados = 0;
        falta_insumo = FALSE;
        consumiu = 0;
        ja_consumiu = FALSE;
        deve_esperar = FALSE;
        visitouTodosLabs = FALSE;
        idLab = 0;
        posicaoNaBancada = 0;
        

        if ( (*infectado->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS) ) {
            
            /* QUANDO TODOS ATINGIRAM O OBJETIVO */

            /* flag indica se a thread infectado deve terminar ou continuar a execução */
            continuarOperando = FALSE;    

            /* envia um sinal para acordar todas as threads infectados que podem estar dormindo */
            pthread_cond_broadcast(infectado->infectadoCondicional);

            /* envia um sinal para acordar todas as threads laboratorio que podem estar dormindo */
            for (int l = 0; l < QUANT_LABORATORIOS; l++) {
                pthread_cond_signal(&(infectado->laboratorioCondicional[l]));
            }

        } else {
            
            /* pega o tamanho do vetor */
            tamanhoVetor = infectado->bancada[0];

            /* gera um valor de indice 'aleatorio' para iniciar a busca de insumo no vetor (bancada) */
            i = gera_indice(tamanhoVetor, TAMANHO_REPOSITORIO);            


            while( bolsa_estah_cheia(infectado->bolsa) != TRUE && visitouTodosLabs == FALSE) {

                /* flag para indicar se o infectado ja consumiu ou nao insumo em alguma posicao da bancada do laboratorio */
                ja_consumiu = FALSE;
                
                /* garante que o valor atribuido a variavel 'i' seja correto */
                i = (i >= (tamanhoVetor - 1)) ? 1 : i;
                i = (i == 0) ? 1 : i;
                

                /* calculo para encontrar o id do laboratorio */
                idLab = ((i-1)/TAMANHO_REPOSITORIO) + 1;


                /* posicao do virus morto */
                posicaoNaBancada = i;
                if ( (infectado->bolsa[0] == 0)  &&  (infectado->bancada[posicaoNaBancada] > 0) && (ja_consumiu == FALSE)) {
                    infectado->bancada[posicaoNaBancada] = 0;
                    infectado->bolsa[0] = 1;
                    ja_consumiu = TRUE;
                }

                /* posicao da injecao */
                posicaoNaBancada++;
                if ( (infectado->bolsa[1] == 0)  &&  (infectado->bancada[posicaoNaBancada] > 0) && (ja_consumiu == FALSE)) {
                    infectado->bancada[posicaoNaBancada] = 0;
                    infectado->bolsa[1] = 1;
                    ja_consumiu = TRUE;
                }

                /* posicao do elementox */
                posicaoNaBancada++;
                if ( (infectado->bolsa[2] == 0)  &&  (infectado->bancada[posicaoNaBancada] > 0) && (ja_consumiu == FALSE)) {
                    infectado->bancada[posicaoNaBancada] = 0;
                    infectado->bolsa[2] = 1;
                    ja_consumiu = TRUE;
                }

                /* verifica se o laboratorio tem ou nao insumo */
                falta_insumo = bancada_sem_insumos(infectado->bancada, i);

                if (falta_insumo == TRUE) {
                    /* envia um sinal para acordar a thread laboratorio para produzir insumo */
                    pthread_cond_signal(&(infectado->laboratorioCondicional[idLab-1]));
                }

                i += 3;
                totalLaboratoriosVisitados++;

                if (totalLaboratoriosVisitados == ((tamanhoVetor - 1)/TAMANHO_REPOSITORIO)) {
                    /* flag para indicar que o infectado visitou ou não todas as bancadas dos labaratorios */
                    visitouTodosLabs = TRUE;
                }

            }

            if ( bolsa_estah_cheia(infectado->bolsa) == TRUE ) {
                
                /* QUANDO O INFECTADO COMPLETOU A VACINA */

                esvaziar_bolsa(infectado->bolsa);

                conseguiu_consumir = TRUE;

                infectado->ciclosAtual += 1;
                if (infectado->ciclosAtual == infectado->ciclosMinimos ) {
                    (*infectado->atingiramObjetivo) += 1;
                }
                
                printf("\n~INF[%d] completou a vacina %d vezes.\n", infectado->id, infectado->ciclosAtual);
                        
                if ( (*infectado->contadorBarreira) == (QUANT_INFECTADOS-1) ) {
                    
                    /* QUANDO A ÚLTIMA THREAD INFECTADO COMPLETA A VACINA */

                    (*infectado->contadorBarreira) = 0;

                    /* envia um sinal para acordar todas as threads infectados que podem estar dormindo */
                    pthread_cond_broadcast(infectado->infectadoCondicional);
                    
                    /* flag para indicar que a thread deve esperar alguns segudos antes de tentar visitar a bancada */
                    deve_esperar = TRUE;

                } else {
                    
                    /* QUANDO AS DUAS PRIMEIRAS THREADS INFECTADOS COMPLETAM A VACINA */

                    (*infectado->contadorBarreira)++;
                    
                    /* coloca as threads infectados para dormir */
                    while ( pthread_cond_wait(infectado->infectadoCondicional, infectado->bancadaMutex) != 0 );
                }

            } else {

                /* QUANDO O INFECTADO NÃO COMPLETOU A VACINA */

                /* flag para indicar que a thread deve esperar alguns segudos antes de tentar visitar a bancada */
                deve_esperar = TRUE;

                /* se for a segunda vez que o infectado tentou e não conseguiu completar a vacina, então acorda os outros infectados */
                if (conseguiu_consumir == FALSE) {
                    pthread_cond_broadcast(infectado->infectadoCondicional);
                }
                
                /* flag para indicar que o infectado não conseguiu consumir os insumos, ou seja, NÃO completou a vacina */
                conseguiu_consumir = FALSE;
            }

        }

        pthread_mutex_unlock(infectado->bancadaMutex);

        if (deve_esperar == TRUE) {
            deve_esperar = FALSE;
            sleep(1);
        }
    
    }
    
    return NULL;
}



int main(int argc, char** argv) {

    /* DECLARAÇÃO DAS VARIAVEIS */
    int i, tamVetor, atingiramObjetivo, ciclosMinimos, contadorBarreira, infectadoAguardando, posicaoInsumoInfinito, posicaoInsumoIndisponivel;
    int *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    sem_t objetivoMutex;
    pthread_cond_t *laboratorioCondicional, infectadoCondicional;
    pthread_mutex_t bancadaMutex;


    /* VALIDAÇÃO DE ENTRADAS */
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
    atingiramObjetivo = 0;
    contadorBarreira = 0;
    infectadoAguardando = 0;
    posicaoInsumoIndisponivel = 0;
    posicaoInsumoInfinito = 0;
    laboratorios = malloc(sizeof(laboratorio_t) * QUANT_LABORATORIOS);
    laboratorioCondicional = malloc(sizeof(pthread_cond_t) * QUANT_LABORATORIOS);
    tamVetor = ( TAMANHO_REPOSITORIO * QUANT_LABORATORIOS) + 1;
    bancada =  malloc(sizeof(int) * tamVetor);                
    infectados = malloc(sizeof(infectado_t) * QUANT_INFECTADOS);
    bancada[0] = tamVetor;                                  
    sem_init(&objetivoMutex, 0, 1);
    pthread_mutex_init(&bancadaMutex, NULL);
    pthread_cond_init(&infectadoCondicional, NULL);


    /* INICIALIZA LABORATORIOS */

    for (i=0; i < QUANT_LABORATORIOS; i++) {
        pthread_cond_init(&(laboratorioCondicional[i]), NULL);
    }


    int posicaoInsumo = 1;
    for (i=0; i < QUANT_LABORATORIOS; i++){

        laboratorios[i].id = i+1;
        laboratorios[i].bancada = bancada;
        laboratorios[i].indiceInicial = posicaoInsumo;
        laboratorios[i].ciclosMinimos = ciclosMinimos;
        laboratorios[i].ciclosAtual = 0;
        laboratorios[i].atingiramObjetivo = &atingiramObjetivo;
        laboratorios[i].bancadaMutex = &bancadaMutex;
        laboratorios[i].laboratorioCondicional = laboratorioCondicional;
        laboratorios[i].infectadoCondicional = &infectadoCondicional;

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
        infectados[i].ciclosAtual = 0;       
        infectados[i].contadorBarreira = &contadorBarreira;       
        infectados[i].bancadaMutex = &bancadaMutex;
        infectados[i].laboratorioCondicional = laboratorioCondicional;
        infectados[i].infectadoCondicional = &infectadoCondicional;
        infectados[i].atingiramObjetivo = &atingiramObjetivo;

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


    /* EXECUTA AS THREADS */   

    for (i = 0; i < QUANT_LABORATORIOS; i++) {
        pthread_create(&(laboratorios[i].thread), NULL, f_laboratorio, &(laboratorios[i]));
    }

    for (i = 0; i < QUANT_INFECTADOS; i++) {
        pthread_create(&(infectados[i].thread), NULL, f_infectado, &(infectados[i]));
    }


    /* ESPERA AS THREADS */    

    for (i = 0; i < QUANT_LABORATORIOS; i++) {
        pthread_join(laboratorios[i].thread, NULL);
    }

    for (i = 0; i < QUANT_INFECTADOS; i++) {
        pthread_join(infectados[i].thread, NULL);
    }



    /* MOSTRA O RESULTADO */

    for (i = 0; i < QUANT_LABORATORIOS; i++) {
        printf("\n>>> laboratorio %d: %d\n", laboratorios[i].id, laboratorios[i].ciclosAtual); 
    }

    for (i = 0; i < QUANT_INFECTADOS; i++) {
        printf("\n>>> infectado %d: %d\n", infectados[i].id, infectados[i].ciclosAtual); 
    }


    /* DESTROI MEMORIA ALOCADA */
    
    // sem_destroy(&objetivoMutex);
    // pthread_mutex_destroy(&bancadaMutex);
    // pthread_cond_destroy(&condicionalLaboratorio);    
    // pthread_cond_destroy(&condicionalInfectado);    

    return 0;
}