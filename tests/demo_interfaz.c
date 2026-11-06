/**
 * @file demo_interfaz.c
 * @brief Programa de demostracion independiente para el modulo de interfaz (ARTEMIS CLI).
 *
 * Proyecto: Simulador de Misiones Artemis - Escenario 1: Ascenso y Propulsion con el SLS
 * Asignatura: Algoritmos y Estructuras de Datos (Ingenieria en Sistemas de Informacion - UTN)
 * Estandar: ISO/IEC 9899:2011 (C11)
 *
 * ============================================================================
 * NOTA PARA LOS COMPANEROS DEL EQUIPO:
 * ============================================================================
 * 1. Este archivo contiene una funcion `main()` EXCLUSIVAMENTE para probar y
 *    visualizar las pantallas de `src/interfaz.c` mientras el resto del equipo
 *    desarrolla `simulador.c`, `fisica.c`, `validaciones.c` y `reportes.c`.
 *
 * 2. Cuando se compile el proyecto final (con `src/main.c`), este archivo NO
 *    debe incluirse en el comando de compilacion para que no haya dos `main()`.
 *
 * 3. Tambien sirve como ejemplo practico de como llenar las estructuras
 *    (`UiTelemetria`, `UiAnalisisCientifico`, `UiReporteFinal`, etc.) y pasarselas
 *    a las funciones de `interfaz.h`.
 */

#if defined(__has_include)
#  if __has_include("interfaz.h")
#    include "interfaz.h"
#  elif __has_include("../include/interfaz.h")
#    include "../include/interfaz.h"
#  elif __has_include("../incluide/interfaz.h")
#    include "../incluide/interfaz.h"
#  else
#    include "interfaz.h"
#  endif
#else
#  include "interfaz.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* ==========================================================================
 * FUNCIONES INTERNAS DE RECORRIDO VISUAL (SOLO PARA LA DEMO)
 * ========================================================================== */

/**
 * @brief Permite recorrer paso a paso las 5 pantallas involucradas en una mision:
 *        1) Preparacion previa al lanzamiento
 *        2) Cuenta regresiva
 *        3) Telemetria inicial en espera (sin datos)
 *        4) Telemetria en vivo con eventos de muestra
 *        5) Reporte final de mision
 */
