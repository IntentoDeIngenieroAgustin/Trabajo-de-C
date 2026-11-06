/**
 * @file interfaz.h
 * @brief Contrato publico del modulo de interfaz de consola para ARTEMIS CLI.
 *
 * Proyecto: Simulador de Misiones Artemis - Escenario 1: Ascenso y Propulsion con el SLS
 * Asignatura: Algoritmos y Estructuras de Datos (Ingenieria en Sistemas de Informacion - UTN)
 * Estandar: ISO/IEC 9899:2011 (C11)
 *
 * ============================================================================
 * GUIA RAPIDA PARA LOS INTEGRANTES DEL EQUIPO (COMO USAR ESTE ARCHIVO):
 * ============================================================================
 * 1. Este modulo SOLO dibuja pantallas y lee la navegacion del menu.
 *    NO calcula formulas fisicas (eso va en `fisica.c`), NO valida rangos
 *    numericos (eso va en `validaciones.c`) y NO controla el bucle de la
 *    simulacion (eso va en `simulador.c`).
 *
 * 2. ¿Como conectarlo desde `main.c` y `simulador.c`?
 *    - Al iniciar el programa en `main()`, llamar una vez a `ui_inicializarTerminal()`.
 *    - Para mostrar el menu principal y saber que eligio el usuario, llamar a:
 *      `UiOpcionMenu op = ui_ejecutarMenuInteractivo(UI_MENU_INICIAR_MISION, UI_MISION_STANDBY, NULL);`
 *    - Durante la simulacion en `simulador.c`, cargar los datos calculados en una
 *      estructura `UiTelemetria` y llamar en cada turno/paso a:
 *      `ui_mostrarTelemetria(&datosTelemetria, listaEventos, cantEventos, true);`
 *
 * 3. ¿Como conectarlo desde `fisica.c` y `reportes.c`?
 *    - Cargar los resultados matematicos en `UiAnalisisCientifico` o `UiReporteFinal`
 *      y pasar su direccion de memoria (`&estructura`) a `ui_mostrarAnalisisCientifico()`
 *      o `ui_mostrarReporteFinal()`.
 */

#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * 1. CONSTANTES DE IDENTIDAD Y VALORES DE REFERENCIA VISUAL
 * ==========================================================================
 * Estas constantes definen los textos institucionales y los valores fisicos
 * predeterminados del Escenario 1 unicamente para mostrarlos por pantalla
 * cuando el usuario aun no ha cargado parametros personalizados.
 */

#define UI_NOMBRE_SISTEMA            "ARTEMIS CLI"
#define UI_SUBTITULO_SISTEMA         "Control de Mision SLS / Sistema de Simulacion de Vuelo"
#define UI_VERSION_SISTEMA           "1.0.0-TFI"
#define UI_ESCENARIO_CORTO           "ASCENSO Y PROPULSION"
#define UI_ESCENARIO_NOMBRE          "Escenario 1: Ascenso y Propulsion con el SLS"
#define UI_UNIVERSIDAD_INFO          "Universidad Tecnologica Nacional (UTN) - ISI"
#define UI_ASIGNATURA_INFO           "Algoritmos y Estructuras de Datos"

/* Anchos de referencia en columnas para adaptar el diseno al tamano de consola */
#define UI_ANCHO_MINIMO_COMPACTO     68
#define UI_ANCHO_ESTANDAR            86
#define UI_ANCHO_MAXIMO_PANEL        96

/* Valores fisicos predeterminados de referencia del Escenario 1 (Unidades SI) */
#define UI_REF_VELOCIDAD_INICIAL     0.0       /* v0 [m/s]   */
#define UI_REF_ALTITUD_INICIAL       0.0       /* h0 [m]     */
#define UI_REF_ACELERACION_FASE1     20.0      /* a1 [m/s^2] */
#define UI_REF_TIEMPO_SEP_SRB        120.0     /* t_SRB [s]  */
#define UI_REF_ACELERACION_FASE2     12.5      /* a2 [m/s^2] */
#define UI_REF_ALTITUD_CORTE         160000.0  /* h_corte [m]*/
#define UI_REF_GRAVEDAD              9.80      /* g [m/s^2]  */

