#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "circularBuffer.h"

/* ==========================================================
   BINARY MODE: read integers directly
   ========================================================== */
long sumBinary(int fd, int bufferSize) {

    // Adjust buffer size to multiple of int
    bufferSize -= bufferSize % sizeof(int);

    int *buffer = malloc(bufferSize);
    if (!buffer) {
        perror("malloc");
        exit(1);
    }

    long sum = 0;
    int bytesRead;

    while ((bytesRead = read(fd, buffer, bufferSize)) > 0) {

        int count = bytesRead / sizeof(int);

        for (int i = 0; i < count; i++) {
            sum += buffer[i];
        }
    }

    free(buffer);
    return sum;
}

/* ==========================================================
   TEXT MODE: CSV with circular buffer
   ========================================================== */
long sumText(int fd, int bufferSize) {

    CircularBuffer cb;
    buffer_init(&cb, bufferSize);

    char *temp = malloc(bufferSize);
    if (!temp) {
        perror("malloc");
        exit(1);
    }

    long sum = 0;
    int reachedEOF = 0;

    while (!reachedEOF || buffer_used_bytes(&cb) > 0) {

        /* ---- Read more data if not EOF ---- */
        if (!reachedEOF && buffer_free_bytes(&cb) > 0) {

            int bytesRead = read(fd, temp, bufferSize);

            if (bytesRead == 0) {
                reachedEOF = 1;
            } else  {
                for (int i = 0; i < bytesRead; i++) {
                    if (buffer_free_bytes(&cb) > 0)
                        buffer_push(&cb, temp[i]);
                }
            }
        }

        /* ---- Extract complete numbers ---- */
        int elemSize = buffer_size_next_element(&cb, ',', reachedEOF);

        while (elemSize != -1) {

            char numberStr[elemSize + 1];

            for (int i = 0; i < elemSize; i++) {
                numberStr[i] = buffer_pop(&cb);
            }

            numberStr[elemSize] = '\0';

            // Remove comma if present
            if (numberStr[elemSize - 1] == ',')
                numberStr[elemSize - 1] = '\0';

            int num = atoi(numberStr);
            sum += num;

            elemSize = buffer_size_next_element(&cb, ',', reachedEOF);
        }
    }

    free(temp);
    buffer_deallocate(&cb);

    return sum;
}

/* ==========================================================
   MAIN PROGRAM
   ========================================================== */
int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: %s binary|text pathToFile bufferSize\n", argv[0]);
        return 1;
    }

    char *mode = argv[1];
    char *filePath = argv[2];
    int bufferSize = atoi(argv[3]);

    int fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    long result;

    if (strcmp(mode, "binary") == 0) {
        result = sumBinary(fd, bufferSize);

    } else if (strcmp(mode, "text") == 0) {
        result = sumText(fd, bufferSize);

    } else {
        printf("Error: mode must be 'binary' or 'text'\n");
        close(fd);
        return 1;
    }

    close(fd);

    printf("SUM = %ld\n", result);

    return 0;
}
