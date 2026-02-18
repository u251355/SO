//Ex1 
#include <fcntl.h>
#include <unistd.h>
int main(){
int count =0;
char c; 
while (read(0, &c, 1) > 0) {   //mientras lea, std input fd=0
        if (c == 'a') //si la letra coincide
        count++; //suma
    }
    char buffer[20];
    int len = sprintf(buffer, "%d\n", count); // lo guarda en el buffer, en formato d el counter(para escribir en strings como buffer)
    write(1, buffer, len); //1 is the standard output fd=1
    return 0;
}
 
//EX2
int main2()´{
int fd1 = open (text.txt,O_WRONLY|O_CREATE, 0644);//siempre es asi
int fd2 = open (binary.dat,O_WRONLY|O_CREATE, 0644);
char buffer [70];
   for(int i=0;i<100;i++) { //100 cosas
        int n = rand(); //crear numeros random
        int len = sprintf(buffer,"%d,",n); //guardamos en el buffer
        write(fd1, buffer, len);//donde, desde donde, que
        write(fd_bin, &n, sizeof(n));
    }
    close(fd1);
    close(fd2);
}



