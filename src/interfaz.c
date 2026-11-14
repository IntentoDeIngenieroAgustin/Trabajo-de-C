#include "interfaz.h"
#include "validaciones.h"
#include <stdio.h>

void mostrar_menu_principal(void) {
    printf("\n==================================================\n");
    printf("   SIMULADOR DE MISIONES ARTEMIS - UTN FRRe       \n");
    printf("==================================================\n");
    printf("1. Ejecutar Simulación Nominal (SLS / Orion)\n");
    printf("2. Configurar Parámetros de Vuelo\n");
    printf("3. Guardar Reporte Técnico de Misión\n");
    printf("0. Salir\n");
    printf("==================================================\n");
}

void mostrar_resultados_simulacion(const EstadoSimulador *sim) {
    if (!sim->simulacion_ejecutada) {
        printf("\n[ADVERTENCIA] Debe ejecutar la simulación primero.\n");
        return;
    }
    printf("\n--- RESULTADOS CINEMATICOS DE LA MISION ---\n");
    printf("Fase 1 (Desacople SRB): v1 = %.2f m/s, h1 = %.2f m (%.2f km)\n",
           sim->resultados.v1, sim->resultados.h1, sim->resultados.h1 / 1000.0);
    printf("Fase 2 (Corte Motores): v_corte = %.2f m/s, t2 = %.2f s\n",
           sim->resultados.v_corte, sim->resultados.t2);
    printf("Fase 3 (Apogeo Balístico Max.): h_max = %.2f m (%.2f km)\n",
           sim->resultados.h_max, sim->resultados.h_max / 1000.0);
    printf("Cargas de Gravedad: Fase 1 = %.2f g | Fase 2 = %.2f g\n",
           sim->resultados.g_max_f1, sim->resultados.g_max_f2);
}

void configurar_parametros_interactivo(EstadoSimulador *sim) {
    printf("\n--- CONFIGURACION DE PARAMETROS DE MISION ---\n");
    sim->params.a1 = leer_double_positivo("Aceleración Fase 1 (m/s^2): ");
    sim->params.t1 = leer_double_positivo("Duración Fase 1 (s): ");
    sim->params.a2 = leer_double_positivo("Aceleración Fase 2 (m/s^2): ");
    sim->params.h_corte = leer_double_positivo("Altitud de corte (m): ");
    sim->simulacion_ejecutada = 0;
    printf("[OK] Parámetros actualizados correctamente.\n");
}
