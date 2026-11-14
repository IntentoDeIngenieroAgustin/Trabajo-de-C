#include "simulador.h"

void inicializar_simulador(EstadoSimulador *sim) {
    sim->params.v0 = 0.0;
    sim->params.h0 = 0.0;
    sim->params.a1 = 20.0;
    sim->params.t1 = 120.0;
    sim->params.a2 = 12.5;
    sim->params.h_corte = 160000.0;
    sim->params.g = 9.80;
    sim->simulacion_ejecutada = 0;
}

void ejecutar_simulador(EstadoSimulador *sim) {
    ejecutar_calculos_cinematicos(&sim->params, &sim->resultados);
    sim->simulacion_ejecutada = 1;
}
