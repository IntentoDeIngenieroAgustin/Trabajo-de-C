#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "fisica.h"

#define MAX_TELEMETRIA        1000   /* Capacidad del arreglo de telemetría */
#define MAX_INTENTOS          3      /* Turnos disponibles en el Modo Misión */
#define CANT_OBJETIVOS        4      /* Cantidad de apogeos objetivo posibles */
#define TOLERANCIA_MISION_PCT 5.0    /* Error máximo admitido en el apogeo (%) */
#define G_LIMITE_TRIPULACION  3.0    /* Carga máxima soportable (g) */

/* Una muestra de telemetría: estado de la nave en un instante. */
typedef struct {
    double t;           /* Tiempo desde el despegue (s) */
    double h;           /* Altitud (m) */
    double v;           /* Velocidad vertical (m/s) */
    double a;           /* Aceleración (m/s^2) */
    FaseVuelo fase;
} RegistroTelemetria;

/* Un turno jugado en el Modo Misión. */
typedef struct {
    ParametrosFisicos params;
    double h_max;
    double g_max;
    double error_pct;
    int puntaje;
} IntentoMision;

typedef struct {
    ParametrosFisicos params;
    ResultadosFisicos resultados;
    int simulacion_ejecutada;

    /* Telemetría paso a paso (arreglo de structs) */
    RegistroTelemetria telemetria[MAX_TELEMETRIA];
    int n_telemetria;
    double dt_telemetria;
    int falla_simulada;
    double h_falla;

    /* Modo Misión (juego por turnos) */
    IntentoMision intentos[MAX_INTENTOS];
    int n_intentos;
    double objetivo_apogeo;
    int mision_cumplida;
} EstadoSimulador;

void inicializar_simulador(EstadoSimulador *sim);
void restaurar_parametros_nominales(ParametrosFisicos *p);
void ejecutar_simulador(EstadoSimulador *sim);

int  generar_telemetria(EstadoSimulador *sim, double dt, double h_falla);
void analizar_telemetria(const EstadoSimulador *sim, double *v_max,
                         double *h_max, int conteo_fases[4]);

void iniciar_mision(EstadoSimulador *sim);
const IntentoMision *registrar_intento(EstadoSimulador *sim, const ParametrosFisicos *p);
int  mejor_intento(const EstadoSimulador *sim);

int  ejecutar_autoprueba(EstadoSimulador *sim);

#endif
