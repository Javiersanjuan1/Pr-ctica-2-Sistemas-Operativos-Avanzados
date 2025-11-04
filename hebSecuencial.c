// hebSecuencial.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#define DIMFILA 300000
#define NUMFILAS 20

typedef struct fila {
    int   vector[DIMFILA];
    long  suma;
} fila_t;

static fila_t matriz[NUMFILAS];

static void init_matrix(void) {
    for (int i = 0; i < NUMFILAS; ++i) {
        matriz[i].suma = 0;
        for (int j = 0; j < DIMFILA; ++j) {
            matriz[i].vector[j] = 10;
        }
    }
}

static void *worker(void *arg) {
    (void)arg;
    for (int i = 0; i < NUMFILAS; ++i) {
        long acc = 0;
        for (int j = 0; j < DIMFILA; ++j) {
            acc += matriz[i].vector[j];
        }
        matriz[i].suma = acc;
    }
    return NULL;
}

static void print_sums(void) {
    for (int i = 0; i < NUMFILAS; ++i) {
        printf("fila %02d -> suma = %ld\n", i, matriz[i].suma);
    }
}

int main(void) {
    init_matrix();

    pthread_t tid;
    if (pthread_create(&tid, NULL, worker, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    if (pthread_join(tid, NULL) != 0) {
        perror("pthread_join");
        return 1;
    }

    print_sums();
    return 0;
}