static void demo_recorrerSecuenciaLanzamiento(void) {
    int etapaActual = 1;
    int varianteEstadoChecklist = 0;
    int indiceCountdown = 0;
    int indiceSnapshotTelemetria = 0;

    /* Tiempos y mensajes de ejemplo para mostrar como se ve la cuenta regresiva */
    static const double tiemposCountdown[4] = { 10.0, 5.0, 3.0, 0.0 };
    static const char * const mensajesCountdown[4] = {
        "Iniciando secuencia automatica de lanzamiento...",
        "Secuencia de encendido de motores principales (4 motores RS-25)...",
        "Empuje nominal verificado - Preparando liberacion de anclajes...",
        "DESPEGUE CONFIRMADO: Ascenso inicial del vehiculo Artemis SLS"
    };

    /* Eventos ilustrativos para probar como se dibuja la bitacora de eventos */
    static const UiEventoMision eventosDemo[6] = {
        {   0.0, UI_EVENTO_INICIO_LANZAMIENTO, UI_NIVEL_INFO,   "Inicio de secuencia de lanzamiento" },
        {   1.0, UI_EVENTO_DESPEGUE,           UI_NIVEL_EVENTO, "Despegue confirmado desde plataforma LC-39B" },
        {  60.0, UI_EVENTO_ASCENSO_NOMINAL,    UI_NIVEL_OK,     "Ascenso nominal Fase 1 - Empuje combinado SLS" },
        { 120.0, UI_EVENTO_SEPARACION_SRB,     UI_NIVEL_EVENTO, "Separacion de propulsores SRB completada" },
        { 121.0, UI_EVENTO_ACTIVACION_FASE_2,  UI_NIVEL_INFO,   "Fase 2 de propulsion principal activa (Etapa Central)" },
        { 245.0, UI_EVENTO_CORTE_MOTORES,      UI_NIVEL_OK,     "Altitud objetivo 160.000 m alcanzada - Corte de motores" }
    };

    /* Cuatro instantaneas de muestra (con `esDemostracion = true` para avisar en pantalla) */
    static const UiTelemetria snapshotsDemo[4] = {
        {
            true, 60.0, 36000.0, 1200.0, 20.0, 160000.0, 22.5,
            UI_FASE_1_ASCENSO_SRB, UI_SRB_ACTIVE_BURN, UI_CORE_ACTIVE_THRUST, true
        },
        {
            true, 120.0, 144000.0, 2400.0, 20.0, 160000.0, 90.0,
            UI_FASE_SEPARACION_SRB, UI_SRB_SEPARATED, UI_CORE_ACTIVE_THRUST, true
        },
        {
            true, 121.0, 146400.0, 2412.5, 12.5, 160000.0, 91.5,
            UI_FASE_2_PROPULSION_CORE, UI_SRB_SEPARATED, UI_CORE_ACTIVE_THRUST, true
        },
        {
            true, 126.6, 160000.0, 2482.5, 0.0, 160000.0, 100.0,
            UI_FASE_CORTE_MECO, UI_SRB_SEPARATED, UI_CORE_MECO_CUTOFF, true
        }
    };
    static const size_t cantEventosPorSnapshot[4] = { 3, 4, 5, 6 };

    for (;;) {
        ui_limpiarPantalla(false);

        if (etapaActual == 1) {
            /* Vista 1: Lista de verificacion previa al lanzamiento */
            UiItemChecklist items[6] = {
                { "Sistema de propulsion principal",  "RS-25",     UI_ESTADO_OK,   "Presion nominal LH2/LOX" },
                { "Verificacion de trayectoria",      "GUIADO",    UI_ESTADO_OK,   "Vector de ascenso verificado" },
                { "Propulsores de cohete solido SRB", "SRB",       UI_ESTADO_OK,   "Acoplados y armados" },
                { "Computadora de vuelo",             "AVIONICA",  UI_ESTADO_OK,   "Telemetria sincronizada" },
                { "Conexion de telemetria",           "ENLACE",    UI_ESTADO_OK,   "Enlace de control activo" },
                { "Autorizacion de lanzamiento",      "DIRECCION", UI_ESTADO_WAIT, "En espera de autorizacion" }
            };

            /* Al presionar la tecla D, permite probar los distintos estados visuales */
            if (varianteEstadoChecklist == 1) {
                items[5].estado  = UI_ESTADO_OK;
                items[5].detalle = "Autorizacion otorgada (Demostracion)";
            } else if (varianteEstadoChecklist == 2) {
                items[0].estado  = UI_ESTADO_WARN;
                items[0].detalle = "Advertencia en sensor secundario (Demostracion)";
                items[5].estado  = UI_ESTADO_FAIL;
                items[5].detalle = "Lanzamiento retenido por anomalia (Prueba)";
            }

            ui_mostrarPreparacionLanzamiento(items, 6, true);
        } else if (etapaActual == 2) {
            /* Vista 2: Cuenta regresiva */
            ui_mostrarCuentaRegresiva(
                tiemposCountdown[indiceCountdown],
                mensajesCountdown[indiceCountdown],
                true
            );
        } else if (etapaActual == 3) {
            /* Vista 3: Telemetria en estado inicial (sin valores calculados) */
            ui_mostrarTelemetria(NULL, NULL, 0, false);
        } else if (etapaActual == 4) {
            /* Vista 4: Telemetria con datos de maqueta */
            ui_mostrarTelemetria(
                &snapshotsDemo[indiceSnapshotTelemetria],
                eventosDemo,
                cantEventosPorSnapshot[indiceSnapshotTelemetria],
                false
            );
        } else {
            /* Vista 5: Reporte final de mision */
            UiReporteFinal repDemo;
            repDemo.disponible             = true;
            repDemo.idMision               = 1;
            repDemo.estadoFinal            = UI_MISION_COMPLETADA;
            repDemo.parametrosUtilizados   = ui_obtenerParametrosReferencia();
            repDemo.velocidadSeparacionSRB = 2400.0;
            repDemo.altitudSeparacionSRB   = 144000.0;
            repDemo.velocidadCorteMotores  = 2482.5;
            repDemo.altitudMaximaCalculada = 474418.0;
            repDemo.aceleracionMaximaG     = 2.04;
            repDemo.duracionTotalSegundos  = 379.9;
            repDemo.eventosRelevantes      = eventosDemo;
            repDemo.cantidadEventos        = 6;
            repDemo.observaciones          =
                "Vista previa de estructura UiReporteFinal (los calculos reales vendran de fisica.c).";
            repDemo.esDemostracion         = true;

            ui_mostrarReporteFinal(&repDemo);
        }

        ui_mostrarAyudaNavegacion(
            "[1-5 / Izq/Der] Cambiar pantalla de vuelo  |  [D] Cambiar estado/muestra  |  [Enter] Siguiente  |  [Q] Menu"
        );

        UiAccionTecla t = ui_leerAccionTeclado();
        if (t == UI_TECLA_SALIR || t == UI_TECLA_ESCAPE) {
            break;
        } else if (t == UI_TECLA_NUMERO_1) {
            etapaActual = 1;
        } else if (t == UI_TECLA_NUMERO_2) {
            etapaActual = 2;
        } else if (t == UI_TECLA_NUMERO_3) {
            etapaActual = 3;
        } else if (t == UI_TECLA_NUMERO_4) {
            etapaActual = 4;
        } else if (t == UI_TECLA_NUMERO_5) {
            etapaActual = 5;
        } else if (t == UI_TECLA_DERECHA || t == UI_TECLA_ABAJO || t == UI_TECLA_ENTER) {
            etapaActual = (etapaActual % 5) + 1;
        } else if (t == UI_TECLA_IZQUIERDA || t == UI_TECLA_ARRIBA) {
            etapaActual = (etapaActual == 1) ? 5 : (etapaActual - 1);
        } else if (t == UI_TECLA_DEMO_TOGGLE) {
            if (etapaActual == 1) {
                varianteEstadoChecklist = (varianteEstadoChecklist + 1) % 3;
            } else if (etapaActual == 2) {
                indiceCountdown = (indiceCountdown + 1) % 4;
            } else if (etapaActual == 3) {
                etapaActual = 4;
            } else if (etapaActual == 4) {
                indiceSnapshotTelemetria = (indiceSnapshotTelemetria + 1) % 4;
            }
        }
    }
}

