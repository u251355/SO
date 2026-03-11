#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
unsigned char is_printable(char c) {  // TE LO DAN
 if (c >= 32 && c <= 126) {
 return c;
 } else {
 return 0;
 }
}
char buffer[100];
int main(int argc, char*argv[]){
  int  fd1[2]; //siempre las pipes antes que los hijos
  int fd2[2];
  pipe(fd1);
  pipe(fd2);
  int pid = fork(); //hijo 1

  if(pid!=0){//padre
    close(fd1[0]);
    close(fd2[0]);
    close(fd2[1]);
  int ar= open(argv[1], O_RDONLY,0644);//abrimos el archivo
  int bytes;
  while( bytes= read(ar,buffer, 1 )>0){//mientras haya algo que leer
     for (bytes i = 0; i < bytes; i++) {
    write(fd1[1], buffer, 1)//se lo escribe al hijo
  }
  wait(NULL);
  }
close(ar);
close(fd1[1]);
  }
if (pid ==0){ //el hijo crea otro hijo
    int pid2= fork();//hijo 2
    close(fd1[1]);
    char c;
    read(fd1[0], &c, 1);
    unsigned char filtrada = is_printable(c);
    close(fd2[0]);
    write(fd2[1], &filtrada,1);

if (pid2==0){
    close(fd2[1]);
   while( read(fd2[0],&buffer, 1)>0){
    write(1,&buffer, 1)
}
}
wait(NULL);
}
return 0

}
