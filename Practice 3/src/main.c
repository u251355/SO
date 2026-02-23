#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parsePGM.h"

#define BUFF_SIZE 1024

typedef struct {
    char* path;
    int offset;
    int bytesToRead;
    int maxval;
    unsigned int* localHist;
} ThreadInfo;

void* worker(void* arg) {
    ThreadInfo* info = (ThreadInfo*)arg;

    int fd = open(info->path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        pthread_exit(NULL);
    }

    if (lseek(fd, info->offset, SEEK_SET) < 0) {
        perror("lseek");
        close(fd);
        pthread_exit(NULL);
    }

    unsigned char buffer[BUFF_SIZE];
    int remaining = info->bytesToRead;

    while (remaining > 0) {
        int toRead = remaining > BUFF_SIZE ? BUFF_SIZE : remaining;
        int totalRead = 0;

        while (totalRead < toRead) {
            int r = read(fd, buffer + totalRead, toRead - totalRead);
            if (r <= 0) {
                perror("read");
                close(fd);
                pthread_exit(NULL);
            }
            totalRead += r;
        }

        for (int i = 0; i < totalRead; i++) {
            info->localHist[buffer[i]]++;
        }

        remaining -= totalRead;
    }

    close(fd);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        printf("Usage: %s image.pgm histogram.txt numThreads\n", argv[0]);
        return 1;
    }

    int numThreads = atoi(argv[3]);
    if (numThreads <= 0) {
        printf("Number of threads must be > 0\n");
        return 1;
    }

    int width, height, maxval;
    int nBytesHeader = parse_pgm_header(argv[1], &width, &height, &maxval);

    if (nBytesHeader < 0) {
        printf("Error parsing PGM header\n");
        return 1;
    }

    if (maxval > 255) {
        printf("Expecting 1 byte per pixel\n");
        return 1;
    }

    int nPixels = width * height;
    int dataSize = nPixels;

    /* Global histogram (0..maxval) */
    unsigned int* histogram = calloc(maxval + 1, sizeof(unsigned int));
    if (!histogram) {
        perror("calloc histogram");
        return 1;
    }

    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
    ThreadInfo* infos = malloc(numThreads * sizeof(ThreadInfo));

    if (!threads || !infos) {
        perror("malloc");
        free(histogram);
        return 1;
    }

    int chunk = dataSize / numThreads;

    for (int i = 0; i < numThreads; i++) {

        infos[i].path = argv[1];
        infos[i].offset = nBytesHeader + i * chunk;
        infos[i].maxval = maxval;
        infos[i].bytesToRead = (i == numThreads - 1) ?
                               (dataSize - i * chunk) : chunk;

        infos[i].localHist = calloc(maxval + 1, sizeof(unsigned int));
        if (!infos[i].localHist) {
            perror("calloc localHist");
            for (int j = 0; j < i; j++)
                free(infos[j].localHist);
            free(histogram);
            free(threads);
            free(infos);
            return 1;
        }

        if (pthread_create(&threads[i], NULL, worker, &infos[i]) != 0) {
            perror("pthread_create");
            for (int j = 0; j <= i; j++)
                free(infos[j].localHist);
            free(histogram);
            free(threads);
            free(infos);
            return 1;
        }
    }

    /* Join threads */
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Merge histograms */
    for (int t = 0; t < numThreads; t++) {
        for (int i = 0; i <= maxval; i++) {
            histogram[i] += infos[t].localHist[i];
        }
        free(infos[t].localHist);
    }

    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
        perror("open output");
        free(histogram);
        free(threads);
        free(infos);
        return 1;
    }

    for (int i = 0; i <= maxval; i++) {
        char line[64];
        int len = sprintf(line, "%d,%d\n", i, histogram[i]);
        write(fd_out, line, len);
    }

    close(fd_out);

    free(histogram);
    free(threads);
    free(infos);

    return 0;
}