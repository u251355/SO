#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>
int bytes;
char buffer[100];
char c;
int i=0;
int readf(int fd, char*buffer){//FUNCION QUE LEE DEL FILE DESCRIPTOR
    while( (bytes= read(fd,&c,1))>0){ //mientras lea algo del fichero lo guarda en c
        buffer[i]=c; //c lo guarda en un array
        i++;
         if(c == '\0'){ //si lo que lee es el final del numero
            int num = atoi(buffer);//lo transfora a int
            return num;
}
}
return 0;
}
int main(int argc, char* argv[]){
    char c[100];
    int fd1[2];
    pipe(fd1);
    pid_t pid = fork(); //crea al hijo
    if (pid ==0){ //el hijo
        close(fd1[1]);
        int outputf = open(argv[2], O_WRONLY, 0644);
        while(read(fd1[0], c, sizeof(int))>0){
        write(outputf, c, sizeof(int));
        }
        close(fd1[0]);
        close(outputf);
    }
    else{ //el padre
        close(fd1[0]);
        int inputf= open(argv[1], O_RDONLY, 0644);
        int r;
        while (r=readf(inputf, buffer)>0);{
        write(fd1[1], &num, sizeof(int)); 
        }
         close(inputf);
        }
  close(fd1[1]);
  wait(NULL);
}