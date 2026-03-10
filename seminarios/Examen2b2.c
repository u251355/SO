#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>
int eog=0;
int main(int argc, char*argv[]){
int fd[2];
int fd2[2];
pipe(fd);
pipe(fd2);
int number;
pid_t pid = fork();
while(eog!=1){
if (pid==0){ //hijo
    close(fd[1]);
    close(fd2[0]);
    read(fd[0], &number,sizeof(int));
    if(number == getpid() % 100){//compara con su pid
    close(fd[0]);//cierra para decir que ya ha acabado
    eog=1;
    }
    else{
    close(fd2[0]);
    int z=0;
    write(fd2[1],&z,sizeof(int));//escribe 0 si ha perdido
    }

    }
else{ //el padre
    number= rand() % 100;
    close(fd2[1]);
    close(fd[0]);
    if (number==getpid() % 100){ //compara con su pid y si es igual cierra las pipes
    close(fd[1]);
    close(fd[0]);
    eog=1;
    }
    else{
        int z;
    write(fd[1],&number, sizeof(int));//si ha perdido se lo pasa al hijo
    read(fd2[0], &z, sizeof(int));
    }
}
}
wait(NULL);
}