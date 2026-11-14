#ifndef FISICA_H
#define FISICA_H

typedef struct {
    double v0;          /* Velocidad inicial (m/s) */
    double h0;          /* Altura inicial (m) */
    double a1;          /* Aceleración Fase 1 SRB (m/s^2) */
    double t1;          /* Duración Fase 1 (s) */
    double a2;          /* Aceleración Fase 2 Motor Principal (m/s^2) */
    double h_corte;     /* Altitud de corte de motores (m) */
    double g;           /* Gravedad (m/s^2) */
} ParametrosFisicos;

typedef struct {
    double v1;          /* Velocidad al desacople SRB (m/s) */
    double h1;          /* Altura al desacople SRB (m) */
    double v_corte;     /* Velocidad al corte de motores (m/s) */
    double t2;          /* Tiempo transcurrido en Fase 2 (s) */
    double h_max;       /* Apogeo máximo balístico (m) */
    double g_max_f1;    /* Carga g en Fase 1 */
    double g_max_f2;    /* Carga g en Fase 2 */
} ResultadosFisicos;

void calcular_fase1(const ParametrosFisicos *p, ResultadosFisicos *r);
void calcular_fase2(const ParametrosFisicos *p, ResultadosFisicos *r);
void calcular_apogeo(const ParametrosFisicos *p, ResultadosFisicos *r);
void ejecutar_calculos_cinematicos(const ParametrosFisicos *p, ResultadosFisicos *r);

#endif
