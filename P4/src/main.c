#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "parsePGM.h"

#define BLOCK_SIZE (1024*16)
#define HIST_SIZE 256

typedef struct {
    unsigned char *data;
    int size;
} block_t;

block_t *buffer;
int sizeBuffer;

int in = 0;
int out = 0;
int elementsInBuffer = 0;

int histogram[HIST_SIZE] = {0};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lock_read = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t hist_mutex = PTHREAD_MUTEX_INITIALIZER;

int producers_finished = 0;
int Nprod;

off_t readPos;
int blockSize = BLOCK_SIZE;

int fd;
char *inputFile;

void *producer(void *arg)
{
    int blockSize = *((int *)arg);
    int nBytesRead;
    off_t readPosLocal;

    while(1)
    {
        pthread_mutex_lock(&lock_read);
        readPosLocal = readPos;
        readPos += blockSize;
        pthread_mutex_unlock(&lock_read);

        lseek(fd, readPosLocal, SEEK_SET);

        unsigned char *buff = malloc(blockSize);
        if(buff == NULL)
        {
            perror("malloc");
            exit(1);
        }

        nBytesRead = read(fd, buff, blockSize);

        if(nBytesRead == 0)
        {
            free(buff);
            break;
        }

        if(nBytesRead < 0)
        {
            perror("read");
            free(buff);
            break;
        }

        pthread_mutex_lock(&mutex);

        while(elementsInBuffer == sizeBuffer)
            pthread_cond_wait(&notFull, &mutex);

        buffer[in].data = buff;
        buffer[in].size = nBytesRead;

        in = (in + 1) % sizeBuffer;
        elementsInBuffer++;

        pthread_cond_signal(&notEmpty);
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
    producers_finished++;
    pthread_cond_broadcast(&notEmpty);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *Consumer(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex);

        while(elementsInBuffer == 0 && producers_finished < Nprod)
            pthread_cond_wait(&notEmpty, &mutex);

        if(elementsInBuffer == 0 && producers_finished == Nprod)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        unsigned char *data = buffer[out].data;
        int dataSize = buffer[out].size;

        out = (out + 1) % sizeBuffer;
        elementsInBuffer--;

        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&hist_mutex);

        for(int i = 0; i < dataSize; i++)
            histogram[data[i]]++;

        pthread_mutex_unlock(&hist_mutex);

        free(data);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if(argc != 6)
    {
        printf("Usage: computeHistogram input.pgm output.txt Nprod Ncons sizeBuffer\n");
        return 1;
    }

    inputFile = argv[1];
    char *output = argv[2];

    Nprod = atoi(argv[3]);
    int Ncons = atoi(argv[4]);
    sizeBuffer = atoi(argv[5]);

    int width, height, maxval;

    int header = parse_pgm_header(inputFile, &width, &height, &maxval);

    if(header < 0)
    {
        printf("Error reading PGM header\n");
        return 1;
    }

    fd = open(inputFile, O_RDONLY);

    if(fd < 0)
    {
        perror("open");
        return 1;
    }

    readPos = header;

    buffer = malloc(sizeof(block_t) * sizeBuffer);

    if(buffer == NULL)
    {
        perror("malloc");
        return 1;
    }

    pthread_t producers[Nprod];
    pthread_t consumers[Ncons];

    for(int i = 0; i < Nprod; i++)
        pthread_create(&producers[i], NULL, producer, &blockSize);

    for(int i = 0; i < Ncons; i++)
        pthread_create(&consumers[i], NULL, Consumer, NULL);

    for(int i = 0; i < Nprod; i++)
        pthread_join(producers[i], NULL);

    for(int i = 0; i < Ncons; i++)
        pthread_join(consumers[i], NULL);

    FILE *f = fopen(output, "w");

    if(f == NULL)
    {
        perror("fopen");
        return 1;
    }

    for(int i = 0; i < 256; i++)
        fprintf(f, "%d %d\n", i, histogram[i]);

    fclose(f);

    free(buffer);
    close(fd);

    return 0;
}