/* ==========================================================================
 * 2. ENUMERACIONES DE INTERFAZ Y CONTRATOS DE ESTADO
 * ==========================================================================
 * Nota para el equipo: Usar estos enums en lugar de numeros magicos (1, 2, 3...)
 * hace que el codigo de `simulador.c` y `reportes.c` sea mucho mas legible.
 */

/**
 * @brief Opciones obligatorias del menu principal de Artemis CLI.
 *        `ui_ejecutarMenuInteractivo()` devuelve uno de estos valores.
 */
typedef enum {
    UI_MENU_INICIAR_MISION      = 1, /**< 1. Iniciar mision */
    UI_MENU_CONFIGURACION       = 2, /**< 2. Configuracion de mision */
    UI_MENU_ANALISIS_CIENTIFICO = 3, /**< 3. Analisis cientifico */
    UI_MENU_HISTORIAL           = 4, /**< 4. Historial de misiones */
    UI_MENU_INSTRUCCIONES       = 5, /**< 5. Instrucciones */
    UI_MENU_INFO_SISTEMA        = 6, /**< 6. Informacion del sistema */
    UI_MENU_SALIR               = 7, /**< 7. Salir */
    UI_MENU_TOTAL_OPCIONES      = 7  /**< Cantidad total de items del menu */
} UiOpcionMenu;

/**
 * @brief Acciones de teclado abstraidas por el sistema de navegacion.
 *        Permite manejar igual las flechas del teclado y los numeros 1-9.
 */
typedef enum {
    UI_TECLA_NINGUNA = 0,
    UI_TECLA_ARRIBA,
    UI_TECLA_ABAJO,
    UI_TECLA_IZQUIERDA,
    UI_TECLA_DERECHA,
    UI_TECLA_ENTER,
    UI_TECLA_ESCAPE,
    UI_TECLA_SALIR,
    UI_TECLA_AYUDA,
    UI_TECLA_DEMO_TOGGLE,
    UI_TECLA_COLOR_TOGGLE,
    UI_TECLA_COMPACTO_TOGGLE,
    UI_TECLA_NUMERO_1,
    UI_TECLA_NUMERO_2,
    UI_TECLA_NUMERO_3,
    UI_TECLA_NUMERO_4,
    UI_TECLA_NUMERO_5,
    UI_TECLA_NUMERO_6,
    UI_TECLA_NUMERO_7,
    UI_TECLA_NUMERO_8,
    UI_TECLA_NUMERO_9
} UiAccionTecla;

/**
 * @brief Estado general del sistema o de una mision para mostrar en encabezados y tablas.
 */
typedef enum {
    UI_MISION_STANDBY = 0,       /**< Sistema en espera (antes de iniciar) */
    UI_MISION_CONFIGURADA,       /**< Parametros cargados y listos */
    UI_MISION_PREFLIGHT,         /**< En verificacion previa de sistemas */
    UI_MISION_CUENTA_REGRESIVA,  /**< En secuencia de cuenta regresiva */
    UI_MISION_EN_VUELO,          /**< Simulacion activa con telemetria en vivo */
    UI_MISION_COMPLETADA,        /**< Mision finalizada con exito */
    UI_MISION_ABORTADA,          /**< Mision cancelada o abortada */
    UI_MISION_FALLO_CRITICO      /**< Mision fallida por condicion critica */
} UiEstadoMisionGeneral;

/**
 * @brief Fases operativas del vuelo durante el Escenario 1 (Ascenso SLS).
 */
typedef enum {
    UI_FASE_STANDBY = 0,         /**< En espera en plataforma de lanzamiento */
    UI_FASE_PRELANZAMIENTO,      /**< Verificacion y conteo previo */
    UI_FASE_1_ASCENSO_SRB,       /**< Fase 1: Empuje conjunto Core Stage + SRB */
    UI_FASE_SEPARACION_SRB,      /**< Evento de desacople de los propulsores SRB */
    UI_FASE_2_PROPULSION_CORE,   /**< Fase 2: Propulsion con motores principales RS-25 */
    UI_FASE_CORTE_MECO,          /**< Corte de motores principales en altitud objetivo */
    UI_FASE_VUELO_BALISTICO,     /**< Ascenso inercial libre post-apagado hasta apogeo */
    UI_FASE_FINALIZADA,          /**< Simulacion concluida */
    UI_FASE_ABORTADA             /**< Vuelo abortado */
} UiFaseVuelo;

