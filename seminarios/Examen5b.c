#include<stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<pthread.h>
#include<semaphore.h>

int main(int argc, char*argv[]){
int fd[2];
pipe(fd);

int pid = fork();
if (pid ==0){





}
else{
    
}
wait(NULL);






}
