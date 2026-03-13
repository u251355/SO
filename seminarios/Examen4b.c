#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>
pthread_mutex_t lock;//inicializamos el lock fuera de todo

void* increment(void *arg){
   int *i = (int*)arg;//casteamos
   pthread_mutex_lock(&lock);
   A[i]= i*1000;
   pthread_mutex_unlock(&lock);
   return NULL
}

int main(int argc, char* argv[]){
        int A[50];
        int thread[50];// inicializamos thread y mutex
        pthread_mutex_init(&lock, NULL);
        for (int i=0; i<50;i++){
            int *index = malloc(sizeof(int));  // SIEMPRE QUE QUERAMOS PASRELE A OS THREADS ALGO UNICO reservar memoria para un int
             *index = i;                         // guardamos el valor del índice
            pthread_create(&thread[i],NULL, increment, index); //creamos los threads pasandoles el indice
    }
    for (int i=0; i<50;i++){
            pthread_join(thread[i], NULL);
    }

}