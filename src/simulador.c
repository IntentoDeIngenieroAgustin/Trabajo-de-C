#include "simulador.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Apogeos objetivo posibles para el Modo Misión (m). */
static const double OBJETIVOS_APOGEO[CANT_OBJETIVOS] = {
    300000.0, 400000.0, 550000.0, 700000.0
};

void restaurar_parametros_nominales(ParametrosFisicos *p) {
    /* Valores de la Situación-Problema 1 de la consigna */
    p->v0 = 0.0;
    p->h0 = 0.0;
    p->a1 = 20.0;
    p->t1 = 120.0;
    p->a2 = 12.5;
    p->h_corte = 160000.0;
    p->g = G_TIERRA;
}

void inicializar_simulador(EstadoSimulador *sim) {
    restaurar_parametros_nominales(&sim->params);
    sim->simulacion_ejecutada = 0;
    sim->n_telemetria = 0;
    sim->dt_telemetria = 0.0;
    sim->falla_simulada = 0;
    sim->h_falla = 0.0;
    sim->n_intentos = 0;
    sim->objetivo_apogeo = 0.0;
    sim->mision_cumplida = 0;
}

void ejecutar_simulador(EstadoSimulador *sim) {
    ejecutar_calculos_cinematicos(&sim->params, &sim->resultados);
    sim->simulacion_ejecutada = 1;
    sim->falla_simulada = 0;
}

/* Muestrea el vuelo cada dt segundos desde el despegue hasta el apogeo y lo
 * guarda en el arreglo sim->telemetria. Si h_falla > 0, los motores fallan al
 * alcanzar esa altitud (el resto del vuelo es balístico).
 * Devuelve 1 si dt tuvo que ampliarse para no desbordar el arreglo, 0 si no. */
int generar_telemetria(EstadoSimulador *sim, double dt, double h_falla) {
    ParametrosFisicos p = sim->params;   /* copia: no altera la configuración */
    RegistroTelemetria *reg;
    double eventos[2];
    double t;
    double t_anterior = -1.0;
    int n_eventos;
    int e;
    int i;
    int ajustado = 0;

    sim->falla_simulada = 0;
    if (h_falla > 0.0 && h_falla < p.h_corte) {
        p.h_corte = h_falla;
        sim->falla_simulada = 1;
        sim->h_falla = h_falla;
    }

    ejecutar_calculos_cinematicos(&p, &sim->resultados);
    sim->simulacion_ejecutada = 1;

    if ((int)ceil(sim->resultados.t_apogeo / dt) + 3 > MAX_TELEMETRIA) {  /* +1 apogeo, +2 eventos */
        dt = sim->resultados.t_apogeo / (MAX_TELEMETRIA - 3);
        ajustado = 1;
    }
    sim->dt_telemetria = dt;

    /* Instantes de cambio de fase: se registran aunque no coincidan con un
     * múltiplo de dt, para que la telemetría capture los valores exactos. */
    n_eventos = 0;
    if (!sim->resultados.corte_en_fase1) {
        eventos[n_eventos++] = p.t1;
    }
    eventos[n_eventos++] = sim->resultados.t_corte;

    sim->n_telemetria = 0;
    for (i = 0; sim->n_telemetria < MAX_TELEMETRIA; i++) {
        t = i * dt;
        if (t > sim->resultados.t_apogeo) {
            t = sim->resultados.t_apogeo;
        }
        for (e = 0; e < n_eventos && sim->n_telemetria < MAX_TELEMETRIA - 1; e++) {
            if (eventos[e] > t_anterior && eventos[e] < t) {
                reg = &sim->telemetria[sim->n_telemetria];
                reg->t = eventos[e];
                reg->fase = estado_en_tiempo(&p, &sim->resultados, eventos[e],
                                             &reg->h, &reg->v, &reg->a);
                sim->n_telemetria++;
            }
        }
        t_anterior = t;
        reg = &sim->telemetria[sim->n_telemetria];
        reg->t = t;
        reg->fase = estado_en_tiempo(&p, &sim->resultados, t, &reg->h, &reg->v, &reg->a);
        sim->n_telemetria++;
        if (t >= sim->resultados.t_apogeo) {
            break;
        }
    }
    return ajustado;
}

/* Recorre el arreglo de telemetría y obtiene máximos y cantidad de muestras
 * por fase (conteo_fases[1..3]; el índice 0 no se usa). */
void analizar_telemetria(const EstadoSimulador *sim, double *v_max,
                         double *h_max, int conteo_fases[4]) {
    int i;

    *v_max = 0.0;
    *h_max = 0.0;
    for (i = 0; i < 4; i++) {
        conteo_fases[i] = 0;
    }
    for (i = 0; i < sim->n_telemetria; i++) {
        if (sim->telemetria[i].v > *v_max) {
            *v_max = sim->telemetria[i].v;
        }
        if (sim->telemetria[i].h > *h_max) {
            *h_max = sim->telemetria[i].h;
        }
        conteo_fases[sim->telemetria[i].fase]++;
    }
}

