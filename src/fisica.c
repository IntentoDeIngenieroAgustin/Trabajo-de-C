#include "fisica.h"
#include <math.h>

void calcular_fase1(const ParametrosFisicos *p, ResultadosFisicos *r) {
    r->v1 = p->v0 + (p->a1 * p->t1);
    r->h1 = p->h0 + (p->v0 * p->t1) + (0.5 * p->a1 * p->t1 * p->t1);
    r->g_max_f1 = p->a1 / p->g;
}

void calcular_fase2(const ParametrosFisicos *p, ResultadosFisicos *r) {
    double delta_h = p->h_corte - r->h1;
    if (delta_h > 0.0) {
        r->v_corte = sqrt((r->v1 * r->v1) + (2.0 * p->a2 * delta_h));
        r->t2 = (r->v_corte - r->v1) / p->a2;
    } else {
        r->v_corte = r->v1;
        r->t2 = 0.0;
    }
    r->g_max_f2 = p->a2 / p->g;
}

void calcular_apogeo(const ParametrosFisicos *p, ResultadosFisicos *r) {
    r->h_max = p->h_corte + ((r->v_corte * r->v_corte) / (2.0 * p->g));
}

void ejecutar_calculos_cinematicos(const ParametrosFisicos *p, ResultadosFisicos *r) {
    calcular_fase1(p, r);
    calcular_fase2(p, r);
    calcular_apogeo(p, r);
}
