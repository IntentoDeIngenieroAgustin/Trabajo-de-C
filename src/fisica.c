#include "fisica.h"
#include <math.h>

void calcular_fase1(const ParametrosFisicos *p, ResultadosFisicos *r) {
    r->v1 = p->v0 + (p->a1 * p->t1);
    r->h1 = p->h0 + (p->v0 * p->t1) + (0.5 * p->a1 * p->t1 * p->t1);
    r->g_max_f1 = p->a1 / p->g;
}

void calcular_fase2(const ParametrosFisicos *p, ResultadosFisicos *r) {
    double delta_h;

    if (p->h_corte <= r->h1) {
        /* La altitud de corte se alcanza DURANTE la Fase 1: los motores se
         * apagan antes del desacople y la Fase 2 no ocurre.
         * v^2 = v0^2 + 2*a1*(h_corte - h0) */
        r->corte_en_fase1 = 1;
        r->v_corte = sqrt((p->v0 * p->v0) + (2.0 * p->a1 * (p->h_corte - p->h0)));
        r->t_corte = (r->v_corte - p->v0) / p->a1;
        r->t2 = 0.0;
        r->g_max_f2 = 0.0;
    } else {
        r->corte_en_fase1 = 0;
        delta_h = p->h_corte - r->h1;
        r->v_corte = sqrt((r->v1 * r->v1) + (2.0 * p->a2 * delta_h));
        r->t2 = (r->v_corte - r->v1) / p->a2;
        r->t_corte = p->t1 + r->t2;
        r->g_max_f2 = p->a2 / p->g;
    }
}

void calcular_apogeo(const ParametrosFisicos *p, ResultadosFisicos *r) {
    r->h_max = p->h_corte + ((r->v_corte * r->v_corte) / (2.0 * p->g));
    r->t_apogeo = r->t_corte + (r->v_corte / p->g);
}

void ejecutar_calculos_cinematicos(const ParametrosFisicos *p, ResultadosFisicos *r) {
    calcular_fase1(p, r);
    calcular_fase2(p, r);
    calcular_apogeo(p, r);
}

double carga_g_maxima(const ResultadosFisicos *r) {
    if (r->corte_en_fase1 || r->g_max_f1 >= r->g_max_f2) {
        return r->g_max_f1;
    }
    return r->g_max_f2;
}

/* Calcula altura, velocidad y aceleración en el instante t (0 <= t <= t_apogeo)
 * con las ecuaciones exactas del MRUV de cada tramo. Devuelve la fase. */
FaseVuelo estado_en_tiempo(const ParametrosFisicos *p, const ResultadosFisicos *r,
                           double t, double *h, double *v, double *a) {
    double dt;

    if (t <= r->t_corte && (r->corte_en_fase1 || t <= p->t1)) {
        *a = p->a1;
        *v = p->v0 + p->a1 * t;
        *h = p->h0 + p->v0 * t + 0.5 * p->a1 * t * t;
        return FASE_SRB;
    }
    if (t <= r->t_corte) {
        dt = t - p->t1;
        *a = p->a2;
        *v = r->v1 + p->a2 * dt;
        *h = r->h1 + r->v1 * dt + 0.5 * p->a2 * dt * dt;
        return FASE_NUCLEO;
    }
    dt = t - r->t_corte;
    *a = -p->g;
    *v = r->v_corte - p->g * dt;
    *h = p->h_corte + r->v_corte * dt - 0.5 * p->g * dt * dt;
    return FASE_BALISTICA;
}
