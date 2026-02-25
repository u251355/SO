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







}
