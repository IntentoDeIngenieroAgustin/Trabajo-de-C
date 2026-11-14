#ifndef FISICA_H
#define FISICA_H

#define G_TIERRA 9.80   /* Aceleración de la gravedad (m/s^2) */

typedef enum {
    FASE_SRB = 1,        /* Fase 1: núcleo + cohetes SRB */
    FASE_NUCLEO = 2,     /* Fase 2: solo motor del núcleo */
    FASE_BALISTICA = 3   /* Fase 3: motores apagados, ascenso inercial */
} FaseVuelo;

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
    double t_corte;     /* Instante del corte de motores desde el despegue (s) */
    double h_max;       /* Apogeo máximo balístico (m) */
    double t_apogeo;    /* Instante en que se alcanza el apogeo (s) */
    double g_max_f1;    /* Carga g en Fase 1 */
    double g_max_f2;    /* Carga g en Fase 2 (0 si no hubo Fase 2) */
    int corte_en_fase1; /* 1 si los motores se apagan antes del desacople */
} ResultadosFisicos;

void calcular_fase1(const ParametrosFisicos *p, ResultadosFisicos *r);
void calcular_fase2(const ParametrosFisicos *p, ResultadosFisicos *r);
void calcular_apogeo(const ParametrosFisicos *p, ResultadosFisicos *r);
void ejecutar_calculos_cinematicos(const ParametrosFisicos *p, ResultadosFisicos *r);
double carga_g_maxima(const ResultadosFisicos *r);
FaseVuelo estado_en_tiempo(const ParametrosFisicos *p, const ResultadosFisicos *r,
                           double t, double *h, double *v, double *a);

#endif
