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
int main(int argc, char*argv){
  int  fd1[2]; //siempre las pipes antes que los hijos
  int fd2[2];
  pid_t pid = fork(); //hijo 1
  while(int read(buffer,argv[], 1 )>0){//mientras haya algo que leer
    close(fd1[0]);
    write(fs1[1], buffer, 1)//s elo escribe al hijo
  }
if (pid ==o){ //el hijo crea otro hijo
    pid_t pid2= fork();
    
}








}