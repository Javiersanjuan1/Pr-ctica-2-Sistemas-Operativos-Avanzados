// procConcurrente.c
// Compila en Linux/WSL/macOS con procesos + memoria compartida (POSIX)
// y en Windows con una ruta alternativa basada en hilos (para poder compilar).

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DIMFILA 300000
#define NUMFILAS 20

typedef struct fila {
    int  vector[DIMFILA];
    long suma;
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

#ifdef _WIN32
// ---------------------------
// RUTA WINDOWS (plan B: hilos)
// ---------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>  // _beginthreadex

static fila_t matriz_global[NUMFILAS];

typedef struct {
    int idx;
} tarea_t;

static unsigned __stdcall worker(void *arg) {
    tarea_t *t = (tarea_t*)arg;
    int i = t->idx;
    long acc = 0;
    for (int j = 0; j < DIMFILA; ++j) {
        acc += matriz_global[i].vector[j];
    }
    matriz_global[i].suma = acc;
    return 0;
}

int main(void) {
    init_matrix(matriz_global);

    HANDLE threads[NUMFILAS];
    tarea_t tareas[NUMFILAS];

    for (int i = 0; i < NUMFILAS; ++i) {
        tareas[i].idx = i;
        uintptr_t th = _beginthreadex(NULL, 0, worker, &tareas[i], 0, NULL);
        if (!th) {
            fprintf(stderr, "_beginthreadex fallo en i=%d\n", i);
            return 1;
        }
        threads[i] = (HANDLE)th;
    }

    WaitForMultipleObjects(NUMFILAS, threads, TRUE, INFINITE);
    for (int i = 0; i < NUMFILAS; ++i) CloseHandle(threads[i]);

    print_sums(matriz_global);
    return 0;
}

#else
// -----------------------------------------
// RUTA POSIX (Linux/WSL/macOS): PROCESOS
// -----------------------------------------
#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "/matriz_shm_p2"

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

    init_matrix(matriz);

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
            long acc = 0;
            for (int j = 0; j < DIMFILA; ++j) {
                acc += matriz[i].vector[j];
            }
            matriz[i].suma = acc;
            _exit(0);
        }
    }

    for (int k = 0; k < NUMFILAS; ++k) {
        int status = 0;
        if (wait(&status) == -1) perror("wait");
    }

    print_sums(matriz);

    if (munmap(addr, total_size) == -1) perror("munmap");
    if (close(fd) == -1) perror("close");
    if (shm_unlink(SHM_NAME) == -1) perror("shm_unlink");
    return 0;
}
#endif
