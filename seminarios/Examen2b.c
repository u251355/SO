#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>
int sum=0;//global pq se necesita tmb en el codigo del thread
pthread_mutex_t lock;//esto siempre fuera
Semaphore sem;//siempre fuera tambien, faltaria copiar y pegar el codigo del semaforo para que funcione
void* sumation(void *arg){ //codigo del thread
    char *file = (char*) arg; //SIEMPRE
   int fd= open(file,O_RDONLY,0644);
   int bytes;
   int c;
   sem_wait(&sem);//el semaforo hace esperar
   while(bytes=read(fd,&c, sizeof(int))>0){//siempre pq sino no lo lees todo
    pthread_mutex_lock(&lock);
   sum=sum+c;
   pthread_mutex_unlock(&lock);
    sem_signal(&sem);//indica que ya esta
   }
   close(fd);
   return NULL;
}

int main(int argc, char* argv[]){
int threads[32];
pthread_mutex_init(&lock,NULL);//inicializar todo en el main siempre
sem_init(&sem,2);
for(int i=0; i<argc; i++){
    pthread_create(&threads[i],NULL,sumation, argv[i]);//creas los threads
}
for(int i=0; i<argc; i++){
    pthread_join(&threads[i],NULL);//esperas a que todos acaben
}

}