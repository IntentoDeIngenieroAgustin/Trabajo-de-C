#ifndef VALIDACIONES_H
#define VALIDACIONES_H

/* Tamaño máximo de una línea de entrada (incluye '\n' y '\0'). */
#define TAM_LINEA 128

void   limpiar_buffer_stdin(void);
int    leer_entero_rango(const char *mensaje, int min, int max);
double leer_double_rango(const char *mensaje, double min, double max);
char   leer_caracter_sn(const char *mensaje);

#endif
