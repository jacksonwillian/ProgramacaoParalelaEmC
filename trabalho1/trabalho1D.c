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
} infectado_t; // obs.: poderia guardar o valor do item ele ja tem


int gera_multiplo_x(int intervalo_max, int x) {
    int numero = 0;
    numero = rand() % intervalo_max;
    return numero - (numero % x) + 1;
}

bool bancada_sem_insumos(int * bancada, int posicao) {

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

    return (totalFaltante == 2) ? TRUE : FALSE;
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

void repor_bancada(int * bancada, int labID, int posicaoInicialBancada) {

    // vai para posição do vírus e imprime
    int posicao = posicaoInicialBancada;
    if (bancada[posicao] != INSUMO_INDISPONIVEL && bancada[posicao] == 0) {
        bancada[posicao] = 1;
    }    
    // vai para posição do injecao e imprime       
    posicao++; 
    if (bancada[posicao] != INSUMO_INDISPONIVEL && bancada[posicao] == 0) {
        bancada[posicao] = 1;
    }   

    // vai para posição do elementoX e imprime
    posicao++; 
    if (bancada[posicao] != INSUMO_INDISPONIVEL && bancada[posicao] == 0) {
        bancada[posicao] = 1;
    }   
    
    printf("\n#LAB[%d] diz: 'Repus o estoque'\n", labID);

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

bool bolsa_estah_cheia(int * bolsa) {

    int total_itens = 0;

    // conta virus 
    if (bolsa[0] > 0) {
       total_itens++;
    }
    // conta injecao 
    if (bolsa[1] > 0) {
        total_itens++;
    }
    // conta elementoX 
    if (bolsa[2] > 0) {
        total_itens++;
    } 

    return (total_itens == 2) ? TRUE : FALSE;

}

void esvaziar_bolsa(int * bolsa) {

    // esvazia virus 
    if (bolsa[0] != INSUMO_INFINITO) {
        bolsa[0] = 0;
    }
    // esvazia injecao 
    if (bolsa[1] != INSUMO_INFINITO) {
        bolsa[1] = 0;
    }
    // esvazia elementoX 
    if (bolsa[2] != INSUMO_INFINITO) {
        bolsa[2] = 0;
    } 

}

// Permite que os laboratorios produzam os insumos
void* f_laboratorio (void* argumento) {

    laboratorio_t *laboratorio = (laboratorio_t *)argumento;

    int quantEmEstoque = 0;
    bool continuarOperando = TRUE;
    bool sem_insumos = FALSE;
    int posicao=0;

    // printf("\n#LAB[%d] diz: 'Abri!'\n", laboratorio->id); 

    while (continuarOperando == TRUE) {


        pthread_mutex_lock(laboratorio->bancadaMutex);

        if ( (*laboratorio->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS) ) {
            continuarOperando = FALSE;  

            for (int l = 0; l < QUANT_LABORATORIOS; l++) {
                pthread_cond_signal(&(laboratorio->laboratorioCondicional[l]));
            }

            pthread_cond_broadcast(laboratorio->infectadoCondicional);

        } else {

            /*
            * IMPRIME A BANCADA DO LABORATÓRIO
            */
            sem_insumos = bancada_sem_insumos(laboratorio->bancada, laboratorio->indiceInicial);

            if (sem_insumos == TRUE) {
                repor_bancada(laboratorio->bancada, laboratorio->id, laboratorio->indiceInicial);
                laboratorio->ciclosAtual += 1;
                if (laboratorio->ciclosAtual == laboratorio->ciclosMinimos) {
                    (*laboratorio->atingiramObjetivo) += 1;
                }

                printf("\n#LAB[%d] Diz: 'ciclo atual %d'\n", laboratorio->id, laboratorio->ciclosAtual);
            }

            mostra_bancada(laboratorio->bancada, laboratorio->id, laboratorio->indiceInicial);


            printf("\n#LAB[%d] diz: 'Vou dar uma pausa, me avise quando os insumos terminarem...'\n", laboratorio->id); 
            while ( pthread_cond_wait (&(laboratorio->laboratorioCondicional[(laboratorio->id - 1)]), laboratorio->bancadaMutex) != 0 );
            printf("\n#LAB[%d] diz: 'Opa! Acordei, vou repor os insumos.'\n", laboratorio->id); 

        }


        pthread_mutex_unlock(laboratorio->bancadaMutex);
        
        
    }
    
    printf("\n#LAB[%d] Diz: 'Acabei de fechar :('\n", laboratorio->id);

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
    bool nao_conseguiu_consumir = FALSE;
    
    // printf("\n~INF[%d] diz: 'cheguei!'\n", infectado->id); 

    while (continuarOperando == TRUE) {
    
        // consumir em par

        pthread_mutex_lock(infectado->bancadaMutex);


        int totalLaboratoriosVisitados = 0;
        bool falta_insumo = FALSE;
        int consumiu = 0;
        bool ja_consumiu = FALSE;
        bool deve_esperar = FALSE;
        bool visitouTodosLabs = FALSE;
        int idLab = 0;
        int posicaoNaBancada = 0;
        



        if ( (*infectado->atingiramObjetivo) == (QUANT_LABORATORIOS + QUANT_INFECTADOS) ) {
            continuarOperando = FALSE;    

            pthread_cond_broadcast(infectado->infectadoCondicional);

            for (int l = 0; l < QUANT_LABORATORIOS; l++) {
                pthread_cond_signal(&(infectado->laboratorioCondicional[l]));
            }

        } else {


            /*
            * IMPRIME A BOLSA DO INFECTADO
            */

            mostra_bolsa(infectado->bolsa, infectado->id);

            tamanhoVetor = infectado->bancada[0];

            i = gera_multiplo_x(tamanhoVetor, TAMANHO_REPOSITORIO);            

            printf("\n~INF[%d] diz: 'vou buscar meus insumos!'\n", infectado->id); 

            while( bolsa_estah_cheia(infectado->bolsa) != TRUE && visitouTodosLabs == FALSE) {
                
                i = (i >= (tamanhoVetor - 1)) ? 1 : i;
                i = (i == 0) ? 1 : i;
                
                idLab = ((i-1)/TAMANHO_REPOSITORIO) + 1;
                printf("\n~INF[%d] diz: 'Estou no LAB[%d]'\n", infectado->id, idLab);

                ja_consumiu = FALSE;

                posicaoNaBancada = i;
                if ( (infectado->bolsa[0] == 0)  &&  (infectado->bancada[posicaoNaBancada] > 0) && (ja_consumiu == FALSE)) {
                    printf("\n~INF[%d] diz: 'peguei um virus!'\n", infectado->id); 
                    infectado->bancada[posicaoNaBancada] = 0;
                    infectado->bolsa[0] = 1;
                    ja_consumiu = TRUE;
                }
                posicaoNaBancada++;
                if ( (infectado->bolsa[1] == 0)  &&  (infectado->bancada[posicaoNaBancada] > 0) && (ja_consumiu == FALSE)) {
                    printf("\n~INF[%d] diz: 'peguei uma injeção!'\n", infectado->id);                 
                    infectado->bancada[posicaoNaBancada] = 0;
                    infectado->bolsa[1] = 1;
                    ja_consumiu = TRUE;
                }
                posicaoNaBancada++;
                if ( (infectado->bolsa[2] == 0)  &&  (infectado->bancada[posicaoNaBancada] > 0) && (ja_consumiu == FALSE)) {
                    printf("\n~INF[%d] diz: 'peguei um elementoX!'\n", infectado->id);                 
                    infectado->bancada[posicaoNaBancada] = 0;
                    infectado->bolsa[2] = 1;
                    ja_consumiu = TRUE;
                }


                falta_insumo = bancada_sem_insumos(infectado->bancada, i);
                if (falta_insumo == TRUE) {
                    printf("\n~INF[%d] diz: 'O estoque estah vazio no LAB[%d]'\n", infectado->id, idLab);
                    // printf("\n~INF[%d] diz: 'Vou acordar o LAB[%d]'\n", infectado->id, idLab);
                    // pthread_cond_signal(&(infectado->laboratorioCondicional[idLab-1]));
                }

                i += 3;
                totalLaboratoriosVisitados++;

                if (totalLaboratoriosVisitados == ((tamanhoVetor - 1)/TAMANHO_REPOSITORIO)) {
                    visitouTodosLabs = TRUE;
                }

            }

            if ( bolsa_estah_cheia(infectado->bolsa) == TRUE ) {

                printf("\n~INF[%d] diz: 'Opa produzi minha vacina'\n", infectado->id);
                
                esvaziar_bolsa(infectado->bolsa);

                nao_conseguiu_consumir = FALSE;


                infectado->ciclosAtual += 1;
                if (infectado->ciclosAtual == infectado->ciclosMinimos ) {
                    (*infectado->atingiramObjetivo) += 1;
                }
                
                // remover depois
                printf("\n~INF[%d] diz: 'ciclo atual %d'", infectado->id, infectado->ciclosAtual);
                        
                printf("\n~INF[%d] diz: 'Terminei o que eu tinha que fazer!'\n", infectado->id); 

                if ( (*infectado->contadorBarreira) == (QUANT_INFECTADOS-1) ) {

                    (*infectado->contadorBarreira)++;

                    printf("\n~INF[%d] diz: '$A Completaram ao todo ciclo  %d infectados!'\n", infectado->id, (*infectado->contadorBarreira));

                    (*infectado->contadorBarreira) = 0;

                    pthread_cond_broadcast(infectado->infectadoCondicional);

                    deve_esperar = TRUE;


                } else {

                    (*infectado->contadorBarreira)++;

                    printf("\n~INF[%d] diz: '$B Completaram ao todo ciclo  %d infectados!'\n", infectado->id, (*infectado->contadorBarreira));
                    
                    if ( (*infectado->atingiramObjetivo) != (QUANT_LABORATORIOS + QUANT_INFECTADOS) ) {
                        while ( pthread_cond_wait(infectado->infectadoCondicional, infectado->bancadaMutex) != 0 );
                    }



                }

            
            } else {

                printf("\n~INF[%d] diz: 'Poxa, não produzi minha vacina'\n", infectado->id);

                deve_esperar = TRUE;

                
                if (nao_conseguiu_consumir == TRUE) {
                    printf("\n~INF[%d] diz: 'Precisei chamar os outros!'\n", infectado->id);
                    pthread_cond_broadcast(infectado->infectadoCondicional);
                }
                
                // pthread_cond_broadcast(infectado->infectadoCondicional);
                
                // printf("\n~INF[%d] diz: 'Acorda cambada'\n", infectado->id);

                printf("\n~INF[%d] diz: 'Vou acordar os LABs\n", infectado->id);

                for (int l = 0; l < QUANT_LABORATORIOS; l++) {
                    pthread_cond_signal(&(infectado->laboratorioCondicional[l]));
                }

                nao_conseguiu_consumir = TRUE;

            }

        }


        pthread_mutex_unlock(infectado->bancadaMutex);

        if (deve_esperar == TRUE) {
            deve_esperar = FALSE;
            sleep(3);
        }
    

    }
    
    printf("\n~INF[%d] saiu\n", infectado->id);

    return NULL;
    }



int main(int argc, char** argv) {

    /* DECLARACAO DAS VARIAVEIS */
    int i, tamVetor, atingiramObjetivo, ciclosMinimos, contadorBarreira, infectadoAguardando, posicaoInsumoInfinito, posicaoInsumoIndisponivel;
    int *bancada;
    laboratorio_t *laboratorios;
    infectado_t *infectados;
    sem_t objetivoMutex;
    pthread_cond_t *laboratorioCondicional, infectadoCondicional;
    pthread_mutex_t bancadaMutex;


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
    atingiramObjetivo = 0;
    contadorBarreira = 0;
    infectadoAguardando = 0;
    posicaoInsumoIndisponivel = 0;
    posicaoInsumoInfinito = 0;
    laboratorios = malloc(sizeof(laboratorio_t) * QUANT_LABORATORIOS);
    laboratorioCondicional = malloc(sizeof(pthread_cond_t) * QUANT_LABORATORIOS);
    tamVetor = ( TAMANHO_REPOSITORIO * QUANT_LABORATORIOS) + 1;
    printf("\nº%d\n", tamVetor);
    bancada =  malloc(sizeof(int) * tamVetor);                // inicializa todos vetor com valor zero
    infectados = malloc(sizeof(infectado_t) * QUANT_INFECTADOS);
    bancada[0] = tamVetor;                                  // Inicializa a primeira posicao do vetor
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


    /* EXECUTAS AS THREADS LABORATORIOS E INFECTADOS */    
    for (i = 0; i < QUANT_LABORATORIOS; i++) {
        pthread_create(&(laboratorios[i].thread), NULL, f_laboratorio, &(laboratorios[i]));
    }

    for (i = 0; i < QUANT_INFECTADOS; i++) {
        pthread_create(&(infectados[i].thread), NULL, f_infectado, &(infectados[i]));
    }

    for (i = 0; i < QUANT_LABORATORIOS; i++) {
        pthread_join(laboratorios[i].thread, NULL);
        printf("\n>>> laboratorio %d: %d", laboratorios[i].id, laboratorios[i].ciclosAtual); 
    }

    for (i = 0; i < QUANT_INFECTADOS; i++) {
        pthread_join(infectados[i].thread, NULL);
         printf("\n>>> infectado %d: %d", infectados[i].id, infectados[i].ciclosAtual); 
    }


    /* DESTROI MEMORIA ALOCADA */
    
    // sem_destroy(&objetivoMutex);
    // pthread_mutex_destroy(&bancadaMutex);
    // pthread_cond_destroy(&condicionalLaboratorio);    
    // pthread_cond_destroy(&condicionalInfectado);    

    return 0;
}