#include <stdio.h>
#include <string.h>
#include "simulador.h"
#include "interfaz.h"
#include "validaciones.h"
#include "reportes.h"

int main(int argc, char *argv[]) {
    EstadoSimulador sim;
    inicializar_simulador(&sim);

    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
        ejecutar_simulador(&sim);
        guardar_reporte_mision(&sim, "reporte_mision_artemis.txt");
        printf("Prueba automatizada finalizada con éxito.\n");
        return 0;
    }

    int opcion = 0;
    do {
        mostrar_menu_principal();
        opcion = leer_entero_rango("Seleccione una opción: ", 0, 3);
        switch (opcion) {
            case 1:
                ejecutar_simulador(&sim);
                mostrar_resultados_simulacion(&sim);
                break;
            case 2:
                configurar_parametros_interactivo(&sim);
                break;
            case 3:
                if (!sim.simulacion_ejecutada) {
                    ejecutar_simulador(&sim);
                }
                guardar_reporte_mision(&sim, "reporte_mision_artemis.txt");
                printf("\n[OK] Reporte guardado en 'reporte_mision_artemis.txt'.\n");
                break;
            case 0:
                printf("\nSaliendo del simulador ARTEMIS. Hasta luego.\n");
                break;
        }
    } while (opcion != 0);

    return 0;
}
