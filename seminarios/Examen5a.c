#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>

pthread_mutex_t lock;
pthread_cond_t cond;
A[10];
int accessing[10] = {0};//falso

typedef struct{ //creas el struct de la particula
    int i;
    int j;
} particle;


void *move(void arg){ //funcion del struct
    particle *p = (particle*)arg; //para castear structs es asi
    int i = p->i;//te guardas tus cositas, porque las necesitas luego
    int j= p->j;
    pthread_mutex_lock(&lock);
    while(accessing[i] || accessing[j]) { //condition(chat), mientras alguien este en alguna de las dos
        pthread_cond_wait(&cond, &lock); //waitea en el while despues del lock
    }
    accessing[i] = 1; //si que hay alguien
    accessing[j] = 1;

    if(A[i]>0){ //si es mayor de 0 se pasa de uno a otro
        A[i]--;
        A[j]++;
    }
    
    accessing[i] = 0;//ya no hay nadie
    accessing[j] = 0;
    pthread_mutex_unlock(&lock); //desbloqueas y broadcast
    pthread_cond_broadcast(&cond);
return NULL;
}

int main(int argc, char* argv[]){
    int threads[5];
    particle p[5];
    pthread_mutex_init(&lock);
    pthread_mutex_cond_init(&cond);

    for (int t = 0; t < 5; t++) {
        p[t].i = rand()%10;
        p[t].j = rand()%10;
        pthread_create(&threads[t], NULL, move, &p[t]);
    }

    for (int t = 0; t < 5; t++) {
        pthread_join(threads[t], NULL);
    }










}
