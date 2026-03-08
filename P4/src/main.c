#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define HIST_SIZE 256

unsigned char *image;
int imageSize;

int *buffer;
int bufferSize;

int in = 0;
int out = 0;
int count = 0;

int nextPixel = 0;
int producers_finished = 0;

int histogram[HIST_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexHist = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        if (nextPixel >= imageSize)
        {
            producers_finished = 1;
            pthread_cond_broadcast(&notEmpty);
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        while (count == bufferSize)
            pthread_cond_wait(&notFull, &mutex);

        int value = image[nextPixel++];
        buffer[in] = value;

        in = (in + 1) % bufferSize;
        count++;

        pthread_cond_signal(&notEmpty);
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        while (count == 0 && !producers_finished)
            pthread_cond_wait(&notEmpty, &mutex);

        if (count == 0 && producers_finished)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        int value = buffer[out];
        out = (out + 1) % bufferSize;
        count--;

        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutexHist);
        histogram[value]++;
        pthread_mutex_unlock(&mutexHist);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        printf("Usage:\n");
        printf("./computeHistogram image.pgm output.txt producers consumers bufferSize\n");
        return 1;
    }

    char *input = argv[1];
    char *output = argv[2];
    int producers = atoi(argv[3]);
    int consumers = atoi(argv[4]);
    bufferSize = atoi(argv[5]);

    FILE *f = fopen(input, "rb");
    if (!f)
    {
        printf("Error opening file\n");
        return 1;
    }

    char format[3];
    int width, height, maxval;

    fscanf(f, "%s", format);
    fscanf(f, "%d %d", &width, &height);
    fscanf(f, "%d", &maxval);
    fgetc(f);

    imageSize = width * height;
    image = malloc(imageSize);

    fread(image, 1, imageSize, f);
    fclose(f);

    buffer = malloc(sizeof(int) * bufferSize);

    for (int i = 0; i < HIST_SIZE; i++)
        histogram[i] = 0;

    pthread_t prod[producers];
    pthread_t cons[consumers];

    for (int i = 0; i < producers; i++)
        pthread_create(&prod[i], NULL, producer, NULL);

    for (int i = 0; i < consumers; i++)
        pthread_create(&cons[i], NULL, consumer, NULL);

    for (int i = 0; i < producers; i++)
        pthread_join(prod[i], NULL);

    for (int i = 0; i < consumers; i++)
        pthread_join(cons[i], NULL);

    FILE *outFile = fopen(output, "w");

    for (int i = 0; i < 255; i++)
        fprintf(outFile, "%d,%d\n", i, histogram[i]);

    fclose(outFile);

    free(image);
    free(buffer);

    return 0;
}