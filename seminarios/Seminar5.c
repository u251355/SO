//Ex1
#include <pthread.h>
#include <unistd.h>
void* wait_thread(void* arg) {//codigo para el create donde dices que duerman
    sleep(1);
    return NULL;
}
int main() {
    pthread_t threads[100];//define los threads
    for (int i = 0; i < NUM_THREADS; i++) {//bucle para crear muchos
        pthread_create(&threads[i], NULL, wait_thread, NULL);//crea los threads, null porque no necesita rgumento
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL); //espera a que el thread acabe
    }
}
//EX2
//a
int A[100];
void* square(void* arg) {
    int* index = (int*)arg; //el indice sera lo que reciba como argumento, casteamos
    A[*index] = (*index) * (*index); //cada array sera su &indice por su &indice
    return NULL;
}
int main() {
    pthread_t threads[100]; //def
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, square, &i);//crea, como parametro le pasas &i
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);//espera a los 100
    }
    return 0;
}
//b
int A[100];
void* allocate(void* arg) {
    int index = *(int*)arg;//cast a int
    free(arg);
    A[index] = index * index;//index por index
    return NULL;
}
int main() {
    pthread_t threads[100]; //def
    for (int i = 0; i < NUM_THREADS; i++) {
        int* index = malloc(sizeof(int)); //reservamos memoria para guardar int del index
        *index = i;//lo guardamos en i
        pthread_create(&threads[i], NULL, allocate, index);//crea pasandole index, no su direccion
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);//espera a los 100
    }
    return 0;
}