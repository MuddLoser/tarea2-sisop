#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h> 

typedef struct {
    int valid;                
    unsigned long numero_pagina; 
    int referencia;
} Marco;

// estadisticas
unsigned long total_referencias = 0;
unsigned long total_fallos = 0;

int reemplazo_reloj(Marco *marcos, int num_marcos, int *reloj_puntero) {
    while (true) {
        if (marcos[*reloj_puntero].referencia == 0) {
            int reemplazo = *reloj_puntero;
            *reloj_puntero = (*reloj_puntero + 1) % num_marcos;
            return reemplazo;
        }
        marcos[*reloj_puntero].referencia = 0;
        *reloj_puntero = (*reloj_puntero + 1) % num_marcos;
    }
}

int buscar_en_tabla(Marco *marcos, int n_marcos, unsigned long vpn) {
    for (int i = 0; i < n_marcos; i++) {

        if (marcos[i].valid && marcos[i].numero_pagina == vpn) {
            return i; // HIT
        }
    }
    return -1; // FALLO
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "uso: %s Nmarcos tamano_marco [--verbose] traza.txt\n", argv[0]);
        return 1;
    }

    int n_marcos = atoi(argv[1]);
    int page_size = atoi(argv[2]);
    char *trace_file_path = NULL;
    int verbose = 0;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else {
            trace_file_path = argv[i];
        }
    }

    if (!trace_file_path) {
        fprintf(stderr, "error: no se especifica archivo de traza\n");
        return 1;
    }

    FILE *file = fopen(trace_file_path, "r");
    if (!file) {
        perror("error al abrir archivo");
        return 1;
    }

    Marco *marcos = (Marco *)malloc(sizeof(Marco) * n_marcos);
    for (int i = 0; i < n_marcos; i++) {
        marcos[i].valid = 0;
        marcos[i].referencia = 0;
        marcos[i].numero_pagina = 0;
    }

    int clock_hand = 0;

    int b = (int)log2(page_size); 
    unsigned long mask = page_size - 1;

    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        unsigned long dv = strtoul(line, NULL, 0);
        total_referencias++;

        unsigned long offset = dv & mask;
        unsigned long npv = dv >> b;

        int marco_fisico = buscar_en_tabla(marcos, n_marcos, npv);
        char *estado = "HIT";

        if (marco_fisico != -1) {
            marcos[marco_fisico].referencia = 1;
        } else {
            estado = "FALLO";
            total_fallos++;

            int marco_libre = -1;
            for (int i = 0; i < n_marcos; i++) {
                if (!marcos[i].valid) {
                    marco_libre = i;
                    break;
                }
            }

            if (marco_libre != -1) {

                marco_fisico = marco_libre;
                marcos[marco_fisico].valid = 1;
                marcos[marco_fisico].numero_pagina = npv;
                marcos[marco_fisico].referencia = 1; 
            } else {
                marco_fisico = reemplazo_reloj(marcos, n_marcos, &clock_hand);
                
                marcos[marco_fisico].numero_pagina = npv;
                marcos[marco_fisico].referencia = 1;
            }
        }

        unsigned long df = ((unsigned long)marco_fisico << b) | offset;

        if (verbose) {
            printf("DV: 0x%lx, nvp: 0x%lx, offset: 0x%lx, %s, marco: %d, DF: 0x%lx\n",
                   dv, npv, offset, estado, marco_fisico, df);
        }
    }

    double tasa_fallos = (total_referencias > 0) ? (double)total_fallos / total_referencias : 0.0;
    
    printf("referencias: %lu\n", total_referencias);
    printf("fallos de pagina: %lu\n", total_fallos);
    printf("tasa de fallos: %.4f\n", tasa_fallos);

    free(marcos);
    fclose(file);

    return 0;
}