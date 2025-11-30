#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>   // para sleep() y usleep
#include <time.h>     // para srand

// Estructura Barrera: monitor reutilizable
typedef struct {
    int count;              // Hebras que han llegado
    int N;                  // Número total de hebras
    int etapa;              // Etapa actual
    pthread_mutex_t mtx;    // Exclusión mutua
    pthread_cond_t cond;    // Condición de espera
} Barrera;

// Crear e inicializar
void barrera_init(Barrera* b, int numHebras) {
    b->count = 0;
    b->N = numHebras;
    b->etapa = 0;
    pthread_mutex_init(&b->mtx, NULL);
    pthread_cond_init(&b->cond, NULL);
}

// Destruir
void barrera_destroy(Barrera* b) {
    pthread_mutex_destroy(&b->mtx);
    pthread_cond_destroy(&b->cond);
}

// Operación fundamental: wait()
void barrera_wait(Barrera* b) {
    pthread_mutex_lock(&b->mtx);

    int etapaLocal = b->etapa;   
    b->count++;

    if (b->count == b->N) {
        // Última hebra en llegar → reinicia la barrera
        b->count = 0;
        b->etapa++;
        pthread_cond_broadcast(&b->cond); // despierta a todas
    } else {
        // Las demás esperan hasta que cambie la etapa
        while (b->etapa == etapaLocal) {
            pthread_cond_wait(&b->cond, &b->mtx);
        }
    }

    pthread_mutex_unlock(&b->mtx);
}

// Estructura para pasar argumentos a las hebras
typedef struct {
    Barrera* b;
    int id;
    int etapas;
} Args;

// Mutex global para imprimir ordenado
pthread_mutex_t print_mtx = PTHREAD_MUTEX_INITIALIZER;

void print_line(const char* s) {
    pthread_mutex_lock(&print_mtx);
    printf("%s\n", s);
    pthread_mutex_unlock(&print_mtx);
}

// Función que ejecuta cada hebra
void* tarea(void* arg) {
    Args* datos = (Args*) arg;
    Barrera* b = datos->b;
    int id = datos->id;
    int etapas = datos->etapas;

    for (int e = 0; e < etapas; e++) {
        // trabajo simulado con usleep aleatorio (50–150 ms)
        int delay = (rand() % 100 + 50) * 1000;
        usleep(delay);

        char buffer[100];
        sprintf(buffer, "[%d] esperando en etapa %d", id, e);
        print_line(buffer);

        barrera_wait(b);

        sprintf(buffer, "[%d] paso barrera en etapa %d", id, e);
        print_line(buffer);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int N = 5; 
    int E = 4;  

    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) E = atoi(argv[2]);

    srand(time(NULL)); 

    Barrera barrera;
    barrera_init(&barrera, N);

    pthread_t hilos[N];
    Args args[N];

    // Crear hebras
    for (int i = 0; i < N; i++) {
        args[i].b = &barrera;
        args[i].id = i;
        args[i].etapas = E;
        pthread_create(&hilos[i], NULL, tarea, &args[i]);
    }

    // Esperar a que terminen
    for (int i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
    }

    barrera_destroy(&barrera);
    return 0;
}
