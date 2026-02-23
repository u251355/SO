//Ex1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
int main(int argc, char* argv[]) {
int fd[2]; //crea la pipe
pid_t pid = fork(); //crea al hijo
pid_t parent_pid_from_pipe; //variable donde el hijo guardara el pid
if(pid>0){//padre
    close(fd[0]);  // cierra la lectura
    pid_t mypid = getpid(); //coje su pid
    write(fd[1], &mypid, sizeof(mypid)); //lo escribe
    close(fd[1]);//cierra la escritura
    } 
else{
    close(fd[1]);//cierra escritura
     read(fd[0],&parent_pid_from_pipe, sizeof(parent_pid_from_pipe)); //lee
}
if (parent_pid_from_pipe == getppid())
            printf("Correct\n");

}
//Ex2



