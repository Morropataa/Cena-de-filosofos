#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 5
#define RONDAS 3

static pthread_mutex_t tenedores[N];
static pthread_mutex_t impresion = PTHREAD_MUTEX_INITIALIZER;

static void pausa(long milisegundos) {
    struct timespec t = {milisegundos / 1000, (milisegundos % 1000) * 1000000L};
    nanosleep(&t, NULL);
}

static void mensaje(int id, int ronda, const char *accion) {
    pthread_mutex_lock(&impresion);
    printf("Filosofo %d | ronda %d | %s\n", id + 1, ronda, accion);
    fflush(stdout);
    pthread_mutex_unlock(&impresion);
}

static void *filosofo(void *arg) {
    int id = *(int *)arg;
    int izq = id;
    int der = (id + 1) % N;
    /* Orden global: siempre adquirir primero el mutex de indice menor. */
    int primero = izq < der ? izq : der;
    int segundo = izq < der ? der : izq;

    for (int ronda = 1; ronda <= RONDAS; ronda++) {
        mensaje(id, ronda, "pensando");
        pausa(30 + id * 10);
        pthread_mutex_lock(&tenedores[primero]);
        pthread_mutex_lock(&tenedores[segundo]);
        mensaje(id, ronda, "comiendo");
        pausa(35);
        pthread_mutex_unlock(&tenedores[segundo]);
        pthread_mutex_unlock(&tenedores[primero]);
        mensaje(id, ronda, "termino de comer y libero los tenedores");
    }
    return NULL;
}

int main(void) {
    pthread_t hilos[N];
    int ids[N];
    for (int i = 0; i < N; i++) {
        if (pthread_mutex_init(&tenedores[i], NULL) != 0) {
            fprintf(stderr, "Error al inicializar mutex\n");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        if (pthread_create(&hilos[i], NULL, filosofo, &ids[i]) != 0) {
            fprintf(stderr, "Error al crear hilo\n");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < N; i++) {
        if (pthread_join(hilos[i], NULL) != 0) {
            fprintf(stderr, "Error al esperar hilo\n");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < N; i++) pthread_mutex_destroy(&tenedores[i]);
    pthread_mutex_destroy(&impresion);
    puts("Todos los filosofos terminaron sus 3 rondas.");
    return EXIT_SUCCESS;
}
