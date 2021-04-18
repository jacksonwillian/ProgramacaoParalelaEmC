
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
    
    int i = 0;

    while(true) {

        while( pthread_create(&(package->thread), NULL, f_opening, package) != 0) {
            printf("Erro ao criar thread %d \n", i);
            sleep(10);
        } 
            
        while (pthread_detach(package->thread) != 0) {
            printf("\nErro ao separara thread %d \n", i);
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