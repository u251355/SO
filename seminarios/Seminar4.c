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
volatile sig_atomic_t step = 1;
void handler(int sig)//dice que vaya suemando de mas en mas
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