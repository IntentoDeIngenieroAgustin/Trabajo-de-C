#include "reportes.h"
#include <stdio.h>

static void escribir_resultados(FILE *f, const EstadoSimulador *sim) {
    const ResultadosFisicos *r = &sim->resultados;

    fprintf(f, "[RESULTADOS DE SIMULACION]\n");
    if (sim->falla_simulada) {
        fprintf(f, "Escenario con FALLA de motores a %.2f m\n", sim->h_falla);
    }
    if (r->corte_en_fase1) {
        fprintf(f, "Corte de motores antes del desacople SRB (sin Fase 2)\n");
    } else {
        fprintf(f, "Velocidad desacople SRB (v1):  %.2f m/s\n", r->v1);
        fprintf(f, "Altura desacople SRB (h1):     %.2f m\n", r->h1);
        fprintf(f, "Tiempo en Fase 2 (t2):         %.2f s\n", r->t2);
    }
    fprintf(f, "Velocidad al corte (v2):       %.2f m/s\n", r->v_corte);
    fprintf(f, "Instante del corte:            %.2f s\n", r->t_corte);
    fprintf(f, "Apogeo maximo (h_max):         %.2f m (%.2f km)\n", r->h_max, r->h_max / 1000.0);
    fprintf(f, "Instante del apogeo:           %.2f s\n", r->t_apogeo);
    fprintf(f, "Carga maxima Fase 1:           %.2f g\n", r->g_max_f1);
    if (!r->corte_en_fase1) {
        fprintf(f, "Carga maxima Fase 2:           %.2f g\n", r->g_max_f2);
    }
    fprintf(f, "\n");
}

static void escribir_telemetria(FILE *f, const EstadoSimulador *sim) {
    double v_max;
    double h_max;
    int conteo[4];
    int i;

    if (sim->n_telemetria == 0) {
        return;
    }
    analizar_telemetria(sim, &v_max, &h_max, conteo);
    fprintf(f, "[TELEMETRIA COMPLETA] %d muestras, dt = %.2f s\n", sim->n_telemetria,
            sim->dt_telemetria);
    fprintf(f, "Velocidad maxima: %.2f m/s | Altitud maxima: %.2f m\n", v_max, h_max);
    fprintf(f, "%10s %14s %12s %10s %5s\n", "t (s)", "h (m)", "v (m/s)", "a (m/s2)", "Fase");
    for (i = 0; i < sim->n_telemetria; i++) {
        fprintf(f, "%10.2f %14.2f %12.2f %10.2f %5d\n", sim->telemetria[i].t,
                sim->telemetria[i].h, sim->telemetria[i].v, sim->telemetria[i].a,
                (int)sim->telemetria[i].fase);
    }
    fprintf(f, "\n");
}

static void escribir_mision(FILE *f, const EstadoSimulador *sim) {
    const IntentoMision *it;
    int i;

    if (sim->n_intentos == 0) {
        return;
    }
    fprintf(f, "[MODO MISION] Objetivo de apogeo: %.0f km - %s\n", sim->objetivo_apogeo / 1000.0,
            sim->mision_cumplida ? "CUMPLIDA" : "FALLIDA");
    for (i = 0; i < sim->n_intentos; i++) {
        it = &sim->intentos[i];
        fprintf(f, "Turno %d: a1=%.2f t1=%.2f a2=%.2f h_corte=%.0f -> apogeo %.2f km, "
                   "%.2f g, error %.2f%%, %d puntos\n",
                i + 1, it->params.a1, it->params.t1, it->params.a2, it->params.h_corte,
                it->h_max / 1000.0, it->g_max, it->error_pct, it->puntaje);
    }
    fprintf(f, "\n");
}

int guardar_reporte_mision(const EstadoSimulador *sim, const char *nombre_archivo) {
    FILE *f = fopen(nombre_archivo, "w");
    if (f == NULL) {
        return 0;
    }

    fprintf(f, "===============================================================\n");
    fprintf(f, "       REPORTE TECNICO DE SIMULACION - PROYECTO ARTEMIS        \n");
    fprintf(f, "       Algoritmos y Estructuras de Datos - UTN FRRe            \n");
    fprintf(f, "===============================================================\n\n");
    fprintf(f, "[PARAMETROS DE ENTRADA]\n");
    fprintf(f, "Velocidad inicial (v0):        %.2f m/s\n", sim->params.v0);
    fprintf(f, "Aceleracion Fase 1 (a1):       %.2f m/s^2\n", sim->params.a1);
    fprintf(f, "Duracion Fase 1 (t1):          %.2f s\n", sim->params.t1);
    fprintf(f, "Aceleracion Fase 2 (a2):       %.2f m/s^2\n", sim->params.a2);
    fprintf(f, "Altitud de corte (h_corte):    %.2f m\n", sim->params.h_corte);
    fprintf(f, "Gravedad (g):                  %.2f m/s^2\n\n", sim->params.g);

    escribir_resultados(f, sim);
    escribir_mision(f, sim);
    escribir_telemetria(f, sim);
    fprintf(f, "===============================================================\n");

    fclose(f);
    return 1;
}