/**
 * @brief Permite navegar por la pantalla de Configuracion de Mision y probar
 *        el resaltado de cada uno de los 7 parametros fisicos.
 */
static void demo_explorarConfiguracion(void) {
    UiParametrosMision params = ui_obtenerParametrosReferencia();
    int seleccionado = 0;

    for (;;) {
        ui_limpiarPantalla(false);
        ui_mostrarConfiguracionMision(&params, seleccionado);
        ui_mostrarAyudaNavegacion(
            "[Arriba/Abajo o 1-7] Resaltar parametro  |  [Enter / Q / Esc] Volver al menu principal"
        );

        UiAccionTecla t = ui_leerAccionTeclado();
        if (t == UI_TECLA_SALIR || t == UI_TECLA_ESCAPE || t == UI_TECLA_ENTER) {
            break;
        } else if (t == UI_TECLA_ARRIBA) {
            seleccionado = (seleccionado == 0) ? 6 : (seleccionado - 1);
        } else if (t == UI_TECLA_ABAJO) {
            seleccionado = (seleccionado + 1) % 7;
        } else if (t >= UI_TECLA_NUMERO_1 && t <= UI_TECLA_NUMERO_7) {
            seleccionado = (int)(t - UI_TECLA_NUMERO_1);
        }
    }
}

/**
 * @brief Permite visualizar la pantalla de Analisis Cientifico tanto cuando
 *        aun no hay resultados (`NULL`) como cuando `fisica.c` entrega datos.
 */
static void demo_explorarAnalisisCientifico(void) {
    bool mostrarMaqueta = false;

    for (;;) {
        ui_limpiarPantalla(false);

        if (!mostrarMaqueta) {
            ui_mostrarAnalisisCientifico(NULL);
        } else {
            UiAnalisisCientifico maqueta;
            maqueta.resultadosDisponibles        = true;
            maqueta.tiempoSeparacionSRB          = 120.0;
            maqueta.velocidadSeparacionSRB       = 2400.0;
            maqueta.altitudSeparacionSRB         = 144000.0;
            maqueta.altitudCorteMotores          = 160000.0;
            maqueta.tiempoCorteMotores           = 126.6;
            maqueta.velocidadMaximaCorte         = 2482.5;
            maqueta.altitudMaximaApogeo          = 474418.0;
            maqueta.gananciaAltitudBalistica     = 314418.0;
            maqueta.tiempoApogeo                 = 379.9;
            maqueta.aceleracionFase1EnG          = 2.04;
            maqueta.aceleracionFase2EnG          = 1.28;
            maqueta.aceleracionMaximaEnG         = 2.04;
            maqueta.mostrarExplicacionConceptual = true;
            maqueta.esDemostracion               = true;

            ui_mostrarAnalisisCientifico(&maqueta);
        }

        ui_mostrarAyudaNavegacion(
            "[D] Alternar entre Estado Sin Datos y Maqueta Visual  |  [Enter / Q / Esc] Volver al menu"
        );

        UiAccionTecla t = ui_leerAccionTeclado();
        if (t == UI_TECLA_DEMO_TOGGLE) {
            mostrarMaqueta = !mostrarMaqueta;
        } else if (t == UI_TECLA_SALIR || t == UI_TECLA_ESCAPE || t == UI_TECLA_ENTER) {
            break;
        }
    }
}

