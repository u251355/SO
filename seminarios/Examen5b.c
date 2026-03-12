#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>

int main(int argc, char*argv[]){
    int fd[2]; //creamos pipe
    pipe(fd);

    int pid = fork(); //creamos hijo
    if (pid ==0){ //el hijo
        close(fd[0]); //cerramos lectura
        char *args[]= {"ls", NULL}; //creamos exe implementando un array char antes
        dup2(fd[1], 1); //redirige a la pipe la salida
        close(fd[1]);
        execvp("ls",args);//todo antes del exe pq nada despues pasara
    }
    else{//el padre
        close(fd[1]);
        int file= open("out.txt", O_WRONLY, 0644);
        char buffer[100];
        int bytes;
        while((bytes= read(fd[0],buffer,100 ))>0){
            write(file, buffer, bytes);
        }
        close(file);
        close(fd[0]);
    }
    wait(NULL);
}
