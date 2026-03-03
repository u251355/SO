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
//EX3
//a
int A[10];//array con 0 o 2 en cad aposicion
typedef struct { ///i es origen, j es destino
    int i, j;
} move_t;
void *move(void *arg) { //codigo que le pasas al thread
    move_t *m = (move_t *)arg; //casteas el arg
    if (A[m->i] > 0) { //solo movemos si hay particulas
        A[m->i]--; //mueves particula
        A[m->j]++;// quitas del origen
    }
    free(m);
    return NULL;
}
int main() {
    pthread_t th[5]; //creamos 5 hilos
    for (int k = 0; k < 5; k++) {//cada thread
        move_t *m = malloc(sizeof(move_t));
        m->i = 2*k;
        m->j = 2*k + 1;
        pthread_create(&th[k], NULL, move, m);//los creas y le pasas m
    }
    for (int k = 0; k < 5; k++)//cada thread espera hasta que todos hayan acabdo
        pthread_join(th[k], NULL);
    return 0;
}
//no necesitamos metodos de sincronizaxion porque cada thread pasa de uno mas pequeño a aotro mas grande, no hay riesgo de colision
//b
//aqui si que necesitamos una sincronizacion porque sino puede haber "race conditions"
int A[10];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //siempre primero inicializar el mutex
typedef struct {
    int i, j;
} move_t;
void *move(void *arg) {
    move_t *m = (move_t *)arg;
    pthread_mutex_lock(&lock);//lockeamos
    if (A[m->i] > 0) {
        A[m->i]--;
        A[m->j]++;
    }
    pthread_mutex_unlock(&lock);//unlockeamos
    free(m);
    return NULL;
}

//c
//lock
//Ex4
#define NUM_RESOURCES 5 //funcion que nos dan
int nThreadsAccessing = 0;
int getFromDatabase() {
nThreadsAccessing++;
printf("Number of Threads accessing the DB = %d\n",
nThreadsAccessing);
if (nThreadsAccessing > NUM_RESOURCES) _exit(1);
usleep(rand()%10000);
nThreadsAccessing--;
return rand();
}
sem_t sem; //semaphor
int main(){
    pthread_t threads[20]; //inicializamos
    sem_init(&sem, 0, NUM_RESOURCES); //inicializamos
    for int(i=0; i<20; i++){ //creamos 20 threads
        pthread_create(&threads[i], NULL, getFromDatabase, NULL)
    }
    for (int i = 0; i < 20; i++)
        pthread_join(th[i], NULL);//siempre hay que esperar
        sem_destroy(&sem); //siempre hay que destruir los semaphores 
    return 0;




}


