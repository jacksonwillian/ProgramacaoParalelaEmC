
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
    int id;
} package_t;


void* f_opening(void* argumento);


int main(int argc, char** argv) {

    package_t * package = malloc(sizeof(package_t));
    
    package->id = 3;

    pthread_create(&(package->thread), NULL, f_opening, package);

    if (pthread_detach(package->thread) != 0) {
        printf("\nErro ao separara thread\n");
        return -1;
    } // não permite dar o join

    pthread_join(package->thread, NULL);


    return 0;

    // pthread_exit(0); // espera thread teminarem
}



void* f_opening(void* argumento) {

    package_t* package = (package_t*)argumento;

    int i = 0;

    while (i < 100000) {
        printf(">>> %d\n", i);
        i++;
    }

    printf(">>> %d\n", package->id);

    return NULL;
}