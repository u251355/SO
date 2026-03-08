#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define BLOCK_SIZE 16384

typedef struct {
    unsigned char data[BLOCK_SIZE]; // store bytes read from file
    int size;              // number of bytes actually read
} block_t;

FILE *file; // shared file pointer
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER; // lock for reading file
int active_producers; // how many producers are running
int producers_done = 0; // flag all producers finished

block_t **buffer; // circular buffer for blocks
int buffer_size;
int in = 0, out = 0, count = 0; // indexes and count of blocks
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER; // lock for buffer
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;  //wait if buffer full
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER; // wait if buffer empty
// global histogram
pthread_mutex_t hist_mutex = PTHREAD_MUTEX_INITIALIZER;  //lock for histogram
int histogram[256] = {0};  // count for each byte
void *producer(void *arg) {
    while (1) {
        block_t *blk = malloc(sizeof(block_t)); // allocate memory for a block
        if (!blk) { //error control
            perror("malloc");
            pthread_exit(NULL);
        }
        pthread_mutex_lock(&file_mutex); //lock file for reading
        blk->size = fread(blk->data, 1, BLOCK_SIZE, file);// read up to 16KB
        if (blk->size == 0) { // if EOF
            free(blk);
            pthread_mutex_unlock(&file_mutex); // unlock file
            pthread_mutex_lock(&buffer_mutex);
            active_producers--; // decrement running producers
            if (active_producers == 0) { //if last producer signal all producers done and wake up consumers
                producers_done = 1;
                pthread_cond_broadcast(&notEmpty); // despertar consumidores
            }
            pthread_mutex_unlock(&buffer_mutex); //unlock
            pthread_exit(NULL);//exit thread
        }
        pthread_mutex_unlock(&file_mutex);
        pthread_mutex_lock(&buffer_mutex); // lock buffer to insert block
        while (count == buffer_size) { // wait if buffer full
            pthread_cond_wait(&notFull, &buffer_mutex);
        }
        buffer[in] = blk; // put block in buffer
        in = (in + 1) % buffer_size; // circular increment
        count++; //increase counter
        pthread_cond_signal(&notEmpty); // signal consumer
        pthread_mutex_unlock(&buffer_mutex);
    }
    return NULL;
}
void *consumer(void *arg) {
    while (1) {
        pthread_mutex_lock(&buffer_mutex);  // lock buffer to take block
        while (count == 0 && !producers_done) {  // wait if buffer empty and producers still working
            pthread_cond_wait(&notEmpty, &buffer_mutex);
        }
        if (count == 0 && producers_done) { // if empty and all done
            pthread_mutex_unlock(&buffer_mutex);
            pthread_exit(NULL);  //exit thread
        }
        block_t *blk = buffer[out]; // take block from buffer
        out = (out + 1) % buffer_size;  //circular increment
        count--; //decrease block count
        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&buffer_mutex);
        pthread_mutex_lock(&hist_mutex);
        for (int i = 0; i < blk->size; i++) {  //for each byte in block
            histogram[blk->data[i]]++;  //update histogram
        }
        pthread_mutex_unlock(&hist_mutex);  // unlock histogram
        free(blk); //free block memory
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Use: %s image.pgm output.txt producers consumers tam_buffer\n", argv[0]); 
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