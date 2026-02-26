#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parsePGM.h"
#define BUFF_SIZE 1024

typedef struct { // structure used to pass information to each thread 
    char* path;
    int offset;
    int bytesToRead;
    int maxval; // maximum grayscale value
} ThreadInfo;
unsigned int* histogram; // global shared histogram 
pthread_mutex_t hist_mutex; // global mutex 

void* worker(void* arg) { //thread worker
    ThreadInfo* info = (ThreadInfo*)arg; // cast argument to ThreadInfo pointer
    int fd = open(info->path, O_RDONLY); // open the image file, each thread opens its own fd
    if (fd < 0) { //error control
        perror("open");
        pthread_exit(NULL);
    }
    if (lseek(fd, info->offset, SEEK_SET) < 0) {  // move file pointer to the correct starting position
        perror("lseek");
        close(fd);
        pthread_exit(NULL);
    }
    unsigned char buffer[BUFF_SIZE];  // buffer to read data
    int remaining = info->bytesToRead; // number of bytes still left to read
    while (remaining > 0) { // keep reading until this thread finishes its assigned part
        int toRead = remaining > BUFF_SIZE ? BUFF_SIZE : remaining; // decide how many bytes to read in this iteration
        int totalRead = 0;
        while (totalRead < toRead) { // read from file
            int r = read(fd, buffer + totalRead, toRead - totalRead); // try to read the remaining missing bytes from the file
            if (r <= 0) { //error control
                perror("read");
                close(fd);
                pthread_exit(NULL);
            }
            totalRead += r;   // add the number of bytes just read to the total counter
        }
        pthread_mutex_lock(&hist_mutex); // LOCK before updating shared histogram 
        for (int i = 0; i < totalRead; i++) { // increase the count of each pixel value 
            histogram[buffer[i]]++; // increment the histogram count for this pixel value
        }
        pthread_mutex_unlock(&hist_mutex);// unlock the mutex so other threads can update the histogram
        remaining -= totalRead; // update remaining bytes left for this thread
    }
    close(fd); // close the file descriptor for this thread
    pthread_exit(NULL); // terminate the thread execution.
}
int main(int argc, char* argv[]) {
    if (argc != 4) { // check if the user gave exactly 3 arguments and program name
        printf("Input: %s image.pgm histogram.txt numThreads\n", argv[0]);
        return 1;
    }
    int numThreads = atoi(argv[3]); // convert the 3rd argument number of threads
    if (numThreads <= 0) { // make sure the number of threads is greater than 0
        printf("Number of threads must be > 0\n");
        return 1;
    }
    int width, height, maxval; // declare variables image width, height, and maximum grayscale value
    int nBytesHeader = parse_pgm_header(argv[1], &width, &height, &maxval); // call function to read the PGM header and get width, height, max value
    if (nBytesHeader < 0) {// if header parsing failed
        printf("Error parsing PGM header\n");
        return 1;
    }
    if (maxval > 255) { // we only handle 1 byte per pixel
        printf("Expecting 1 byte per pixel\n");
        return 1;
    }
    int nPixels = width * height; // total number of pixels in the image
    int dataSize = nPixels; // number of bytes of image data 
    histogram = calloc(maxval, sizeof(unsigned int)); // allocate memory for the global histogram array
    if (!histogram) {
        perror("calloc histogram");
        return 1;
    }
    if (pthread_mutex_init(&hist_mutex, NULL) != 0) {
        perror("mutex init");
        free(histogram);
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
        if (pthread_create(&threads[i], NULL, worker, &infos[i]) != 0) {
            perror("pthread_create");
            free(histogram);
            free(threads);
            free(infos);
            return 1;
        }
    }
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
        perror("open output");
        free(histogram);
        free(threads);
        free(infos);
        return 1;
    }
    for (int i = 0; i < maxval; i++) {
        char line[64];
        int len = sprintf(line, "%d,%d\n", i, histogram[i]);
        write(fd_out, line, len);
    }
    close(fd_out);
    pthread_mutex_destroy(&hist_mutex);
    free(histogram);
    free(threads);
    free(infos);
    return 0;
}