/**
 * @brief Estado visual de los propulsores laterales de combustible solido (SRB).
 */
typedef enum {
    UI_SRB_ATTACHED = 0,         /**< Acoplados al vehiculo en espera */
    UI_SRB_ACTIVE_BURN,          /**< Ignicion activa generando empuje (Fase 1) */
    UI_SRB_SEPARATED,            /**< Desacoplados tras cumplir t_SRB */
    UI_SRB_FAULT                 /**< Fallo o anomalia en propulsores */
} UiEstadoSRB;

/**
 * @brief Estado visual de los motores principales RS-25 de la etapa central (Core Stage).
 */
typedef enum {
    UI_CORE_READY = 0,           /**< Listos en plataforma */
    UI_CORE_IGNITION,            /**< En secuencia de arranque */
    UI_CORE_ACTIVE_THRUST,       /**< Empuje nominal activo */
    UI_CORE_MECO_CUTOFF,         /**< Motores apagados tras alcanzar altitud de corte */
    UI_CORE_FAULT                /**< Fallo en motores principales */
} UiEstadoMotorCore;

/**
 * @brief Estados de verificacion para subsistemas (Lista de verificacion pre-vuelo y modulos).
 */
typedef enum {
    UI_ESTADO_OK = 0,            /**< [OK]     Verde: Subsistema verificado */
    UI_ESTADO_WAIT,              /**< [ESPERA] Amarillo: Pendiente de autorizacion/accion */
    UI_ESTADO_WARN,              /**< [AVISO]  Amarillo: Advertencia no bloqueante */
    UI_ESTADO_FAIL,              /**< [FALLO]  Rojo: Error o condicion critica */
    UI_ESTADO_INFO               /**< [INFO]   Celeste: Nota informativa */
} UiEstadoSistema;

/**
 * @brief Catalogo de tipos de eventos relevantes durante la mision.
 *        `simulador.c` puede enviar cualquiera de estos tipos para el registro de eventos.
 */
typedef enum {
    UI_EVENTO_INICIO_LANZAMIENTO = 0,
    UI_EVENTO_DESPEGUE,
    UI_EVENTO_ASCENSO_NOMINAL,
    UI_EVENTO_SEPARACION_SRB,
    UI_EVENTO_ACTIVACION_FASE_2,
    UI_EVENTO_CORTE_MOTORES,
    UI_EVENTO_FINALIZACION_MISION,
    UI_EVENTO_FALLO_MISION,
    UI_EVENTO_PERSONALIZADO
} UiTipoEvento;

/**
 * @brief Nivel de severidad visual de un evento en la bitacora.
 */
typedef enum {
    UI_NIVEL_EVENTO = 0,         /**< Etiqueta [EVENTO] en azul electrico */
    UI_NIVEL_INFO,               /**< Etiqueta [INFO]   en celeste */
    UI_NIVEL_OK,                 /**< Etiqueta [OK]     en verde */
    UI_NIVEL_WARN,               /**< Etiqueta [AVISO]  en amarillo */
    UI_NIVEL_CRITICAL            /**< Etiqueta [FALLO]  en rojo */
} UiNivelEvento;

/**
 * @brief Estado de implementacion de cada archivo `.c`/`.h` del trabajo practico.
 */
typedef enum {
    UI_MODULO_PENDIENTE = 0,
    UI_MODULO_EN_DESARROLLO,
    UI_MODULO_IMPLEMENTADO
} UiEstadoImplementacionModulo;

/* ==========================================================================
 * 3. ESTRUCTURAS DE DATOS PARA PRESENTACION (CONTRATOS ENTRE MODULOS)
 * ==========================================================================
 * Nota para el equipo: Estas estructuras agrupan los datos que `interfaz.c`
 * necesita recibir por puntero constante (`const Estructura *`) para dibujar
 * las pantallas sin usar variables globales.
 */

