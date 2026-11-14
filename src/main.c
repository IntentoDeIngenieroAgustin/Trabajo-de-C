#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulador.h"
#include "interfaz.h"
#include "validaciones.h"
#include "reportes.h"

static void guardar_reporte(const EstadoSimulador *sim) {
    if (guardar_reporte_mision(sim, ARCHIVO_REPORTE)) {
        printf("\n[OK] Reporte guardado en '%s'.\n", ARCHIVO_REPORTE);
    } else {
        printf("\n[ERROR] No se pudo crear '%s' (permisos o disco).\n", ARCHIVO_REPORTE);
    }
}

int main(int argc, char *argv[]) {
    /* static: el struct contiene arreglos grandes y no conviene ubicarlo en la pila */
    static EstadoSimulador sim;
    int opcion;
    int fallas;

    inicializar_simulador(&sim);
    srand((unsigned int)time(NULL));

    /* Modo de prueba automática: ./bin/artemis -t  (exit code 0 = todo OK) */
    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
        fallas = ejecutar_autoprueba(&sim);
        guardar_reporte(&sim);
        printf("Autoprueba finalizada: %d falla(s).\n", fallas);
        return fallas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    do {
        mostrar_menu_principal();
        opcion = leer_entero_rango("Seleccione una opcion: ", OPC_SALIR, OPC_REPORTE);
        switch (opcion) {
            case OPC_SIMULAR:
                ejecutar_simulador(&sim);
                mostrar_resultados_simulacion(&sim);
                break;
            case OPC_CONFIGURAR:
                configurar_parametros_interactivo(&sim);
                break;
            case OPC_NOMINALES:
                restaurar_parametros_nominales(&sim.params);
                sim.simulacion_ejecutada = 0;
                printf("[OK] Parametros nominales restaurados (a1=20, t1=120, a2=12.5, h=160 km).\n");
                break;
            case OPC_TELEMETRIA:
                simulacion_paso_a_paso(&sim);
                break;
            case OPC_MISION:
                jugar_modo_mision(&sim);
                break;
            case OPC_REPORTE:
                if (!sim.simulacion_ejecutada) {
                    ejecutar_simulador(&sim);
                }
                guardar_reporte(&sim);
                break;
            case OPC_SALIR:
                printf("\nSaliendo del simulador ARTEMIS. Hasta luego.\n");
                break;
            default:
                break;
        }
    } while (opcion != OPC_SALIR);

    return EXIT_SUCCESS;
}
