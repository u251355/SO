#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>
int eog=0;
int main(int argc, char*argv[]){
int fd[2];
pipe(fd);
pid_t pid = fork();
while(eog!=1){
if (pid==0){ //hijo
    close(fd[1]);
    read(fd[0], &number,1);
    if(number == getpid() mod 100){//compara con su pid
    close(fd[0]);//cierra para decir que ya ha acabado
    close(fd[1]);
    eog=1;
    }
    else{
    write(fd[1],0,1);//escribe 0 si ha perdido
    }

    }
else{ //el padre
    int number = rand() % 100;
    if (number==getppid() mod 100){ //compara con su pid y si es igual cierra las pipes
    close(fd[1]);
    close(fd[0]);
    eog=1;
    }
    else: 
    write(fd[1],&number, 1);//si ha perdido se lo pasa al hijo
    }
wait(NULL);
}

}








}