/**
 * @brief Opciones de adaptacion visual de la terminal.
 */
typedef struct {
    bool usarColoresAnsi;    /**< true para habilitar secuencias de color ANSI */
    bool usarUnicode;        /**< true para bordes Unicode, false para ASCII puro */
    bool modoCompacto;       /**< true cuando el ancho de terminal es menor a 68 col. */
    int  anchoTerminal;      /**< Ancho detectado de la consola en columnas */
} UiOpcionesVisuales;

/**
 * @brief Parametros fisicos de entrada del Escenario 1 (Ascenso y Propulsion).
 */
typedef struct {
    double velocidadInicial;     /**< Velocidad inicial v0 [m/s] */
    double altitudInicial;       /**< Altitud inicial h0 [m] */
    double aceleracionFase1;     /**< Aceleracion fase 1 (SRB + Etapa Central) [m/s^2] */
    double tiempoSeparacionSRB;  /**< Tiempo de separacion de los SRB [s] */
    double aceleracionFase2;     /**< Aceleracion fase 2 (Etapa Central) [m/s^2] */
    double altitudCorte;         /**< Altitud objetivo de corte de motores (MECO) [m] */
    double gravedad;             /**< Aceleracion gravitatoria terrestre [m/s^2] */
    bool   personalizados;       /**< true si el usuario modifico los valores por defecto */
} UiParametrosMision;

/**
 * @brief Elemento individual de verificacion en la pantalla de preparacion pre-vuelo.
 */
typedef struct {
    const char      *sistema;        /**< Nombre del subsistema (ej. "Computadora de vuelo") */
    const char      *nombreTecnico;  /**< Codigo o subsistema especifico */
    UiEstadoSistema  estado;         /**< Estado visual ([OK], [ESPERA], [AVISO], [FALLO]) */
    const char      *detalle;        /**< Descripcion breve del estado actual */
} UiItemChecklist;

/**
 * @brief Registro de un evento puntual ocurrido durante la mision.
 */
typedef struct {
    double        tiempoSegundos;  /**< Instante T+ en segundos en que ocurrio el evento */
    UiTipoEvento  tipo;            /**< Tipo de evento del catalogo */
    UiNivelEvento nivel;           /**< Severidad visual ([EVENTO], [INFO], [AVISO], etc.) */
    const char   *descripcion;     /**< Texto personalizado (si es NULL, usa texto estandar) */
} UiEventoMision;

/**
 * @brief Datos instantaneos recibidos desde `simulador.c` para la telemetria en tiempo real.
 */
typedef struct {
    bool              datosDisponibles;      /**< Si es false, muestra "-- m" y estado EN ESPERA */
    double            tiempoTranscurrido;    /**< Tiempo de mision T+ en segundos */
    double            altitud;               /**< Altitud actual [m] */
    double            velocidad;             /**< Velocidad actual [m/s] */
    double            aceleracion;           /**< Aceleracion actual [m/s^2] */
    double            altitudObjetivoCorte;  /**< Altitud objetivo para calcular progreso [m] */
    double            progresoAscenso;       /**< Porcentaje [0.0 a 100.0] hacia el corte de motores */
    UiFaseVuelo       faseActual;            /**< Fase de vuelo activa */
    UiEstadoSRB       estadoSRB;             /**< Estado actual de los propulsores SRB */
    UiEstadoMotorCore estadoMotorPrincipal;  /**< Estado actual de los motores RS-25 */
    bool              esDemostracion;        /**< true solo en `demo_interfaz.c` para mostrar aviso */
} UiTelemetria;

/**
 * @brief Resultados calculados por `fisica.c` para las 4 categorias del Analisis Cientifico.
 */
