#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>
int main(int argc, char* argv[]){
int fd[2];
pipe(fd);//creamos la pipe
int pid = fork();//creamos al hijo
if (pid==0){//el hijo
    close(fd[1]);
    int numby;
    int sum=0;
    while((numby=read(fd[0], &num, sizeof(int))>0)){
       for(int i=0; i<numby;i++){
        sum=sum+num;
       }
    }
    close(fd[0]);
    write(1,&sum,sizeof(int));
}
else{//el padre
 close(fd[0]);
 dup2(fd[1],1);//redirige del 1 al fd[1], osea del reves
 close(fd[1]);
 char buffer[10] = {1,2,3,4,5,6,7,8,9,10};
 write(1, buffer, 10);//cuando escriba al 1 en verdad sera fd[1]
}
wait(NULL);
}