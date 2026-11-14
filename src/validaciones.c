#include "validaciones.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

/* Consume lo que quede en stdin hasta el fin de línea o EOF. */
void limpiar_buffer_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Consume caracteres residuales */
    }
}

/* Lee una línea completa en buf, sin '\n', '\r' ni espacios en los extremos.
 * Devuelve 1 si la línea es utilizable y 0 si superó TAM_LINEA.
 * Ante EOF (Ctrl+Z en Windows / Ctrl+D en Linux) cierra el programa en forma
 * controlada: así es imposible quedar en un bucle infinito de errores. */
static int leer_linea(char *buf, size_t tam) {
    char *inicio;
    size_t len;

    if (fgets(buf, (int)tam, stdin) == NULL) {
        printf("\n[AVISO] Fin de entrada detectado (EOF). Cerrando el simulador.\n");
        exit(EXIT_SUCCESS);
    }
    if (strchr(buf, '\n') == NULL && !feof(stdin)) {
        limpiar_buffer_stdin();      /* línea demasiado larga: se descarta */
        return 0;
    }

    len = strlen(buf);
    while (len > 0 && isspace((unsigned char)buf[len - 1])) {
        buf[--len] = '\0';
    }
    inicio = buf;
    while (*inicio != '\0' && isspace((unsigned char)*inicio)) {
        inicio++;
    }
    if (inicio != buf) {
        memmove(buf, inicio, strlen(inicio) + 1);
    }
    return 1;
}

int leer_entero_rango(const char *mensaje, int min, int max) {
    char linea[TAM_LINEA];
    char *fin;
    long valor;

    for (;;) {
        printf("%s", mensaje);
        if (!leer_linea(linea, sizeof linea)) {
            printf("[ERROR] Entrada demasiado larga.\n");
            continue;
        }
        if (linea[0] == '\0') {
            printf("[ERROR] No ingreso ningun valor.\n");
            continue;
        }
        errno = 0;
        valor = strtol(linea, &fin, 10);
        if (*fin != '\0' || errno == ERANGE) {
            printf("[ERROR] Entrada invalida. Debe ingresar un numero entero.\n");
        } else if (valor < min || valor > max) {
            printf("[ERROR] Valor fuera de rango [%d - %d].\n", min, max);
        } else {
            return (int)valor;
        }
    }
}

double leer_double_rango(const char *mensaje, double min, double max) {
    char linea[TAM_LINEA];
    char *fin;
    char *p;
    double valor;

    for (;;) {
        printf("%s", mensaje);
        if (!leer_linea(linea, sizeof linea)) {
            printf("[ERROR] Entrada demasiado larga.\n");
            continue;
        }
        if (linea[0] == '\0') {
            printf("[ERROR] No ingreso ningun valor.\n");
            continue;
        }
        /* Se acepta la coma decimal (12,5) además del punto (12.5). */
        for (p = linea; *p != '\0'; p++) {
            if (*p == ',') {
                *p = '.';
            }
        }
        errno = 0;
        valor = strtod(linea, &fin);
        if (*fin != '\0' || errno == ERANGE || !isfinite(valor)) {
            printf("[ERROR] Entrada invalida. Debe ingresar un numero real (ej: 12.5).\n");
        } else if (valor < min || valor > max) {
            printf("[ERROR] Valor fuera de rango [%.2f - %.2f].\n", min, max);
        } else {
            return valor;
        }
    }
}

char leer_caracter_sn(const char *mensaje) {
    char linea[TAM_LINEA];
    char c;

    for (;;) {
        printf("%s", mensaje);
        if (leer_linea(linea, sizeof linea) && strlen(linea) == 1) {
            c = (char)toupper((unsigned char)linea[0]);
            if (c == 'S' || c == 'N') {
                return c;
            }
        }
        printf("[ERROR] Responda con una sola letra: S o N.\n");
    }
}