typedef struct {
    bool   resultadosDisponibles;        /**< Si es false, informa: "Sin resultados disponibles..." */
    /* Categoria 1: Velocidad y altitud durante la separacion SRB */
    double velocidadSeparacionSRB;       /**< Velocidad alcanzada en t_SRB [m/s] */
    double altitudSeparacionSRB;         /**< Altitud alcanzada en t_SRB [m] */
    double tiempoSeparacionSRB;          /**< Instante de separacion SRB [s] */
    /* Categoria 2: Velocidad maxima al corte de motores (MECO) */
    double velocidadMaximaCorte;         /**< Velocidad en el instante de corte MECO [m/s] */
    double altitudCorteMotores;          /**< Altitud a la que se apagan los motores [m] */
    double tiempoCorteMotores;           /**< Tiempo total hasta el corte de motores [s] */
    /* Categoria 3: Altitud maxima posterior al apagado (apogeo balistico) */
    double altitudMaximaApogeo;          /**< Altitud maxima final alcanzada [m] */
    double gananciaAltitudBalistica;     /**< Metros ascendidos por inercia tras apagar motores [m] */
    double tiempoApogeo;                 /**< Instante T+ en que alcanza la altura maxima [s] */
    /* Categoria 4: Aceleracion expresada en multiplos de gravedad (g) */
    double aceleracionFase1EnG;          /**< Aceleracion de Fase 1 dividida por g [g] */
    double aceleracionFase2EnG;          /**< Aceleracion de Fase 2 dividida por g [g] */
    double aceleracionMaximaEnG;         /**< Mayor aceleracion experimentada en unidades [g] */
    bool   mostrarExplicacionConceptual; /**< true para mostrar la explicacion fisica de cada item */
    bool   esDemostracion;               /**< true si son valores de maqueta para la demo */
} UiAnalisisCientifico;

/**
 * @brief Fila individual para la tabla del Historial de Misiones.
 */
typedef struct {
    int                   idMision;             /**< Numero de mision (ej. 1, 2, 3...) */
    const char           *identificadorSesion;  /**< Etiqueta de sesion o marca de tiempo */
    UiEstadoMisionGeneral estadoFinal;          /**< Resultado de la mision */
    double                altitudMaxima;        /**< Altitud maxima alcanzada [m] */
    double                velocidadMaxima;      /**< Velocidad maxima alcanzada [m/s] */
    double                duracionTotal;        /**< Duracion total de la mision [s] */
} UiRegistroMision;

/**
 * @brief Estructura integral para presentar la pantalla de Reporte Final (`reportes.c`).
 */
typedef struct {
    bool                  disponible;              /**< true si ya se ejecuto al menos una mision */
    int                   idMision;                /**< Numero identificador de la mision */
    UiEstadoMisionGeneral estadoFinal;             /**< Estado final de la mision */
    UiParametrosMision    parametrosUtilizados;    /**< Parametros con los que se corrio la mision */
    double                velocidadSeparacionSRB;  /**< Velocidad al separar SRB [m/s] */
    double                altitudSeparacionSRB;    /**< Altitud al separar SRB [m] */
    double                velocidadCorteMotores;   /**< Velocidad al apagar motores principales [m/s] */
    double                altitudMaximaCalculada;  /**< Altitud maxima (apogeo) [m] */
    double                aceleracionMaximaG;      /**< Aceleracion maxima en multiplos de g [g] */
    double                duracionTotalSegundos;   /**< Tiempo total de vuelo simulado [s] */
    const UiEventoMision *eventosRelevantes;       /**< Puntero al arreglo de eventos de la mision */
    size_t                cantidadEventos;         /**< Cantidad de eventos en el arreglo */
    const char           *observaciones;           /**< Comentario o conclusion final del reporte */
    bool                  esDemostracion;          /**< true si se muestra como maqueta en la demo */
} UiReporteFinal;

/**
 * @brief Estado de implementacion de un modulo del proyecto (pantalla de Informacion del Sistema).
 */
typedef struct {
    const char                  *nombreModulo;     /**< Nombre de los archivos (ej. "src/fisica.c") */
    const char                  *responsabilidad;  /**< Descripcion breve de lo que hace el modulo */
    UiEstadoImplementacionModulo estado;           /**< Pendiente, En desarrollo o Implementado */
} UiEstadoModulo;

/* ==========================================================================
 * 4. INICIALIZACION Y CONFIGURACION DE TERMINAL
 * ========================================================================== */

