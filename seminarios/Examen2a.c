#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
int sum=0;//global pq se necesita tmb en el codigo del thread
pthread_mutex_t lock;//esto siempre fuera
void* sumation(void *arg){ //codigo del thread
    char *file = (char*) arg; //SIEMPRE
   int fd= open(file,O_RDONLY,0644);
   int bytes;
   int c;
   while(bytes=read(fd,&c, sizeof(int))>0){//siempre pq sino no lo lees todo
    pthread_mutex_lock(&lock);
   sum=sum+c;
   pthread_mutex_unlock(&lock);
   }
   close(fd);
   return NULL;
}

int main(int argc, char* argv[]){
pthread_t threads[32];
for(int i=0; i<argc; i++){
    pthread_create(&threads[i],NULL,sumation, argv[i]);//creas los threads
}
for(int i=0; i<argc; i++){
    pthread_join(&threads[i],NULL);//esperas a que todos acaben
}

}