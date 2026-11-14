#ifndef REPORTES_H
#define REPORTES_H

#include "simulador.h"

#define ARCHIVO_REPORTE "reporte_mision_artemis.txt"

/* Devuelve 1 si el reporte se guardó y 0 si no se pudo abrir el archivo. */
int guardar_reporte_mision(const EstadoSimulador *sim, const char *nombre_archivo);

#endif
