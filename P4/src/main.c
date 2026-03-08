/*
 * Productor‑Consumidor con múltiples productores y consumidores
 * 
 * Compilación: gcc -pthread -o prodcons prodcons.c
 * Uso: ./prodcons fichero_entrada fichero_salida productores consumidores tam_buffer
 *
 * El tamaño de buffer se refiere al número de bloques (de 16384 bytes) que puede almacenar.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BLOCK_SIZE 16384

// Estructura de un bloque de datos
typedef struct {
    unsigned char data[BLOCK_SIZE];
    int size;               // número de bytes realmente leídos (último bloque puede ser menor)
} block_t;

// Variables globales para el fichero compartido
FILE *file;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
int active_producers;               // número de productores que aún no han terminado
int producers_done = 0;              // bandera que indica que todos los productores han acabado

// Buffer circular de bloques
block_t **buffer;                    // array de punteros a bloque
int buffer_size;                     // capacidad del buffer (número de bloques)
int in = 0, out = 0, count = 0;      // índices y contador de elementos
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;

// Histograma global (protegido con su propio mutex)
pthread_mutex_t hist_mutex = PTHREAD_MUTEX_INITIALIZER;
int histogram[256] = {0};

// Función del productor
void *producer(void *arg) {
    while (1) {
        // Reservar memoria para un nuevo bloque
        block_t *blk = malloc(sizeof(block_t));
        if (!blk) {
            perror("malloc");
            pthread_exit(NULL);
        }

        // Leer del fichero (protegido con mutex)
        pthread_mutex_lock(&file_mutex);
        blk->size = fread(blk->data, 1, BLOCK_SIZE, file);
        if (blk->size == 0) {
            // Se ha alcanzado el final del fichero
            free(blk);
            pthread_mutex_unlock(&file_mutex);   // liberar el mutex del fichero

            // Actualizar el contador de productores activos
            pthread_mutex_lock(&buffer_mutex);
            active_producers--;
            if (active_producers == 0) {
                producers_done = 1;
                // Despertar a todos los consumidores que pudieran estar esperando
                pthread_cond_broadcast(&notEmpty);
            }
            pthread_mutex_unlock(&buffer_mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&file_mutex);

        // Intentar colocar el bloque en el buffer circular
        pthread_mutex_lock(&buffer_mutex);
        while (count == buffer_size) {
            // Buffer lleno: esperar a que haya hueco
            pthread_cond_wait(&notFull, &buffer_mutex);
        }
        // Insertar el bloque
        buffer[in] = blk;
        in = (in + 1) % buffer_size;
        count++;
        // Avisar a los consumidores de que hay datos
        pthread_cond_signal(&notEmpty);
        pthread_mutex_unlock(&buffer_mutex);
    }
    return NULL;
}

// Función del consumidor
void *consumer(void *arg) {
    while (1) {
        pthread_mutex_lock(&buffer_mutex);
        // Esperar mientras no haya datos y los productores sigan activos
        while (count == 0 && !producers_done) {
            pthread_cond_wait(&notEmpty, &buffer_mutex);
        }
        // Si no hay datos y todos los productores han terminado, salir
        if (count == 0 && producers_done) {
            pthread_mutex_unlock(&buffer_mutex);
            pthread_exit(NULL);
        }
        // Extraer un bloque del buffer
        block_t *blk = buffer[out];
        out = (out + 1) % buffer_size;
        count--;
        // Avisar a los productores de que hay espacio libre
        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&buffer_mutex);

        // Procesar el bloque: actualizar el histograma
        pthread_mutex_lock(&hist_mutex);
        for (int i = 0; i < blk->size; i++) {
            histogram[blk->data[i]]++;
        }
        pthread_mutex_unlock(&hist_mutex);

        // Liberar la memoria del bloque
        free(blk);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s fichero_entrada fichero_salida productores consumidores tam_buffer\n", argv[0]);
        return 1;
    }

    char *input_file = argv[1];
    char *output_file = argv[2];
    int num_producers = atoi(argv[3]);
    int num_consumers = atoi(argv[4]);
    buffer_size = atoi(argv[5]);

    if (num_producers <= 0 || num_consumers <= 0 || buffer_size <= 0) {
        fprintf(stderr, "Los números de productores, consumidores y tamaño de buffer deben ser positivos.\n");
        return 1;
    }

    // Abrir el fichero de entrada en modo binario
    file = fopen(input_file, "rb");
    if (!file) {
        perror("fopen");
        return 1;
    }

    // Inicializar el buffer
    buffer = malloc(sizeof(block_t *) * buffer_size);
    if (!buffer) {
        perror("malloc buffer");
        fclose(file);
        return 1;
    }

    active_producers = num_producers;

    // Crear los hilos productores
    pthread_t *producers = malloc(sizeof(pthread_t) * num_producers);
    pthread_t *consumers = malloc(sizeof(pthread_t) * num_consumers);
    if (!producers || !consumers) {
        perror("malloc hilos");
        free(buffer);
        fclose(file);
        return 1;
    }

    for (int i = 0; i < num_producers; i++) {
        if (pthread_create(&producers[i], NULL, producer, NULL) != 0) {
            perror("pthread_create productor");
            // Cancelar los ya creados y salir (simplificado)
            exit(1);
        }
    }

    for (int i = 0; i < num_consumers; i++) {
        if (pthread_create(&consumers[i], NULL, consumer, NULL) != 0) {
            perror("pthread_create consumidor");
            exit(1);
        }
    }

    // Esperar a que terminen todos los productores
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers[i], NULL);
    }

    // Esperar a que terminen todos los consumidores
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Escribir el histograma en el fichero de salida
    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("fopen salida");
    } else {
        for (int i = 0; i < 256; i++) {
            fprintf(out, "%d,%d\n", i, histogram[i]);
        }
        fclose(out);
    }

    // Liberar recursos
    free(producers);
    free(consumers);
    free(buffer);
    fclose(file);

    return 0;
}