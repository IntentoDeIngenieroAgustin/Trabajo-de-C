#ifndef INTERFAZ_H
#define INTERFAZ_H

#include "simulador.h"

/* Opciones del menú principal */
#define OPC_SALIR        0
#define OPC_SIMULAR      1
#define OPC_CONFIGURAR   2
#define OPC_NOMINALES    3
#define OPC_TELEMETRIA   4
#define OPC_MISION       5
#define OPC_REPORTE      6

void mostrar_menu_principal(void);
void mostrar_resultados_simulacion(const EstadoSimulador *sim);
void configurar_parametros_interactivo(EstadoSimulador *sim);
void simulacion_paso_a_paso(EstadoSimulador *sim);
void jugar_modo_mision(EstadoSimulador *sim);

#endif
