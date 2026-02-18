//Ex1 
#include <fcntl.h>
#include <unistd.h>
int main(){
int count =0;
char c; 
while (read(0, &c, 1) > 0) {   //mientras lea, std input
        if (c == 'a') //si la letra coincide
        count++; //suma
    }
    char buffer[20];
    int len = sprintf(buffer, "%d\n", count); //convierte a numero y lo guarda en el buffer
    write(1, buffer, len); //1 is the standard output
    return 0;
}