/**
 * @brief Inicializa la consola (habilita UTF-8 y colores ANSI en Windows/Linux/macOS).
 *        Se recomienda llamarla al principio de `main()`.
 */
UiOpcionesVisuales ui_inicializarTerminal(void);

/**
 * @brief Obtiene las opciones visuales actuales segun el tamano de la ventana.
 */
UiOpcionesVisuales ui_obtenerOpcionesVisuales(void);

/**
 * @brief Devuelve el ancho actual de la consola en cantidad de columnas.
 */
int ui_obtenerAnchoTerminal(void);

/**
 * @brief Limpia la pantalla sin usar `system("cls")` ni `system("clear")`.
 * @param enSitio Si es true, solo mueve el cursor al inicio (ideal para actualizar
 *                la telemetria en tiempo real sin que la pantalla parpadee).
 */
void ui_limpiarPantalla(bool enSitio);

/**
 * @brief Devuelve una estructura `UiParametrosMision` cargada con los valores por defecto.
 */
UiParametrosMision ui_obtenerParametrosReferencia(void);

/* ==========================================================================
 * 5. SISTEMA DE COMPONENTES VISUALES REUTILIZABLES (SECCION 7)
 * ==========================================================================
 * Estas funciones pueden ser llamadas desde cualquier modulo (por ejemplo,
 * desde `validaciones.c` para mostrar un error cuando el usuario ingresa un
 * dato invalido).
 */

void ui_mostrarBienvenida(void);
void ui_mostrarTitulo(const char *titulo);
void ui_mostrarSubtitulo(const char *subtitulo);
void ui_mostrarEncabezado(const char *seccion, UiEstadoMisionGeneral estadoSistema);
void ui_mostrarSeparador(void);
void ui_mostrarSeparadorDoble(void);
void ui_mostrarTextoDestacado(const char *etiqueta, const char *valor);
void ui_mostrarError(const char *mensaje);
void ui_mostrarExito(const char *mensaje);
void ui_mostrarAdvertencia(const char *mensaje);
void ui_mostrarInfo(const char *mensaje);
void ui_mostrarProgreso(double porcentaje);
void ui_mostrarProgresoEtiquetado(const char *etiqueta, double porcentaje, int anchoBarra);

/**
 * @brief Muestra una lista de opciones seleccionables con cursor activo.
 */
void ui_mostrarListaOpciones(
    const char * const *opciones,
    const char * const *descripciones,
    size_t cantidad,
    size_t indiceSeleccionado
);

/**
 * @brief Dibuja una tabla de datos generica alineada por columnas.
 */
void ui_mostrarTabla(
    const char * const *encabezados,
    const int *anchosColumnas,
    size_t numColumnas,
    const char * const *celdasFilas,
    size_t numFilas
);

/**
 * @brief Muestra el estado de un sistema con etiqueta coloreada ([OK], [ESPERA], [AVISO], [FALLO]).
 */
void ui_mostrarEstadoSistema(
    const char *nombreSistema,
    UiEstadoSistema estado,
    const char *detalle
);

/**
 * @brief Imprime una linea de evento con formato `T+ 120s [EVENTO] Descripcion...`.
 */
void ui_mostrarEvento(const UiEventoMision *evento);

/**
 * @brief Imprime la lista completa de eventos recientes de la mision.
 */
void ui_mostrarListaEventos(const UiEventoMision *eventos, size_t cantidad);

/**
 * @brief Muestra el pie de pagina con los controles de teclado disponibles.
 */
void ui_mostrarAyudaNavegacion(const char *contextoControles);

/**
 * @brief Muestra una pantalla emergente con la guia de teclas rapidas (tecla H o ?).
 */
void ui_mostrarAyudaContextual(void);

/* ==========================================================================
 * 6. PANTALLAS COMPLETAS DEL SIMULADOR (SECCION 5: A - L)
 * ========================================================================== */

/**
 * @brief Secciones A y B: Dibuja el logo, el estado del sistema y el menu principal.
 */
void ui_mostrarPantallaPrincipal(
    UiOpcionMenu opcionSeleccionada,
    UiEstadoMisionGeneral estadoActual,
    const UiOpcionesVisuales *opciones
);

