#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "fisica.h"

typedef struct {
    ParametrosFisicos params;
    ResultadosFisicos resultados;
    int simulacion_ejecutada;
} EstadoSimulador;

void inicializar_simulador(EstadoSimulador *sim);
void ejecutar_simulador(EstadoSimulador *sim);

#endif
