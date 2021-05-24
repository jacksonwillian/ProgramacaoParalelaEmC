
#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef enum {
    false = 0,
    true = 1
} bool;


typedef struct {
    pthread_t thread;
} package_t;


void* f_opening(void* argumento);


int main(int argc, char** argv) {

    package_t * package;

    package = malloc(sizeof(package_t));
    
    pthread_attr_t tattr;


    /* inicializa variavel atributo thread */
    if (pthread_attr_init(&tattr) != 0) {
        printf("Erro ao inicializar o thread atributo \n");
        return -1;
    }

    /* define atributo para detached (recursos podem ser reutilizados assim que o encadeamento termina) */
    if (pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED) != 0) {
        printf("Erro ao definir atributo da thread \n");
        return -1;
    }

    int i = 0;

    while(true) {

        while( pthread_create(&(package->thread), &tattr, f_opening, package) != 0) {
            printf("Erro ao criar thread %d \n", i);
            sleep(10);
        } 

        /* Nao eh possivel realizar join para thread detach
            while (pthread_join(package->thread, NULL) != 0) {
                printf("\nErro ao unir thread %d \n", i);
                sleep(10);
            }
        */
       
        i++;
    }
   
    pthread_exit(0);
}


void* f_opening(void* argumento) {
    sleep(5);
    return NULL;
}