/**
 * @brief Permite visualizar el Historial de Misiones vacio o con filas de ejemplo.
 */
static void demo_explorarHistorial(void) {
    bool mostrarMaqueta = false;
    static const UiRegistroMision registrosDemo[2] = {
        { 1, "SESION-DEMO-01", UI_MISION_COMPLETADA, 474418.0, 2482.5, 379.9 },
        { 2, "SESION-DEMO-02", UI_MISION_ABORTADA,    88500.0, 1450.0,  94.0 }
    };

    for (;;) {
        ui_limpiarPantalla(false);

        if (!mostrarMaqueta) {
            ui_mostrarHistorialMisiones(NULL, 0);
        } else {
            ui_mostrarAdvertencia(
                "MODO DEMOSTRACION VISUAL: Tabla renderizada con 2 estructuras UiRegistroMision de prueba."
            );
            ui_mostrarHistorialMisiones(registrosDemo, 2);
        }

        ui_mostrarAyudaNavegacion(
            "[D] Alternar entre Estado Vacio y Tabla de Maqueta  |  [Enter / Q / Esc] Volver al menu"
        );

        UiAccionTecla t = ui_leerAccionTeclado();
        if (t == UI_TECLA_DEMO_TOGGLE) {
            mostrarMaqueta = !mostrarMaqueta;
        } else if (t == UI_TECLA_SALIR || t == UI_TECLA_ESCAPE || t == UI_TECLA_ENTER) {
            break;
        }
    }
}

/**
 * @brief Imprime todas las pantallas de una sola vez cuando se ejecuta con `--preview-all`.
 */
static void demo_imprimirVistaCompletaNoInteractiva(void) {
    ui_inicializarTerminal();

    ui_mostrarTitulo("VISTA PREVIA AUTOMATICA DE SECCIONES - ARTEMIS CLI");
    ui_mostrarPantallaPrincipal(UI_MENU_INICIAR_MISION, UI_MISION_STANDBY, NULL);

    ui_mostrarConfiguracionMision(NULL, 0);
    ui_mostrarPreparacionLanzamiento(NULL, 0, true);
    ui_mostrarCuentaRegresiva(10.0, "Iniciando secuencia de lanzamiento...", true);
    ui_mostrarTelemetria(NULL, NULL, 0, false);
    ui_mostrarAnalisisCientifico(NULL);
    ui_mostrarHistorialMisiones(NULL, 0);
    ui_mostrarReporteFinal(NULL);
    ui_mostrarInstrucciones();
    ui_mostrarInformacionSistema(NULL, 0);
    ui_mostrarDespedida();
}

/* ==========================================================================
 * PUNTO DE ENTRADA DE LA DEMO INDEPENDIENTE
 * ========================================================================== */

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "--preview-all") == 0) {
        demo_imprimirVistaCompletaNoInteractiva();
        return 0;
    }

    UiOpcionesVisuales opt = ui_inicializarTerminal();
    UiOpcionMenu seleccion = UI_MENU_INICIAR_MISION;

    /* Bucle principal de navegacion de la demo */
    for (;;) {
        seleccion = ui_ejecutarMenuInteractivo(seleccion, UI_MISION_STANDBY, &opt);

        switch (seleccion) {
            case UI_MENU_INICIAR_MISION:
                demo_recorrerSecuenciaLanzamiento();
                break;

            case UI_MENU_CONFIGURACION:
                demo_explorarConfiguracion();
                break;

            case UI_MENU_ANALISIS_CIENTIFICO:
                demo_explorarAnalisisCientifico();
                break;

            case UI_MENU_HISTORIAL:
                demo_explorarHistorial();
                break;

            case UI_MENU_INSTRUCCIONES:
                ui_limpiarPantalla(false);
                ui_mostrarInstrucciones();
                ui_esperarConfirmacion(NULL);
                break;

            case UI_MENU_INFO_SISTEMA:
                ui_limpiarPantalla(false);
                ui_mostrarInformacionSistema(NULL, 0);
                ui_esperarConfirmacion(NULL);
                break;

            case UI_MENU_SALIR:
            default:
                ui_limpiarPantalla(false);
                ui_mostrarDespedida();
                return 0;
        }
    }
}
