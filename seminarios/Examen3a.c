#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>
int counter = 0;
int end = 0;
pthread_mutex_t lock;
void* increment(void* a){//te lo dan
    while(end == 0){
        pthread_mutex_lock(&lock);
        if(counter == 0){
            counter += rand()%1000;
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* decrement(void* arg){
    int *value = (int*)arg; //casteamos el valor
    while(end==0){ //no se haya acabado el programa
        pthread_mutex_lock(&lock);//bloqueamos
        if(counter>=value){//si se puede restar
        counter = counter - value;//restamos y debloqueamos
        }
        pthread_mutex_unlock(&lock);
    }
    return(NULL);
    }

    int main(int argc, char* argv[]){
        pthread_t thread[4]; 
        pthread_mutex_init(&lock,NULL);//inicializamos el lock
        int c[3] ={1,5,10};
        for (int i=0; i<3;i++){
            pthread_create(&thread[i],NULL,decrement, &c[i]);//creamos los decrement
        }
       pthread_create(&thread[3], NULL, increment, NULL);//creamos en increment
    for (int i=0; i<4;i++){
            pthread_join(thread[i],NULL);//esperamos a todos
    }
return 0;

}