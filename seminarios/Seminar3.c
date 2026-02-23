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
int main(int argc, char* argv[]) {
int fd1[2];//dos pipes
int fd2[2];
pid_t pid = fork();//crea al hijo
if (pid>0){
    close(fd1[0]);
    close(fd2[0]);
    for (int i = 1; i < argc; i++) {//manda por la pipe cada numero
            int num = atoi(argv[i]);
            write(fd1[1], &num, sizeof(num));
        }
    }
    close(fd[1]);
    else {  // child
        close(fd1[1]);
        close(fd2[0]);
        int num;
        while (read(fd1[0], &num, sizeof(num)) > 0) {//lee los numeros de la pipe
            if (num % 2 == 0) {
                write(fd2[1], &num, sizeof(num));
            }
        }char buffer[20];
          int len = sprintf(buffer, "%d\n", num);
         write(1, buffer, len);

        close(pipe1[0]);
        close(pipe2[1]);
    }

    return 0;
}








