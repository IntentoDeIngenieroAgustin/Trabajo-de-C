#include "reportes.h"
#include <stdio.h>

int guardar_reporte_mision(const EstadoSimulador *sim, const char *nombre_archivo) {
    FILE *f = fopen(nombre_archivo, "w");
    if (!f) return 0;

    fprintf(f, "===============================================================\n");
    fprintf(f, "       REPORTE TECNICO DE SIMULACION - PROYECTO ARTEMIS        \n");
    fprintf(f, "       Cátedra de Algoritmos y EE.DD. - UTN FRRe               \n");
    fprintf(f, "===============================================================\n\n");
    fprintf(f, "[PARAMETROS DE ENTRADA]\n");
    fprintf(f, "Velocidad inicial (v0):    %.2f m/s\n", sim->params.v0);
    fprintf(f, "Aceleración Fase 1 (a1):   %.2f m/s^2\n", sim->params.a1);
    fprintf(f, "Duración Fase 1 (t1):      %.2f s\n", sim->params.t1);
    fprintf(f, "Aceleración Fase 2 (a2):   %.2f m/s^2\n", sim->params.a2);
    fprintf(f, "Altitud de corte (h_corte):%.2f m\n\n", sim->params.h_corte);

    fprintf(f, "[RESULTADOS DE SIMULACION]\n");
    fprintf(f, "Velocidad desacople SRB (v1): %.2f m/s\n", sim->resultados.v1);
    fprintf(f, "Altura desacople SRB (h1):    %.2f m\n", sim->resultados.h1);
    fprintf(f, "Velocidad al corte (v2):      %.2f m/s\n", sim->resultados.v_corte);
    fprintf(f, "Tiempo en Fase 2 (t2):        %.2f s\n", sim->resultados.t2);
    fprintf(f, "Apogeo máximo (h_max):        %.2f m (%.2f km)\n",
            sim->resultados.h_max, sim->resultados.h_max / 1000.0);
    fprintf(f, "Aceleración máx. Fase 1:       %.2f g\n", sim->resultados.g_max_f1);
    fprintf(f, "Aceleración máx. Fase 2:       %.2f g\n\n", sim->resultados.g_max_f2);
    fprintf(f, "===============================================================\n");

    fclose(f);
    return 1;
}
