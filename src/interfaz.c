#include "interfaz.h"
#include "validaciones.h"
#include <stdio.h>

/* Los textos de consola se escriben sin tildes para que se vean bien en
 * cualquier terminal (la consola de Windows no usa UTF-8 por defecto). */

#define FILAS_MAX_TABLA 30   /* Filas de telemetría que se imprimen como máximo */

static const char *nombre_fase(FaseVuelo f) {
    switch (f) {
        case FASE_SRB:       return "SRB+Nucleo";
        case FASE_NUCLEO:    return "Nucleo";
        case FASE_BALISTICA: return "Balistica";
    }
    return "?";
}

static void imprimir_separador(void) {
    printf("==================================================\n");
}

void mostrar_menu_principal(void) {
    printf("\n");
    imprimir_separador();
    printf("   SIMULADOR DE MISIONES ARTEMIS - SLS / ORION\n");
    printf("   UTN FRRe - Algoritmos y Estructuras de Datos\n");
    imprimir_separador();
    printf("%d. Ejecutar simulacion (parametros actuales)\n", OPC_SIMULAR);
    printf("%d. Configurar parametros de vuelo\n", OPC_CONFIGURAR);
    printf("%d. Restaurar parametros nominales (consigna)\n", OPC_NOMINALES);
    printf("%d. Simulacion paso a paso (telemetria / falla)\n", OPC_TELEMETRIA);
    printf("%d. MODO MISION (juego por turnos)\n", OPC_MISION);
    printf("%d. Guardar reporte tecnico\n", OPC_REPORTE);
    printf("%d. Salir\n", OPC_SALIR);
    imprimir_separador();
}

void mostrar_resultados_simulacion(const EstadoSimulador *sim) {
    const ResultadosFisicos *r = &sim->resultados;

    if (!sim->simulacion_ejecutada) {
        printf("\n[ADVERTENCIA] Debe ejecutar la simulacion primero.\n");
        return;
    }
    printf("\n--- RESULTADOS CINEMATICOS DE LA MISION ---\n");
    if (sim->falla_simulada) {
        printf("[!] Escenario con FALLA de motores a %.2f m de altitud.\n", sim->h_falla);
    }
    if (r->corte_en_fase1) {
        printf("[!] Los motores se apagan ANTES del desacople SRB: no hay Fase 2.\n");
        printf("Corte de motores: v = %.2f m/s a los %.2f s\n", r->v_corte, r->t_corte);
    } else {
        printf("a) Desacople SRB:  v1 = %.2f m/s | h1 = %.2f m (%.2f km)\n",
               r->v1, r->h1, r->h1 / 1000.0);
        printf("b) Corte motores:  v2 = %.2f m/s (%.2f km/h) | t = %.2f s (Fase 2: %.2f s)\n",
               r->v_corte, r->v_corte * 3.6, r->t_corte, r->t2);
    }
    printf("c) Apogeo balistico: h_max = %.2f m (%.2f km) a los %.2f s\n",
           r->h_max, r->h_max / 1000.0, r->t_apogeo);
    printf("d) Carga maxima: Fase 1 = %.2f g", r->g_max_f1);
    if (!r->corte_en_fase1) {
        printf(" | Fase 2 = %.2f g", r->g_max_f2);
    }
    printf("\n");
    if (carga_g_maxima(r) > G_LIMITE_TRIPULACION) {
        printf("[ALERTA] La carga supera el limite de %.1f g para la tripulacion.\n",
               G_LIMITE_TRIPULACION);
    }
}

void configurar_parametros_interactivo(EstadoSimulador *sim) {
    printf("\n--- CONFIGURACION DE PARAMETROS DE MISION ---\n");
    printf("(Se acepta punto o coma decimal. Rangos entre corchetes.)\n");
    sim->params.a1 = leer_double_rango("Aceleracion Fase 1 SRB [0.1 - 100] m/s^2: ", 0.1, 100.0);
    sim->params.t1 = leer_double_rango("Duracion Fase 1 [1 - 600] s: ", 1.0, 600.0);
    sim->params.a2 = leer_double_rango("Aceleracion Fase 2 [0.1 - 100] m/s^2: ", 0.1, 100.0);
    sim->params.h_corte = leer_double_rango("Altitud de corte [1000 - 1000000] m: ",
                                            1000.0, 1000000.0);
    sim->simulacion_ejecutada = 0;
    printf("[OK] Parametros actualizados correctamente.\n");
}

