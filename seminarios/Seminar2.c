//Ex1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(){
    int fd= open(t.txt, O_WRONLY, 0644);
    pid_t pid = fork(); //crea un proceso hijo
    int mypid = getpid();
int delay = (mypid / 10) % 10000; //el retraso de cada uno
usleep(delay);
    if (pid == 0) { //es el hijo siempre
        write(fd, "World\n", 6);
    } else { //el padre
        write(fd, "Hello\n", 6);
        wait(NULL);
    }
    close(fd);
}
//Ex2
int main2 (){
for (int i=0;i<100;i++){ //crear 100 hijos
    pid_t pid =fork();
}
if (pid == 0) {     //los hijos duermen
        sleep(1);
        exit(0);       
    }
 for (int i=0;i<100;i++){ //espera a los 100 hijos
  wait (NULL); 
}
}
//modificacion execvp
char *args[] = {"sleep", "1", NULL};  //lista de argumentos para el execvp
execvp("sleep", args); //reemplaza por sleep
exit(1);
//modificacion usleep
if (pid == 0) {
            usleep(1000);  // duerme cada hijo 
            exit(0); //siempre que haya un bucle con forks
        } else {
            wait(NULL);    // espera el padre
        }