/**
 * @brief Seccion B: Dibuja unicamente las 7 opciones del menu principal.
 */
void ui_mostrarMenuPrincipal(UiOpcionMenu opcionSeleccionada);

/**
 * @brief Seccion C: Muestra la tabla de parametros fisicos de la mision.
 * @param parametros Puntero a los parametros (si es NULL, muestra los de referencia).
 * @param parametroResaltado Indice (0 a 6) del parametro que se esta editando, o -1.
 */
void ui_mostrarConfiguracionMision(
    const UiParametrosMision *parametros,
    int parametroResaltado
);

/**
 * @brief Seccion D: Muestra la lista de verificacion previa al lanzamiento (pre-vuelo).
 */
void ui_mostrarPreparacionLanzamiento(
    const UiItemChecklist *items,
    size_t cantidad,
    bool esDemostracion
);

/**
 * @brief Seccion E: Muestra el reloj de cuenta regresiva `T - MM:SS` y barra de ignicion.
 */
void ui_mostrarCuentaRegresiva(
    double segundosRestantes,
    const char *mensajeEstado,
    bool esDemostracion
);

/**
 * @brief Secciones F y G: Muestra el panel de telemetria en tiempo real y el registro de eventos.
 */
void ui_mostrarTelemetria(
    const UiTelemetria *telemetria,
    const UiEventoMision *eventos,
    size_t cantidadEventos,
    bool actualizarEnSitio
);

/**
 * @brief Seccion H: Muestra las 4 categorias del Analisis Cientifico del Escenario 1.
 */
void ui_mostrarAnalisisCientifico(const UiAnalisisCientifico *analisis);

/**
 * @brief Seccion I: Muestra la tabla del historial de misiones o el mensaje de lista vacia.
 */
void ui_mostrarHistorialMisiones(const UiRegistroMision *registros, size_t cantidad);

/**
 * @brief Seccion J: Muestra el reporte final completo de la mision.
 */
void ui_mostrarReporteFinal(const UiReporteFinal *reporte);

/**
 * @brief Seccion K: Muestra las instrucciones de uso y el glosario tecnico (SLS, SRB, MRUV).
 */
void ui_mostrarInstrucciones(void);

/**
 * @brief Seccion L: Muestra los datos academicos del proyecto y el estado de los modulos.
 */
void ui_mostrarInformacionSistema(const UiEstadoModulo *modulos, size_t cantidadModulos);

/**
 * @brief Muestra el mensaje de cierre al salir del simulador.
 */
void ui_mostrarDespedida(void);

/* ==========================================================================
 * 7. SISTEMA DE NAVEGACION Y LECTURA DE TECLADO COMPATIBLE (SECCION 6)
 * ========================================================================== */

/**
 * @brief Lee una tecla de navegacion (flechas, Enter, Esc, Q, H o numeros 1-9).
 *        Si la consola no soporta lectura directa, usa `fgets()` automaticamente.
 */
UiAccionTecla ui_leerAccionTeclado(void);

/**
 * @brief Ejecuta el menu principal interactivo y devuelve la opcion seleccionada.
 */
UiOpcionMenu ui_ejecutarMenuInteractivo(
    UiOpcionMenu opcionInicial,
    UiEstadoMisionGeneral estadoSistema,
    const UiOpcionesVisuales *opciones
);

/**
 * @brief Pausa la pantalla actual hasta que el usuario presione Enter, Escape o Q.
 */
void ui_esperarConfirmacion(const char *mensaje);

/* ==========================================================================
 * 8. FUNCIONES PUENTE DE COMPATIBILIDAD CON EL README GENERAL DEL REPOSITORIO
 * ==========================================================================
 * Se incluyen estos nombres exactos sugeridos en el README.md general para que,
 * si algun companero los invoca directamente, funcionen sin errores de enlace.
 */

void mostrarBienvenida(void);
void mostrarMenu(void);
void mostrarInstrucciones(void);
void mostrarFaseActual(int fase);
void mostrarDespedida(void);

#ifdef __cplusplus
}
#endif

#endif /* INTERFAZ_H */