void simulacion_paso_a_paso(EstadoSimulador *sim) {
    const RegistroTelemetria *reg;
    double dt;
    double h_falla = 0.0;
    double v_max;
    double h_max;
    int conteo[4];
    int paso;
    int i;

    printf("\n--- SIMULACION PASO A PASO ---\n");
    dt = leer_double_rango("Intervalo de muestreo dt [0.5 - 60] s: ", 0.5, 60.0);
    if (leer_caracter_sn("Simular FALLA de motores durante el ascenso? (S/N): ") == 'S') {
        h_falla = leer_double_rango("Altitud de la falla (m): ", 1.0, sim->params.h_corte - 1.0);
    }
    if (generar_telemetria(sim, dt, h_falla)) {
        printf("[AVISO] dt ampliado a %.2f s para no superar %d registros.\n",
               sim->dt_telemetria, MAX_TELEMETRIA);
    }

    /* Se imprime como máximo FILAS_MAX_TABLA filas, más los cambios de fase
     * y la última muestra (apogeo). El arreglo conserva todas las muestras. */
    paso = sim->n_telemetria / FILAS_MAX_TABLA + 1;
    printf("\n%10s %14s %12s %10s  %s\n", "t (s)", "h (m)", "v (m/s)", "a (m/s2)", "Fase");
    for (i = 0; i < sim->n_telemetria; i++) {
        reg = &sim->telemetria[i];
        if (i % paso == 0 || i == sim->n_telemetria - 1 ||
            (i > 0 && reg->fase != sim->telemetria[i - 1].fase)) {
            printf("%10.2f %14.2f %12.2f %10.2f  %s\n",
                   reg->t, reg->h, reg->v, reg->a, nombre_fase(reg->fase));
        }
    }

    analizar_telemetria(sim, &v_max, &h_max, conteo);
    printf("\nMuestras registradas: %d (dt = %.2f s)\n", sim->n_telemetria, sim->dt_telemetria);
    printf("Muestras por fase: SRB = %d | Nucleo = %d | Balistica = %d\n",
           conteo[FASE_SRB], conteo[FASE_NUCLEO], conteo[FASE_BALISTICA]);
    printf("Velocidad maxima registrada: %.2f m/s | Altitud maxima: %.2f m\n", v_max, h_max);
    mostrar_resultados_simulacion(sim);
}

static void mostrar_resumen_mision(const EstadoSimulador *sim) {
    const IntentoMision *it;
    int i;
    int mejor = mejor_intento(sim);

    printf("\n--- RESUMEN DE LA MISION (objetivo %.0f km) ---\n", sim->objetivo_apogeo / 1000.0);
    printf("%-6s %8s %8s %8s %10s %12s %8s %8s\n",
           "Turno", "a1", "t1", "a2", "h_corte", "Apogeo(km)", "Error%", "Puntos");
    for (i = 0; i < sim->n_intentos; i++) {
        it = &sim->intentos[i];
        printf("%-6d %8.2f %8.2f %8.2f %10.0f %12.2f %8.2f %8d%s\n",
               i + 1, it->params.a1, it->params.t1, it->params.a2, it->params.h_corte,
               it->h_max / 1000.0, it->error_pct, it->puntaje, i == mejor ? "  <- mejor" : "");
    }
    if (sim->mision_cumplida) {
        printf("\n*** MISION CUMPLIDA: Orion alcanzo el apogeo objetivo. ***\n");
    } else {
        printf("\n*** MISION FALLIDA: no se alcanzo el objetivo en los turnos disponibles. ***\n");
    }
}

void jugar_modo_mision(EstadoSimulador *sim) {
    ParametrosFisicos p;
    const IntentoMision *it;

    iniciar_mision(sim);
    printf("\n");
    imprimir_separador();
    printf("   MODO MISION - DIRECTOR DE VUELO\n");
    imprimir_separador();
    printf("Objetivo: que Orion alcance un apogeo de %.0f km (+/- %.0f%%).\n",
           sim->objetivo_apogeo / 1000.0, TOLERANCIA_MISION_PCT);
    printf("Restriccion: la tripulacion no puede superar %.1f g (%.2f m/s^2).\n",
           G_LIMITE_TRIPULACION, G_LIMITE_TRIPULACION * G_TIERRA);
    printf("Dispone de %d turnos. Puntaje = 100 - 2 x error%% (0 si se supera el limite g).\n",
           MAX_INTENTOS);

    while (sim->n_intentos < MAX_INTENTOS && !sim->mision_cumplida) {
        printf("\n--- TURNO %d de %d ---\n", sim->n_intentos + 1, MAX_INTENTOS);
        restaurar_parametros_nominales(&p);
        p.a1 = leer_double_rango("Aceleracion Fase 1 [1 - 40] m/s^2: ", 1.0, 40.0);
        p.t1 = leer_double_rango("Duracion Fase 1 [10 - 300] s: ", 10.0, 300.0);
        p.a2 = leer_double_rango("Aceleracion Fase 2 [1 - 40] m/s^2: ", 1.0, 40.0);
        p.h_corte = leer_double_rango("Altitud de corte [10000 - 400000] m: ", 10000.0, 400000.0);

        it = registrar_intento(sim, &p);
        if (it == NULL) {
            break;
        }
        printf("Resultado: apogeo = %.2f km | carga max = %.2f g | error = %.2f%% | puntos = %d\n",
               it->h_max / 1000.0, it->g_max, it->error_pct, it->puntaje);
        if (it->g_max > G_LIMITE_TRIPULACION) {
            printf("[ABORTO] La carga de %.2f g pone en riesgo a la tripulacion.\n", it->g_max);
        } else if (!sim->mision_cumplida) {
            printf("[PISTA] El apogeo quedo %s del objetivo.\n",
                   it->h_max > sim->objetivo_apogeo ? "POR ENCIMA" : "POR DEBAJO");
        }
        if (!sim->mision_cumplida && sim->n_intentos < MAX_INTENTOS &&
            leer_caracter_sn("Desea jugar el siguiente turno? (S/N): ") == 'N') {
            break;
        }
    }
    mostrar_resumen_mision(sim);
}
