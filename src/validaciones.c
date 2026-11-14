#include "validaciones.h"
#include <stdio.h>

void limpiar_buffer_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Consume caracteres residuales */
    }
}

int leer_entero_rango(const char *mensaje, int min, int max) {
    int valor;
    int leido;
    do {
        printf("%s", mensaje);
        leido = scanf("%d", &valor);
        if (leido != 1) {
            printf("[ERROR] Entrada invalida. Debe ingresar un numero entero.\n");
            limpiar_buffer_stdin();
        } else if (valor < min || valor > max) {
            printf("[ERROR] Valor fuera de rango [%d - %d].\n", min, max);
            limpiar_buffer_stdin();
            leido = 0;
        } else {
            limpiar_buffer_stdin();
        }
    } while (leido != 1);
    return valor;
}

double leer_double_positivo(const char *mensaje) {
    double valor;
    int leido;
    do {
        printf("%s", mensaje);
        leido = scanf("%lf", &valor);
        if (leido != 1 || valor <= 0.0) {
            printf("[ERROR] Ingrese un valor numerico estrictamente positivo.\n");
            limpiar_buffer_stdin();
            leido = 0;
        } else {
            limpiar_buffer_stdin();
        }
    } while (leido != 1);
    return valor;
}
