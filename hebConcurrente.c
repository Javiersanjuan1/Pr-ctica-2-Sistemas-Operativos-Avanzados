// hebConcurrente.c
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
    intptr_t idx = (intptr_t)arg;   // índice de fila
    long acc = 0;
    for (int j = 0; j < DIMFILA; ++j) {
        acc += matriz[idx].vector[j];
    }
    matriz[idx].suma = acc;
    return NULL;
}

static void print_sums(void) {
    for (int i = 0; i < NUMFILAS; ++i) {
        printf("fila %02d -> suma = %ld\n", i, matriz[i].suma);
    }
}

int main(void) {
    init_matrix();

    pthread_t tids[NUMFILAS];
    for (int i = 0; i < NUMFILAS; ++i) {
        if (pthread_create(&tids[i], NULL, worker, (void*)(intptr_t)i) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < NUMFILAS; ++i) {
        if (pthread_join(tids[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    print_sums();
    return 0;
}

