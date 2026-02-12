#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "circularBuffer.h"
#include "splitCommand.h"
#define BUFFER_SIZE 1024

// Function to read one line using circular buffer
int read_line(int fd, CircularBuffer *cb, char *line, int max_len, int *reachedEOF) {
    int bytes_read;
    int elemSize;

    while (1) {
        elemSize = buffer_size_next_element(cb, '\n', *reachedEOF);

        if (elemSize > 0) {
            if (elemSize > max_len - 1)
                elemSize = max_len - 1;

            for (int i = 0; i < elemSize; i++)
                line[i] = buffer_pop(cb);

            line[elemSize] = '\0';
            return elemSize;
        }

        if (*reachedEOF)
            return 0;

        char temp[BUFFER_SIZE];
        bytes_read = read(fd, temp, BUFFER_SIZE);

        if (bytes_read < 0) {
            perror("read");
            exit(1);
        }

        if (bytes_read == 0)
            *reachedEOF = 1;

        for (int i = 0; i < bytes_read; i++)
            buffer_push(cb, temp[i]);
    }
}

int main() {

    // Evitar procesos zombie en CONCURRENT
    signal(SIGCHLD, SIG_IGN);

    CircularBuffer cb;
    buffer_init(&cb, BUFFER_SIZE);

    char line[BUFFER_SIZE];
    int reachedEOF = 0;

    while (1) {

        // Read execution mode
        if (read_line(STDIN_FILENO, &cb, line, BUFFER_SIZE, &reachedEOF) <= 0)
            break;

        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "EXIT") == 0) {
            break;
        }

        // SINGLE execution
        if (strcmp(line, "SINGLE") == 0) {

            read_line(STDIN_FILENO, &cb, line, BUFFER_SIZE, &reachedEOF);

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                char **argv = split_command(line);
                execvp(argv[0], argv);
                perror("execvp");
                exit(1);
            } else {
                waitpid(pid, NULL, 0);
            }
        }

        // CONCURRENT execution
        else if (strcmp(line, "CONCURRENT") == 0) {

            read_line(STDIN_FILENO, &cb, line, BUFFER_SIZE, &reachedEOF);

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                char **argv = split_command(line);
                execvp(argv[0], argv);
                perror("execvp");
                exit(1);
            }

            // NO wait here (background)
        }

        // PIPED execution
        else if (strcmp(line, "PIPED") == 0 || strcmp(line, "PIPE") == 0) {

            char line1[BUFFER_SIZE];
            char line2[BUFFER_SIZE];

            read_line(STDIN_FILENO, &cb, line1, BUFFER_SIZE, &reachedEOF);
            read_line(STDIN_FILENO, &cb, line2, BUFFER_SIZE, &reachedEOF);

            int pipefd[2];
            if (pipe(pipefd) < 0) {
                perror("pipe");
                exit(1);
            }

            pid_t pid1 = fork();
            if (pid1 < 0) {
                perror("fork");
                exit(1);
            }

            if (pid1 == 0) {
                // First process writes to pipe
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);

                char **argv1 = split_command(line1);
                execvp(argv1[0], argv1);
                perror("execvp");
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 < 0) {
                perror("fork");
                exit(1);
            }

            if (pid2 == 0) {
                // Second process reads from pipe
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[1]);
                close(pipefd[0]);

                char **argv2 = split_command(line2);
                execvp(argv2[0], argv2);
                perror("execvp");
                exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }

    buffer_deallocate(&cb);
    return 0;
}