void iniciar_mision(EstadoSimulador *sim) {
    sim->objetivo_apogeo = OBJETIVOS_APOGEO[rand() % CANT_OBJETIVOS];
    sim->n_intentos = 0;
    sim->mision_cumplida = 0;
}

/* Evalúa un turno del jugador y lo guarda en el arreglo de intentos.
 * Devuelve un puntero al intento registrado, o NULL si no quedan turnos. */
const IntentoMision *registrar_intento(EstadoSimulador *sim, const ParametrosFisicos *p) {
    ResultadosFisicos r;
    IntentoMision *it;
    int puntaje;

    if (sim->n_intentos >= MAX_INTENTOS) {
        return NULL;
    }
    ejecutar_calculos_cinematicos(p, &r);

    it = &sim->intentos[sim->n_intentos];
    it->params = *p;
    it->h_max = r.h_max;
    it->g_max = carga_g_maxima(&r);
    it->error_pct = fabs(r.h_max - sim->objetivo_apogeo) / sim->objetivo_apogeo * 100.0;

    if (it->g_max > G_LIMITE_TRIPULACION) {
        puntaje = 0;                 /* misión abortada: tripulación en riesgo */
    } else {
        puntaje = 100 - (int)(it->error_pct * 2.0);
        if (puntaje < 0) {
            puntaje = 0;
        }
    }
    it->puntaje = puntaje;

    if (it->g_max <= G_LIMITE_TRIPULACION && it->error_pct <= TOLERANCIA_MISION_PCT) {
        sim->mision_cumplida = 1;
    }
    sim->n_intentos++;
    return it;
}

/* Índice del intento con mayor puntaje, o -1 si no hay intentos. */
int mejor_intento(const EstadoSimulador *sim) {
    int i;
    int mejor = -1;

    for (i = 0; i < sim->n_intentos; i++) {
        if (mejor == -1 || sim->intentos[i].puntaje > sim->intentos[mejor].puntaje) {
            mejor = i;
        }
    }
    return mejor;
}

static int verificar(const char *nombre, double obtenido, double esperado, double tol) {
    int ok = fabs(obtenido - esperado) <= tol;
    printf("  [%s] %-34s obtenido = %12.2f | esperado = %12.2f\n",
           ok ? "OK  " : "FALLA", nombre, obtenido, esperado);
    return ok ? 0 : 1;
}

/* Prueba automática (modo -t). Devuelve la cantidad de verificaciones fallidas. */
int ejecutar_autoprueba(EstadoSimulador *sim) {
    int fallas = 0;
    const RegistroTelemetria *ultimo;

    printf("Caso 1: escenario nominal de la consigna\n");
    restaurar_parametros_nominales(&sim->params);
    ejecutar_simulador(sim);
    fallas += verificar("v1 al desacople SRB (m/s)", sim->resultados.v1, 2400.0, 0.01);
    fallas += verificar("h1 al desacople SRB (m)", sim->resultados.h1, 144000.0, 0.01);
    fallas += verificar("v al corte de motores (m/s)", sim->resultados.v_corte, 2481.93, 0.01);
    fallas += verificar("Apogeo h_max (m)", sim->resultados.h_max, 474285.71, 0.01);
    fallas += verificar("Carga g Fase 1", sim->resultados.g_max_f1, 2.04, 0.01);

    printf("Caso 2: telemetria paso a paso (dt = 1 s)\n");
    generar_telemetria(sim, 1.0, 0.0);
    ultimo = &sim->telemetria[sim->n_telemetria - 1];
    fallas += verificar("Ultima muestra: altitud (m)", ultimo->h, 474285.71, 0.01);
    fallas += verificar("Ultima muestra: velocidad (m/s)", ultimo->v, 0.0, 0.01);

    printf("Caso 3: corte de motores antes del desacople\n");
    sim->params.a1 = 100.0;
    sim->params.t1 = 10.0;
    sim->params.h_corte = 1000.0;
    ejecutar_simulador(sim);
    fallas += verificar("Corte en Fase 1 (1 = si)", sim->resultados.corte_en_fase1, 1.0, 0.0);
    fallas += verificar("v al corte (m/s)", sim->resultados.v_corte, 447.21, 0.01);
    fallas += verificar("Apogeo h_max (m)", sim->resultados.h_max, 11204.08, 0.01);

    printf("Caso 4: falla de motores a 150 km (escenario nominal)\n");
    restaurar_parametros_nominales(&sim->params);
    generar_telemetria(sim, 1.0, 150000.0);
    fallas += verificar("Falla registrada (1 = si)", sim->falla_simulada, 1.0, 0.0);
    fallas += verificar("Apogeo con falla (m)", sim->resultados.h_max, 451530.61, 0.01);

    /* Se deja el estado en el escenario nominal para el reporte. */
    restaurar_parametros_nominales(&sim->params);
    ejecutar_simulador(sim);
    generar_telemetria(sim, 1.0, 0.0);
    return fallas;
}
