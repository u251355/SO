#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define BLOCK_SIZE 16384

typedef struct {
    unsigned char data[BLOCK_SIZE];
    int size;               // número de bytes realmente leídos
} block_t;

// Archivo compartido
FILE *file;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
int active_producers;
int producers_done = 0;

// Buffer circular de bloques
block_t **buffer;
int buffer_size;
int in = 0, out = 0, count = 0;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;

// Histograma global
pthread_mutex_t hist_mutex = PTHREAD_MUTEX_INITIALIZER;
int histogram[256] = {0};
void *producer(void *arg) {
    while (1) {
        // Reservar memoria para un nuevo bloque
        block_t *blk = malloc(sizeof(block_t));
        if (!blk) {
            perror("malloc");
            pthread_exit(NULL);
        }
        // Leer del archivo (protegido con mutex)
        pthread_mutex_lock(&file_mutex);
        blk->size = fread(blk->data, 1, BLOCK_SIZE, file);
        if (blk->size == 0) {
            // EOF: no hay más datos
            free(blk);
            pthread_mutex_unlock(&file_mutex);
            // Decrementar contador de productores activos
            pthread_mutex_lock(&buffer_mutex);
            active_producers--;
            if (active_producers == 0) {
                producers_done = 1;
                pthread_cond_broadcast(&notEmpty); // despertar consumidores
            }
            pthread_mutex_unlock(&buffer_mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&file_mutex);
        // Insertar el bloque en el buffer circular
        pthread_mutex_lock(&buffer_mutex);
        while (count == buffer_size) {
            pthread_cond_wait(&notFull, &buffer_mutex);
        }
        buffer[in] = blk;
        in = (in + 1) % buffer_size;
        count++;
        pthread_cond_signal(&notEmpty);
        pthread_mutex_unlock(&buffer_mutex);
    }
    return NULL;
}
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
        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&buffer_mutex);
        // Procesar el bloque: actualizar histograma
        pthread_mutex_lock(&hist_mutex);
        for (int i = 0; i < blk->size; i++) {
            histogram[blk->data[i]]++;
        }
        pthread_mutex_unlock(&hist_mutex);

        free(blk);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s imagen.pgm salida.txt productores consumidores tam_buffer\n", argv[0]);
        return 1;
    }
    char *input_file = argv[1];
    char *output_file = argv[2];
    int num_producers = atoi(argv[3]);
    int num_consumers = atoi(argv[4]);
    buffer_size = atoi(argv[5]);
    if (num_producers <= 0 || num_consumers <= 0 || buffer_size <= 0) {
        fprintf(stderr, "Los números deben ser positivos.\n");
        return 1;
    }
    // Abrir archivo de imagen en modo binario
    file = fopen(input_file, "rb");
    if (!file) {
        perror("fopen");
        return 1;
    }
    // ---- Leer cabecera PGM ----
    char format[3];
    int width, height, maxval;
    // Formato (P5, etc.)
    if (fscanf(file, "%2s", format) != 1) {
        fprintf(stderr, "Error leyendo formato PGM\n");
        fclose(file);
        return 1;
    }
    // Saltar posibles comentarios (líneas que empiezan con '#')
    int c;
    while ((c = fgetc(file)) == '#') {
        while (fgetc(file) != '\n'); // saltar línea completa
    }
    ungetc(c, file); // devolver el carácter leído (que no es '#')

    // Leer dimensiones
    if (fscanf(file, "%d %d", &width, &height) != 2) {
        fprintf(stderr, "Error leyendo dimensiones\n");
        fclose(file);
        return 1;
    }
    // Leer valor máximo
    if (fscanf(file, "%d", &maxval) != 1) {
        fprintf(stderr, "Error leyendo maxval\n");
        fclose(file);
        return 1;
    }
    // Saltar el salto de línea después del maxval
    fgetc(file);
    // A partir de aquí el puntero está al inicio de los datos de píxeles
    // ---------------------------------
    // Inicializar buffer
    buffer = malloc(sizeof(block_t *) * buffer_size);
    if (!buffer) {
        perror("malloc buffer");
        fclose(file);
        return 1;
    }
    active_producers = num_producers;
    pthread_t *producers = malloc(sizeof(pthread_t) * num_producers);
    pthread_t *consumers = malloc(sizeof(pthread_t) * num_consumers);
    if (!producers || !consumers) {
        perror("malloc hilos");
        free(buffer);
        fclose(file);
        return 1;
    }
    // Crear hilos productores
    for (int i = 0; i < num_producers; i++) {
        if (pthread_create(&producers[i], NULL, producer, NULL) != 0) {
            perror("pthread_create productor");
            exit(1);
        }
    }
    // Crear hilos consumidores
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
    // Escribir el histograma en el archivo de salida
    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("fopen salida");
    } else {
        // Solo se imprimen los valores 0..254 (como en el ejemplo proporcionado)
        for (int i = 0; i < 255; i++) {
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