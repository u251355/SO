//EX1
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
int received = 0;
void handler(int sig) //se hara cuando le llegue una señal
{
    received = 1;   
}
int main(int argc, char *argv[]){ //el que waitea
    signal(SIGUSR1, handler);//cuando llegue la señal SIGUSR1 llama al handler
    char msg[] = "Esperando señal SIGUSR1...\n";
    write(1, msg, sizeof(msg) - 1);
    while (received == 0) { //mientras handler no haya recivido nada duerme
        sleep(1);
    }
    char end[] = "Señal recibida. Terminando.\n";
    write(1, end, sizeof(end) - 1); 
    _exit(0);
}
int main(int argc, char *argv[])// el que sendea
{
    int pid = atoi(argv[1]); //pasa anumro el argumento
    kill(pid, SIGUSR1); //envia la señal
    _exit(0);
}

//Ex2
void handler(int sig)//dice que vaya sumando de mas en mas
{
    step++;   
}
int main()
{
    signal(SIGINT, handler); //señal se llama tal y se mandara al handler
    int i = 0;
    char c;
    while (i <= 500)
    {
        if (i >= 100) {
            c = (i / 100) + '0';
            write(1, &c, 1);
        }
        if (i >= 10) {
            c = ((i / 10) % 10) + '0';
            write(1, &c, 1);
        }
        c = (i % 10) + '0';
        write(1, &c, 1);
        write(1, "\n", 1);
        usleep(100000);  
        i += step;
    }
    _exit(0);
}
//con printf
void handler(int sig)
{
    step++;   
}
int main()
{
    signal(SIGINT, handler);
    int counter = 0;
    while (counter <= 500)
    {
        printf("%d\n", counter);
        usleep(100000);   // 100 ms
        counter += step;
    }
    _exit(0);
}
//Ex3
int main() {
    int p1[2], p2[2]; //creamos las pipes lo primero
    pipe(p1);
    pipe(p2);
    if (fork() == 0) { // el primer hijo
        dup2(p1[1], 1);// redirige el mensaje
        close(p1[0]);//cierra las pipes innecesarias
        close(p1[1]);
        char *args1[] = {"ps", "-ef", NULL};//sustituye a ps
        execvp("ps", args1);
        _exit(1);
    }
    if (fork() == 0) {//segundo hijo
        dup2(p1[0], 0);//redirige lo suyo y el mensaje del otro
        dup2(p2[1], 1);
        close(p1[1]); 
        close(p2[0]);
        char *args2[] = {"grep", "-i", "system", NULL};
        execvp("grep", args2); //sustituye a grep
        _exit(1);
    }
    if (fork() == 0) {//trcer hioh
        int fd = open("out.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644); //es loq eu hace wc
        dup2(p2[0], 0);//redirige
        dup2(fd, 1);
        close(p2[1]);
        char *args3[] = {"wc", "-c", NULL};//sustituye wc
        execvp("wc", args3);
        _exit(1);
    }
    close(p1[0]); 
    close(p1[1]);
    close(p2[0]); 
    close(p2[1]);
    return 0;
}

//Ex4
int main(){
int fd1[2];
int fd2[2];
int read =open(text.txt, O_RDONLY, 0644);
for (int i=0; i<4; i++){//crea 4 hijos
    pid_t pid = fork();
    if (fork ==0){//los hijos
    close(fd1[1]);
    close(fd2[0]);
    int sum = 0;
    while (read(p1[0], &c, 1) > 0){//leen lo que les envia el padre
                sum += c;
            write(p2[1], &sum, sizeof(int));//escriben la suma
            _exit(0);//porque es un fork dentro de un bucle
        }
    }
    close(p1[0]);  //el padre
    close(p2[1]);
    int fd = open(argv[1], O_RDONLY);//lee del archivo
    unsigned char c;
    while (read(fd, &c, 1) > 0) //mientras lea algo
    write(p1[1], &c, 1);//escribe a los hijos
    close(p1[1]);  
    close(fd);
    int total = 0;
    int partial;
    for (int i = 0; i < 4; i++) { //lee las respeustas de los 4 hijos
        read(p2[0], &partial, sizeof(int));
        total += partial;
    }
    char buffer[20];
    int i = 0;
     n = total;
    if (n == 0) buffer[i++] = '0';
    else {
        char temp[20];
        int j = 0;
        while (n) { temp[j++] = (n % 10) + '0'; n /= 10; }
        while (j--) buffer[i++] = temp[j];
    }
    buffer[i++] = '\n';
    write(1, buffer, i);
    close(p2[0]);
    for (int i = 0; i < 4; i++)
        wait(NULL);
    return 0;
}
}


    










}