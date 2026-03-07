//second version of 5 wins needed to end the game
#include <pthread.h>
#include <unistd.h>
#include<stdio.h>
 pthread_mutex_t lock;//inicializamos un lock
 int flag =1;
void*player(void*arg){
    int wins=0;
    int eog =0;
    int id= *(int*)arg;//el argumento es el id
    while(!eog){
    pthread_mutex_lock(&lock);//bloquea
    if(flag == 1){
        flag = 0;
        wins++;
    }
    if (wins>=5){
       eog=1;
    }
    flag=1;
    pthread_mutex_unlock(&lock);//desbloquea
    usleep(1000);
    }
    return NULL;
}
int main(int argc, char* argv){
    int id[10];
    pthread_t threads[10]; //inicializamos 10 threads
    for (int i=0; i<10;i++){//creamos los threads
        pthread_create(&threads[i], NULL,player,&id[i]);
    }
 for (int i=0; i<10;i++){//creamos los threads
        pthread_join(threads[i], NULL); //esperamos a que todos acaben
    }

}