//10 threads flag game
#include <pthread.h>
#include <unistd.h>
#include<stdio.h>
 pthread_mutex_t lock;//inicializamos un lock
 int flag =1;
void*player(void*arg){
    int wins=0;
    int id= *(int*)arg;//el argumento es el id
    pthread_mutex_lock(&lock);//bloquea
    if(flag == 1){
        flag = 0;
        wins++;
        printf("Player %d wins the round\n", id);
    }
    pthread_mutex_unlock(&lock);//desblouqa
    free(arg);//porque le has pasado un array
    return NULL;
}
int main(int argc, char* argv){
    int id[10];
    pthread_t threads[10]; //inicializamos 10 threads
    for (int i=0; i<10;i++){//creamos los threads
        int *index= malloc(sizeof(int));//para pasarle a un thread un array
        *index = i;
        pthread_create(&threads[i], NULL,player,index); //le pasas index
    }
 for (int i=0; i<10;i++){//creamos los threads
        pthread_join(threads[i], NULL); //esperamos a que todos acaben
    }

}
