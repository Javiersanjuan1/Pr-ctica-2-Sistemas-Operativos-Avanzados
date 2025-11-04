// procConcurrente.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define DIMFILA 300000
#define NUMFILAS 20
#define SHM_NAME "/matriz_shm_p2"

typedef struct fila {
    int   vector[DIMFILA];
    long  suma;
} fila_t;

static void init_matrix(fila_t *matriz) {
    for (int i = 0; i < NUMFILAS; ++i) {
        matriz[i].suma = 0;
        for (int j = 0; j < DIMFILA; ++j) {
            matriz[i].vector[j] = 10;
        }
    }
}

static void print_sums(const fila_t *matriz) {
    for (int i = 0; i < NUMFILAS; ++i) {
        printf("fila %02d -> suma = %ld\n", i, matriz[i].suma);
    }
}

int main(void) {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    size_t total_size = sizeof(fila_t) * (size_t)NUMFILAS;
    if (ftruncate(fd, (off_t)total_size) == -1) {
        perror("ftruncate");
        shm_unlink(SHM_NAME);
        close(fd);
        return 1;
    }

    void *addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHM_NAME);
        close(fd);
        return 1;
    }
    fila_t *matriz = (fila_t *)addr;

    // Inicializa la matriz en el padre
    init_matrix(matriz);

    // Crea un proceso por fila
    for (int i = 0; i < NUMFILAS; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            munmap(addr, total_size);
            shm_unlink(SHM_NAME);
            close(fd);
            return 1;
        }
        if (pid == 0) {
            // Proceso hijo: suma su fila i
            long acc = 0;
            for (int j = 0; j < DIMFILA; ++j) {
                acc += matriz[i].vector[j];
            }
            matriz[i].suma = acc;
            _exit(0);
        }
        // Padre continúa el bucle para crear más hijos
    }

    // Padre: espera a todos los hijos
    for (int k = 0; k < NUMFILAS; ++k) {
        int status = 0;
        if (wait(&status) == -1) {
            perror("wait");
        }
    }

    // Imprime resultados
    print_sums(matriz);

    // Limpieza
    if (munmap(addr, total_size) == -1) {
        perror("munmap");
    }
    if (close(fd) == -1) {
        perror("close");
    }
    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink");
    }
    return 0;
}

