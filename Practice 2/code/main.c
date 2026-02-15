#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "circularBuffer.h"
#include "splitCommand.h"
#define BUFFER_SIZE 1024  // we define the maximum buffer size

int read_line(int fd, CircularBuffer *cb, char *line, int max_len, int *reachedEOF) // function to read one line using circular buffer
{
    int bytes_read; // initialize number of bytes read from input
    int elemSize; // initialize the size of next complete line in buffer

    while (1) {   // loop until a full line is found
        elemSize = buffer_size_next_element(cb, '\n', *reachedEOF);  // check if a full line ending with '\n' exists in buffer
        if (elemSize > 0) { // if a complete line is available
            if (elemSize > max_len - 1) // avoid overflow if line is too long
                elemSize = max_len - 1;
            for (int i = 0; i < elemSize; i++)  // copy characters from circular buffer into line
                line[i] = buffer_pop(cb);
            line[elemSize] = '\0';  // add string terminator
            return elemSize; // return size of line read
        }
        if (*reachedEOF) // if EOF reached and no full line stop reading
            return 0;
        char temp[BUFFER_SIZE];
        bytes_read = read(fd, temp, BUFFER_SIZE); // read more data from the file
        if (bytes_read < 0) { // check if there is a read error
            perror("read");
            exit(1);
        }
        if (bytes_read == 0) // if no bytes read, EOF true
            *reachedEOF = 1;
        for (int i = 0; i < bytes_read; i++) // push all read bytes into circular buffer
            buffer_push(cb, temp[i]);
    }
}

int main() {
    signal(SIGCHLD, SIG_IGN); //handle child process termination automatically
    CircularBuffer cb; // declare circular buffer
    buffer_init(&cb, BUFFER_SIZE); // initialize buffer
    char line[BUFFER_SIZE];  // store input lines
    int reachedEOF = 0;
    while (1) { // main shell loop
        if (read_line(STDIN_FILENO, &cb, line, BUFFER_SIZE, &reachedEOF) <= 0) // read execution mode (SINGLE, PIPE, CONCURRENT, EXIT)
            break;
        line[strcspn(line, "\n")] = '\0'; // remove newline character from string
        if (strcmp(line, "EXIT") == 0) { // if the command is EXIT, terminates program
            break;
        }

        if (strcmp(line, "SINGLE") == 0) { // SINGLE execution mode
            read_line(STDIN_FILENO, &cb, line, BUFFER_SIZE, &reachedEOF); // read next line containing command
            pid_t pid = fork();  // create child process
            if (pid < 0) { // check fork error
                perror("fork");
                exit(1);
            }
            if (pid == 0) { // child process
                char **argv = split_command(line);  //split command into argv array
                execvp(argv[0], argv); // replace child process with command
                perror("execvp"); // if exec fails, print error
                exit(1);
            } else {
                waitpid(pid, NULL, 0); // parent waits until child finishes
            }
        }
        else if (strcmp(line, "CONCURRENT") == 0) { // CONCURRENT execution
            read_line(STDIN_FILENO, &cb, line, BUFFER_SIZE, &reachedEOF); // read next line containing the command to execute
            pid_t pid = fork(); // create a child process
            if (pid < 0) { // check if fork failed
                perror("fork");
                exit(1);
            }
            if (pid == 0) { // child process executes the command
                char **argv = split_command(line); // split command into argv format
                execvp(argv[0], argv); // replace child process with the new program
                perror("execvp"); // if execvp fails, print error and exit
                exit(1);
            }
               // parent does not wait 
                 }
        else if (strcmp(line, "PIPED") == 0 || strcmp(line, "PIPE") == 0) { // we check if the user has introduced the PIPE or PIPED execution mode
            char line1[BUFFER_SIZE]; // it stores the first command
            char line2[BUFFER_SIZE]; // it stores the second command
            // read two lines from the input using the circular buffer
            read_line(STDIN_FILENO, &cb, line1, BUFFER_SIZE, &reachedEOF);
            read_line(STDIN_FILENO, &cb, line2, BUFFER_SIZE, &reachedEOF);
            int pipefd[2]; // it creates a new pipe 
            if (pipe(pipefd) < 0) { // error control
                perror("pipe");
                exit(1);
            }
            pid_t pid1 = fork(); // it creates a child
            if (pid1 < 0) { //  fork error control
                perror("fork");
                exit(1);
            }
            if (pid1 == 0) { // code for the first child
                dup2(pipefd[1], STDOUT_FILENO); // it redirects the output to the pipe
                // it closes both ends of the pipe
                close(pipefd[0]);
                close(pipefd[1]);
                char **argv1 = split_command(line1); // convert the command to argv format 
                execvp(argv1[0], argv1); // replace the child process with the program of the first command
                perror("execvp"); // if it fails (error control)
                exit(1);
            }
            pid_t pid2 = fork(); // it creates the second child 
            if (pid2 < 0) { //  fork error control
                perror("fork");
                exit(1);
            }
            if (pid2 == 0) { // code for the second child
                dup2(pipefd[0], STDIN_FILENO); // redirects the input to the read end of the pipe
                // closes the descriptors that are not necessary
                close(pipefd[1]);
                close(pipefd[0]);
                char **argv2 = split_command(line2); 
                execvp(argv2[0], argv2);
                perror("execvp"); // error control
                exit(1);
            }
            // closes both ends
            close(pipefd[0]);
            close(pipefd[1]);
            // waits until both children are finished
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
    buffer_deallocate(&cb); // frees the memory of the circcular buffer
    return 0;
}
