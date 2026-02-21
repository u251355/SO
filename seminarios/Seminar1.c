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

//EX3
int main3{
char buffer[100]; //inicializar
int sum=0;
int n;
while (int bytes = read(0,buffer,100)>0) { //mientras lea algo
buffer[bytes] = '\0'; //cierra el texto
sscanf(buffer,"%d",&n)//lee el bufer
sum += n; //suma
    }
int fd4 = open(argv[1], O_WRONLY|O_CREATE,0644);
int write(argv[1],&sum,sizeof(sum));//porque es binario
int close(fd);
}

//Ex4
int main(){
    int fd5 = open(text.txt, O_RDONLY!O_CREATE,0644);
    lseek(fd5,0,SEEK_END);//desde el final
    char c;
     while(lseek(fd, -1, SEEK_CUR) >= 0) { //mientras lea
        read(fd5, &c, 1);//lee
        write(1, &c, 1);//lo escribes en el std output
     }
    int close(fd5);
    }
