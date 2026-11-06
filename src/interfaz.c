/**
 * @file interfaz.c
 * @brief Implementacion del modulo de interfaz de usuario para ARTEMIS CLI.
 *
 * Proyecto: Simulador de Misiones Artemis - Escenario 1: Ascenso y Propulsion con el SLS
 * Asignatura: Algoritmos y Estructuras de Datos (Ingenieria en Sistemas de Informacion - UTN)
 * Estandar: ISO/IEC 9899:2011 (C11)
 *
 * ============================================================================
 * NOTA IMPORTANTE PARA LOS COMPANEROS DEL EQUIPO:
 * ============================================================================
 * 1. ¿Que hace este archivo?
 *    Concentra TODA la salida por pantalla (`printf`) y la lectura de navegacion
 *    del menu principal. Ningun otro modulo necesita preocuparse por colores,
 *    bordes ni alineacion de tablas.
 *
 * 2. ¿Por que NO hay variables globales mutables aqui?
 *    Por requisito academico de programacion estructurada. Las opciones visuales
 *    (como el ancho de la ventana o si se usan colores) se consultan al momento
 *    mediante `ui_obtenerOpcionesVisuales()` o se reciben por parametro.
 *
 * 3. ¿Como se limpia la pantalla sin usar `system("cls")` ni `system("clear")`?
 *    Se utilizan secuencias de escape estandar ANSI/VT100 (`\033[2J\033[H`).
 *    Ademas, durante la telemetria en tiempo real, el modo `enSitio = true` usa
 *    unicamente `\033[H` (volver el cursor arriba a la izquierda) para sobreescribir
 *    el panel sin parpadeos.
 *
 * 4. ¿Como funciona la navegacion con flechas sin usar `<conio.h>`?
 *    - En Windows se usa la API estandar de consola (`ReadConsoleInputA` de `<windows.h>`).
 *    - En Linux/macOS se usa `termios` de POSIX (`<termios.h>`).
 *    - Si la terminal de algun integrante o docente no soporta captura directa de
 *      teclas, el programa pasa automaticamente a leer con `fgets()` (escribiendo
 *      el numero de opcion 1-7 y presionando Enter).
 */

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/* Busqueda automatica de `interfaz.h`:
 * Permite compilar tanto si se pasa `-Iinclude` / `-Iincluide` como si algun
 * companero compila directamente desde adentro de la carpeta `src/`. */
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

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Inclusiones condicionales segun el sistema operativo para manejar la terminal */
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

/* ==========================================================================
 * 1. PALETA DE COLORES ANSI Y SIMBOLOS GRAFICOS (ESTETICA GEMINI CLI)
 * ==========================================================================
 * Los codigos `\033[...m` son secuencias estandar ANSI que cambian el color
 * del texto en terminales modernas (Windows Terminal, PowerShell, VS Code, Linux).
 * Si la terminal no soporta colores, la funcion `ui_color()` devuelve `""`
 * automaticamente para que no se impriman caracteres raros.
 */

#define ANSI_RESET          "\033[0m"
#define ANSI_BOLD           "\033[1m"
#define ANSI_DIM            "\033[2m"

/* Paleta aeroespacial:
 * - Azul electrico: titulos, bordes activos y cursor de seleccion
 * - Celeste: datos de telemetria, valores numericos y barras de progreso
 * - Blanco suave: textos principales y opciones seleccionadas
 * - Gris: textos secundarios, marcos y ayudas de teclado
 * - Verde: estados correctos [OK]
 * - Amarillo: estados de espera o advertencia [ESPERA] / [AVISO]
 * - Rojo: errores o fallos criticos [FALLO]
 */
#define COL_AZUL_ELECTRICO  "\033[1;38;5;39m"
#define COL_AZUL_PROFUNDO   "\033[1;38;5;33m"
#define COL_AZUL_MEDIO      "\033[1;38;5;45m"
#define COL_CELESTE         "\033[1;38;5;81m"
#define COL_CELESTE_SUAVE   "\033[38;5;117m"
#define COL_BLANCO_SUAVE    "\033[97m"
#define COL_GRIS            "\033[38;5;245m"
#define COL_GRIS_OSCURO     "\033[38;5;239m"
#define COL_VERDE           "\033[1;38;5;83m"
#define COL_AMARILLO        "\033[1;38;5;220m"
#define COL_ROJO            "\033[1;38;5;196m"

/* Simbolos Unicode expresados en bytes UTF-8 (`\xE2...`):
 * Se escriben en hexadecimal para evitar que distintos compiladores o
 * configuraciones regionales de Windows deformen los bordes de las cajas. */
#define SYM_CURSOR_UTF8     "\xE2\x9D\xAF" /* ❯ Cursor de seleccion */
#define SYM_PUNTO_UTF8      "\xE2\x80\xA2" /* • Separador visual */
#define SYM_CIRCULO_UTF8    "\xE2\x97\x8F" /* ● Indicador luminoso de estado */
#define SYM_FLECHA_ARR_UTF8 "\xE2\x86\x91" /* ↑ Flecha arriba */
#define SYM_FLECHA_ABA_UTF8 "\xE2\x86\x93" /* ↓ Flecha abajo */
#define SYM_BLOQUE_LLENO    "\xE2\x96\x88" /* █ Bloque lleno para barra de progreso */
#define SYM_BLOQUE_VACIO    "\xE2\x96\x91" /* ░ Bloque vacio para barra de progreso */

/* Caracteres de dibujo de cajas con esquinas redondeadas (estilo Gemini CLI) */
#define BOX_TL_UTF8         "\xE2\x95\xAD" /* ╭ Esquina superior izquierda */
#define BOX_TR_UTF8         "\xE2\x95\xAE" /* ╮ Esquina superior derecha */
#define BOX_BL_UTF8         "\xE2\x95\xB0" /* ╰ Esquina inferior izquierda */
#define BOX_BR_UTF8         "\xE2\x95\xAF" /* ╯ Esquina inferior derecha */
#define BOX_H_UTF8          "\xE2\x94\x80" /* ─ Linea horizontal simple */
#define BOX_V_UTF8          "\xE2\x94\x82" /* │ Linea vertical simple */
#define BOX_DH_UTF8         "\xE2\x95\x90" /* ═ Linea horizontal doble */

/* ==========================================================================
 * 2. TABLAS CONSTANTES DE SOLO LECTURA (LOGOTIPO Y TEXTOS DEL MENU)
 * ==========================================================================
 * Al ser `static const`, residen en memoria de solo lectura y cumplen con la
 * restriccion de no usar variables globales mutables.
 */

/* Logotipo principal "ARTEMIS" en bloques Unicode para terminales estandar (>= 68 col.) */
static const char * const K_LOGO_ARTEMIS_UTF8[6] = {
    "   \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97 "
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97 "
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97   \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97",

    "  \xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97"
    "\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x90\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97 \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D",

    "  \xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x9D"
    "   \xE2\x96\x88\xE2\x96\x88\xE2\x95\x91   "
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97  "
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97",

    "  \xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97"
    "   \xE2\x96\x88\xE2\x96\x88\xE2\x95\x91   "
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D  "
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91\xE2\x95\x9A\xE2\x96\x88\xE2\x96\x88\xE2\x95\x94\xE2\x95\x9D\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91",

    "  \xE2\x96\x88\xE2\x96\x88\xE2\x95\x91  \xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91  \xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "   \xE2\x96\x88\xE2\x96\x88\xE2\x95\x91   "
    "\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x97"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91 \xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D \xE2\x96\x88\xE2\x96\x88\xE2\x95\x91"
    "\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x96\x88\xE2\x95\x91",

    "  \xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D  \xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D"
    "\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D  \xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D"
    "   \xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D   "
    "\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D"
    "\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D     \xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D"
    "\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x9D\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D"
};

/* Logotipo alternativo en ASCII de 7 bits para consolas sin soporte Unicode */
static const char * const K_LOGO_ARTEMIS_ASCII[6] = {
    "    _    ____ _____ _____ __  __ ___ ____  ",
    "   / \\  |  _ \\_   _| ____|  \\/  |_ _/ ___| ",
    "  / _ \\ | |_) || | |  _| | |\\/| || |\\___ \\ ",
    " / ___ \\|  _ < | | | |___| |  | || | ___) |",
    "/_/   \\_\\_| \\_\\|_| |_____|_|  |_|___|____/ ",
    "                                           "
};

/* Las 7 opciones obligatorias del menu principal */
static const char * const K_MENU_OPCIONES[UI_MENU_TOTAL_OPCIONES] = {
    "Iniciar mision",
    "Configuracion de mision",
    "Analisis cientifico",
    "Historial de misiones",
    "Instrucciones",
    "Informacion del sistema",
    "Salir"
};

/* Etiquetas breves en espanol que acompanan a cada opcion del menu */
static const char * const K_MENU_ETIQUETAS_SEC[UI_MENU_TOTAL_OPCIONES] = {
    "SECUENCIA DE VUELO",
    "PARAMETROS FISICOS",
    "CALCULO CINEMATICO",
    "REGISTRO DE SESION",
    "MANUAL OPERATIVO",
    "FICHA DEL SISTEMA",
    "FINALIZAR SESION"
};

/* Descripcion contextual que aparece debajo del menu al resaltar cada opcion */
static const char * const K_MENU_DESCRIPCIONES[UI_MENU_TOTAL_OPCIONES] = {
    "Verificacion previa de sistemas, cuenta regresiva y telemetria SLS en vivo",
    "Visualizar los parametros fisicos del Escenario 1 (Ascenso y Propulsion)",
    "Consultar metricas de separacion SRB, corte de motores, apogeo y fuerzas G",
    "Registro cronologico de las simulaciones ejecutadas en la sesion",
    "Guia operativa del simulador, glosario SLS/SRB/MRUV y controles de teclado",
    "Especificaciones tecnicas, arquitectura modular C11 y estado de desarrollo",
    "Cerrar la consola interactiva del sistema de control de mision Artemis"
};

/* ==========================================================================
 * 3. FUNCIONES INTERNAS AUXILIARES DE FORMATO Y DETECCION DE TERMINAL
 * ==========================================================================
 * Estas funciones son `static` (privadas de `interfaz.c`) porque solo sirven
 * de apoyo interno para construir las pantallas.
 */

/**
 * @brief Devuelve la secuencia de color ANSI si los colores estan activos,
 *        o una cadena vacia `""` si estan desactivados.
 */
static const char *ui_color(const UiOpcionesVisuales *opt, const char *secuenciaAnsi) {
    if (opt != NULL && !opt->usarColoresAnsi) {
        return "";
    }
    return secuenciaAnsi;
}

/**
 * @brief Revisa si el usuario definio variables de entorno para desactivar el color.
 */
static bool ui_entornoDesactivaColor(void) {
    const char *noColor = getenv("NO_COLOR");
    const char *artemisNoColor = getenv("ARTEMIS_NO_COLOR");
    const char *term = getenv("TERM");

    if (noColor != NULL && noColor[0] != '\0') {
        return true;
    }
    if (artemisNoColor != NULL && artemisNoColor[0] != '\0' && strcmp(artemisNoColor, "0") != 0) {
        return true;
    }
    if (term != NULL && strcmp(term, "dumb") == 0) {
        return true;
    }
    return false;
}

/**
 * @brief Revisa si se forzo el modo ASCII puro mediante `ARTEMIS_ASCII=1`.
 */
static bool ui_entornoFuerzaAscii(void) {
    const char *asciiEnv = getenv("ARTEMIS_ASCII");
    return (asciiEnv != NULL && asciiEnv[0] != '\0' && strcmp(asciiEnv, "0") != 0);
}

/**
 * @brief Revisa si se forzo el diseno compacto mediante `ARTEMIS_COMPACT=1`.
 */
static bool ui_entornoFuerzaCompacto(void) {
    const char *compactEnv = getenv("ARTEMIS_COMPACT");
    return (compactEnv != NULL && compactEnv[0] != '\0' && strcmp(compactEnv, "0") != 0);
}

int ui_obtenerAnchoTerminal(void) {
    /* 1. Primero consulta si existe la variable de entorno COLUMNS */
    const char *colsEnv = getenv("COLUMNS");
    if (colsEnv != NULL && colsEnv[0] != '\0') {
        char *fin = NULL;
        long valor = strtol(colsEnv, &fin, 10);
        if (fin != colsEnv && valor >= 40 && valor <= 300) {
            return (int)valor;
        }
    }

    /* 2. Si estamos en Windows, consulta a la API de consola el ancho de la ventana */
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &csbi)) {
        int ancho = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        if (ancho >= 40 && ancho <= 300) {
            return ancho;
        }
    }
    /* 3. Si estamos en Linux o macOS, consulta mediante ioctl(TIOCGWINSZ) */
#elif defined(__unix__) || defined(__APPLE__)
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col >= 40) {
        return (int)ws.ws_col;
    }
#endif

    /* 4. Valor seguro por defecto si no se pudo detectar */
    return UI_ANCHO_ESTANDAR;
}

UiOpcionesVisuales ui_obtenerOpcionesVisuales(void) {
    UiOpcionesVisuales opt;
    int ancho = ui_obtenerAnchoTerminal();

    opt.anchoTerminal   = ancho;
    opt.usarColoresAnsi = !ui_entornoDesactivaColor();
    opt.usarUnicode     = !ui_entornoFuerzaAscii();
    opt.modoCompacto    = ui_entornoFuerzaCompacto() || (ancho < UI_ANCHO_MINIMO_COMPACTO);

    return opt;
}

UiOpcionesVisuales ui_inicializarTerminal(void) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();

#ifdef _WIN32
    /* En Windows, forzamos la pagina de codigos 65001 (UTF-8) para que las lineas
     * de las cajas y el logo se vean bien en PowerShell, CMD y VS Code. */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    /* Activamos el procesamiento de secuencias virtuales ANSI (colores y limpieza) */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD modoActual = 0;
        if (GetConsoleMode(hOut, &modoActual)) {
            DWORD modoNuevo = modoActual | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            if (!SetConsoleMode(hOut, modoNuevo)) {
                /* Si es una consola muy antigua que rechaza ANSI, pasamos a modo plano */
                opt.usarColoresAnsi = false;
                opt.usarUnicode     = false;
            }
        }
    }
#endif

    if (ui_entornoDesactivaColor()) {
        opt.usarColoresAnsi = false;
    }
    if (ui_entornoFuerzaAscii()) {
        opt.usarUnicode = false;
    }

    return opt;
}

/**
 * @brief Calcula el ancho util de los paneles respetando margenes y limites.
 */
static int ui_anchoPanelEfectivo(const UiOpcionesVisuales *opt) {
    int anchoTerm = (opt != NULL) ? opt->anchoTerminal : ui_obtenerAnchoTerminal();
    int anchoDisponible = anchoTerm - 4;

    if (opt != NULL && opt->modoCompacto && anchoDisponible > 60) {
        anchoDisponible = 60;
    }
    if (anchoDisponible < 48) {
        anchoDisponible = 48;
    }
    if (anchoDisponible > UI_ANCHO_MAXIMO_PANEL) {
        anchoDisponible = UI_ANCHO_MAXIMO_PANEL;
    }
    return anchoDisponible;
}

/**
 * @brief Imprime una linea horizontal de longitud dada usando trazos Unicode o ASCII.
 */
static void ui_imprimirTrazoHorizontal(const UiOpcionesVisuales *opt, int longitud, bool doble) {
    const char *trazo = "-";
    if (opt != NULL && opt->usarUnicode) {
        trazo = doble ? BOX_DH_UTF8 : BOX_H_UTF8;
    } else if (doble) {
        trazo = "=";
    }

    for (int i = 0; i < longitud; ++i) {
        fputs(trazo, stdout);
    }
}

/**
 * @brief Formatea una cantidad de segundos como reloj de mision `T+ HH:MM:SS`.
 */
static void ui_formatearTiempoMision(double segundos, bool prefijoMas, char *buffer, size_t tam) {
    if (buffer == NULL || tam == 0) {
        return;
    }
    if (segundos < 0.0) {
        segundos = -segundos;
    }
    long totalSeg = (long)(segundos + 0.5);
    long horas    = totalSeg / 3600L;
    long minutos  = (totalSeg % 3600L) / 60L;
    long segs     = totalSeg % 60L;

    if (horas > 0) {
        snprintf(buffer, tam, "T%c %02ld:%02ld:%02ld",
                 prefijoMas ? '+' : '-', horas, minutos, segs);
    } else {
        snprintf(buffer, tam, "T%c 00:%02ld:%02ld",
                 prefijoMas ? '+' : '-', minutos, segs);
    }
}

/**
 * @brief Traduce el enum `UiEstadoMisionGeneral` a texto legible en espanol.
 */
static const char *ui_nombreEstadoMision(UiEstadoMisionGeneral estado) {
    switch (estado) {
        case UI_MISION_STANDBY:          return "EN ESPERA";
        case UI_MISION_CONFIGURADA:      return "CONFIGURADA / LISTA";
        case UI_MISION_PREFLIGHT:        return "VERIFICACION PREVIA";
        case UI_MISION_CUENTA_REGRESIVA: return "SECUENCIA DE LANZAMIENTO";
        case UI_MISION_EN_VUELO:         return "EN VUELO / TELEMETRIA";
        case UI_MISION_COMPLETADA:       return "MISION COMPLETADA";
        case UI_MISION_ABORTADA:         return "MISION ABORTADA";
        case UI_MISION_FALLO_CRITICO:    return "FALLO CRITICO";
        default:                         return "EN ESPERA";
    }
}

/**
 * @brief Devuelve el color correspondiente al estado general de la mision.
 */
static const char *ui_colorEstadoMision(
    const UiOpcionesVisuales *opt,
    UiEstadoMisionGeneral estado
) {
    switch (estado) {
        case UI_MISION_COMPLETADA:
        case UI_MISION_CONFIGURADA:
            return ui_color(opt, COL_VERDE);
        case UI_MISION_EN_VUELO:
            return ui_color(opt, COL_CELESTE);
        case UI_MISION_PREFLIGHT:
        case UI_MISION_CUENTA_REGRESIVA:
        case UI_MISION_STANDBY:
            return ui_color(opt, COL_AMARILLO);
        case UI_MISION_ABORTADA:
        case UI_MISION_FALLO_CRITICO:
            return ui_color(opt, COL_ROJO);
        default:
            return ui_color(opt, COL_BLANCO_SUAVE);
    }
}

/**
 * @brief Traduce el enum `UiFaseVuelo` a su descripcion en espanol.
 */
static const char *ui_nombreFaseVuelo(UiFaseVuelo fase) {
    switch (fase) {
        case UI_FASE_STANDBY:           return "EN ESPERA (PLATAFORMA LC-39B)";
        case UI_FASE_PRELANZAMIENTO:    return "SECUENCIA PRE-LANZAMIENTO";
        case UI_FASE_1_ASCENSO_SRB:     return "FASE 1 - ASCENSO CON PROPULSORES SRB";
        case UI_FASE_SEPARACION_SRB:    return "SEPARACION DE PROPULSORES SRB";
        case UI_FASE_2_PROPULSION_CORE: return "FASE 2 - PROPULSION PRINCIPAL RS-25";
        case UI_FASE_CORTE_MECO:        return "CORTE DE MOTORES PRINCIPALES (MECO)";
        case UI_FASE_VUELO_BALISTICO:   return "ASCENSO INERCIAL / APOGEO BALISTICO";
        case UI_FASE_FINALIZADA:        return "MISION FINALIZADA NOMINALMENTE";
        case UI_FASE_ABORTADA:          return "SECUENCIA ABORTADA";
        default:                        return "NO ESPECIFICADA";
    }
}

/**
 * @brief Traduce el estado de los propulsores solidos SRB a espanol.
 */
static const char *ui_nombreEstadoSRB(UiEstadoSRB estado) {
    switch (estado) {
        case UI_SRB_ATTACHED:    return "ACOPLADOS (EN ESPERA)";
        case UI_SRB_ACTIVE_BURN: return "IGNICION ACTIVA (EMPUJE FASE 1)";
        case UI_SRB_SEPARATED:   return "SEPARADOS (DESACOPLADOS)";
        case UI_SRB_FAULT:       return "ANOMALIA EN PROPULSORES";
        default:                 return "DESCONOCIDO";
    }
}

/**
 * @brief Traduce el estado de los motores principales de la etapa central a espanol.
 */
static const char *ui_nombreEstadoCore(UiEstadoMotorCore estado) {
    switch (estado) {
        case UI_CORE_READY:         return "LISTO (4 MOTORES RS-25 EN ESPERA)";
        case UI_CORE_IGNITION:      return "SECUENCIA DE IGNICION";
        case UI_CORE_ACTIVE_THRUST: return "EMPUJE NOMINAL ACTIVO";
        case UI_CORE_MECO_CUTOFF:   return "APAGADO POR CORTE (MECO)";
        case UI_CORE_FAULT:         return "FALLO EN MOTOR PRINCIPAL";
        default:                    return "DESCONOCIDO";
    }
}

/**
 * @brief Devuelve una descripcion estandar en espanol para cada tipo de evento.
 */
static const char *ui_descripcionEventoPorDefecto(UiTipoEvento tipo) {
    switch (tipo) {
        case UI_EVENTO_INICIO_LANZAMIENTO:
            return "Inicio de secuencia automatica de lanzamiento";
        case UI_EVENTO_DESPEGUE:
            return "Ignicion SRB y despegue confirmado";
        case UI_EVENTO_ASCENSO_NOMINAL:
            return "Ascenso nominal Fase 1 con empuje combinado SLS";
        case UI_EVENTO_SEPARACION_SRB:
            return "Separacion de propulsores solidos SRB completada";
        case UI_EVENTO_ACTIVACION_FASE_2:
            return "Segunda fase de propulsion principal activa";
        case UI_EVENTO_CORTE_MOTORES:
            return "Altitud de corte alcanzada - Apagado de motores principales";
        case UI_EVENTO_FINALIZACION_MISION:
            return "Apogeo alcanzado y finalizacion nominal de la mision";
        case UI_EVENTO_FALLO_MISION:
            return "Condicion critica detectada - Aborto de mision";
        case UI_EVENTO_PERSONALIZADO:
        default:
            return "Evento de telemetria registrado";
    }
}

/* ==========================================================================
 * 4. FUNCIONES DE CONTROL DE PANTALLA Y PARAMETROS DE REFERENCIA
 * ========================================================================== */

void ui_limpiarPantalla(bool enSitio) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    if (opt.usarColoresAnsi) {
        if (enSitio) {
            /* Mueve el cursor a la fila 1, columna 1 sin borrar toda la pantalla
             * para evitar parpadeos al refrescar la telemetria en tiempo real. */
            fputs("\033[H", stdout);
        } else {
            /* Limpia la pantalla visible y lleva el cursor al inicio */
            fputs("\033[2J\033[H", stdout);
        }
    } else {
        /* Alternativa simple si la terminal no soporta secuencias ANSI */
        fputs("\n\n", stdout);
    }
    fflush(stdout);
}

UiParametrosMision ui_obtenerParametrosReferencia(void) {
    UiParametrosMision ref;
    ref.velocidadInicial    = UI_REF_VELOCIDAD_INICIAL;
    ref.altitudInicial      = UI_REF_ALTITUD_INICIAL;
    ref.aceleracionFase1    = UI_REF_ACELERACION_FASE1;
    ref.tiempoSeparacionSRB = UI_REF_TIEMPO_SEP_SRB;
    ref.aceleracionFase2    = UI_REF_ACELERACION_FASE2;
    ref.altitudCorte        = UI_REF_ALTITUD_CORTE;
    ref.gravedad            = UI_REF_GRAVEDAD;
    ref.personalizados      = false;
    return ref;
}

/* ==========================================================================
 * 5. COMPONENTES VISUALES REUTILIZABLES (SECCION 7)
 * ==========================================================================
 * Nota para el equipo: Pueden usar estas funciones en cualquier parte del
 * programa para mantener el mismo estilo visual.
 */

void ui_mostrarSeparador(void) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    int ancho = ui_anchoPanelEfectivo(&opt);

    printf("  %s", ui_color(&opt, COL_GRIS_OSCURO));
    ui_imprimirTrazoHorizontal(&opt, ancho, false);
    printf("%s\n", ui_color(&opt, ANSI_RESET));
}

void ui_mostrarSeparadorDoble(void) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    int ancho = ui_anchoPanelEfectivo(&opt);

    printf("  %s", ui_color(&opt, COL_AZUL_ELECTRICO));
    ui_imprimirTrazoHorizontal(&opt, ancho, true);
    printf("%s\n", ui_color(&opt, ANSI_RESET));
}

void ui_mostrarTitulo(const char *titulo) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *texto = (titulo != NULL) ? titulo : UI_NOMBRE_SISTEMA;
    const char *simbolo = opt.usarUnicode ? SYM_CURSOR_UTF8 : ">";

    printf("\n  %s%s %s%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO),
           simbolo,
           texto,
           ui_color(&opt, ANSI_RESET));
    ui_mostrarSeparador();
}

void ui_mostrarSubtitulo(const char *subtitulo) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    if (subtitulo == NULL) {
        return;
    }
    printf("  %s%s%s\n",
           ui_color(&opt, COL_GRIS),
           subtitulo,
           ui_color(&opt, ANSI_RESET));
}

/**
 * @brief Dibuja el recuadro superior de cada pantalla con el nombre de la
 *        seccion a la izquierda y el estado general de la mision a la derecha.
 */
void ui_mostrarEncabezado(const char *seccion, UiEstadoMisionGeneral estadoSistema) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    int ancho = ui_anchoPanelEfectivo(&opt);
    const char *nombreSeccion = (seccion != NULL) ? seccion : "CONTROL DE MISION";
    const char *estadoTxt = ui_nombreEstadoMision(estadoSistema);
    const char *tl = opt.usarUnicode ? BOX_TL_UTF8 : "+";
    const char *tr = opt.usarUnicode ? BOX_TR_UTF8 : "+";
    const char *bl = opt.usarUnicode ? BOX_BL_UTF8 : "+";
    const char *br = opt.usarUnicode ? BOX_BR_UTF8 : "+";
    const char *v  = opt.usarUnicode ? BOX_V_UTF8  : "|";
    const char *punto = opt.usarUnicode ? SYM_CIRCULO_UTF8 : "*";

    /* Calculamos el espacio interior para que el borde derecho quede alineado */
    int anchoInterior = ancho - 2;
    int lenEstado = (int)strlen(estadoTxt);
    int anchoSeccion = anchoInterior - 15 - 3 - lenEstado - 1;
    if (anchoSeccion < 10) {
        anchoSeccion = 10;
    }

    /* Borde superior redondeado */
    printf("\n  %s%s", ui_color(&opt, COL_GRIS_OSCURO), tl);
    ui_imprimirTrazoHorizontal(&opt, anchoInterior, false);
    printf("%s%s\n", tr, ui_color(&opt, ANSI_RESET));

    /* Contenido del encabezado */
    printf("  %s%s%s %sARTEMIS CLI%s %s/%s %s%-*.*s%s %s%s %s%s %s%s%s\n",
           ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), anchoSeccion, anchoSeccion, nombreSeccion, ui_color(&opt, ANSI_RESET),
           ui_colorEstadoMision(&opt, estadoSistema), punto,
           estadoTxt, ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET));

    /* Borde inferior redondeado */
    printf("  %s%s", ui_color(&opt, COL_GRIS_OSCURO), bl);
    ui_imprimirTrazoHorizontal(&opt, anchoInterior, false);
    printf("%s%s\n", br, ui_color(&opt, ANSI_RESET));
}

void ui_mostrarTextoDestacado(const char *etiqueta, const char *valor) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *lbl = (etiqueta != NULL) ? etiqueta : "";
    const char *val = (valor != NULL) ? valor : "--";

    printf("  %s%-25s%s %s%s%s\n",
           ui_color(&opt, COL_GRIS), lbl, ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_CELESTE), val, ui_color(&opt, ANSI_RESET));
}

void ui_mostrarError(const char *mensaje) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *msg = (mensaje != NULL) ? mensaje : "Error de operacion no especificado.";

    printf("  %s[ERROR]  %s %s%s%s\n",
           ui_color(&opt, COL_ROJO), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), msg, ui_color(&opt, ANSI_RESET));
}

void ui_mostrarExito(const char *mensaje) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *msg = (mensaje != NULL) ? mensaje : "Operacion completada correctamente.";

    printf("  %s[EXITO]  %s %s%s%s\n",
           ui_color(&opt, COL_VERDE), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), msg, ui_color(&opt, ANSI_RESET));
}

void ui_mostrarAdvertencia(const char *mensaje) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *msg = (mensaje != NULL) ? mensaje : "Advertencia operativa del sistema.";

    printf("  %s[AVISO]  %s %s%s%s\n",
           ui_color(&opt, COL_AMARILLO), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), msg, ui_color(&opt, ANSI_RESET));
}

void ui_mostrarInfo(const char *mensaje) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *msg = (mensaje != NULL) ? mensaje : "";

    printf("  %s[INFO]   %s %s%s%s\n",
           ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_GRIS), msg, ui_color(&opt, ANSI_RESET));
}

void ui_mostrarProgresoEtiquetado(const char *etiqueta, double porcentaje, int anchoBarra) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    double pct = porcentaje;

    /* Acotamos el porcentaje al intervalo [0.0, 100.0] por seguridad */
    if (pct < 0.0) {
        pct = 0.0;
    } else if (pct > 100.0) {
        pct = 100.0;
    }

    if (anchoBarra < 10) {
        anchoBarra = 10;
    } else if (anchoBarra > 50) {
        anchoBarra = 50;
    }

    int llenos = (int)((pct / 100.0) * (double)anchoBarra + 0.5);
    if (llenos > anchoBarra) {
        llenos = anchoBarra;
    }

    if (etiqueta != NULL && etiqueta[0] != '\0') {
        printf("  %s%s%s\n", ui_color(&opt, COL_GRIS), etiqueta, ui_color(&opt, ANSI_RESET));
    }

    printf("  %s[%s", ui_color(&opt, COL_GRIS), ui_color(&opt, COL_CELESTE));
    for (int i = 0; i < anchoBarra; ++i) {
        if (i < llenos) {
            fputs(opt.usarUnicode ? SYM_BLOQUE_LLENO : "#", stdout);
        } else {
            fputs(ui_color(&opt, COL_GRIS_OSCURO), stdout);
            fputs(opt.usarUnicode ? SYM_BLOQUE_VACIO : "-", stdout);
        }
    }
    printf("%s] %s%5.1f%%%s\n",
           ui_color(&opt, COL_GRIS),
           ui_color(&opt, COL_BLANCO_SUAVE),
           pct,
           ui_color(&opt, ANSI_RESET));
}

void ui_mostrarProgreso(double porcentaje) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    int anchoBarra = opt.modoCompacto ? 24 : 34;
    ui_mostrarProgresoEtiquetado(NULL, porcentaje, anchoBarra);
}

void ui_mostrarListaOpciones(
    const char * const *opciones,
    const char * const *descripciones,
    size_t cantidad,
    size_t indiceSeleccionado
) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    if (opciones == NULL || cantidad == 0) {
        return;
    }

    const char *cursor = opt.usarUnicode ? SYM_CURSOR_UTF8 : ">";

    for (size_t i = 0; i < cantidad; ++i) {
        bool activa = (i == indiceSeleccionado);
        const char *texto = (opciones[i] != NULL) ? opciones[i] : "";

        if (activa) {
            printf("  %s%s%s %s[%zu]%s %s%-28s%s",
                   ui_color(&opt, COL_AZUL_ELECTRICO), cursor, ui_color(&opt, ANSI_RESET),
                   ui_color(&opt, COL_CELESTE), i + 1, ui_color(&opt, ANSI_RESET),
                   ui_color(&opt, COL_BLANCO_SUAVE), texto, ui_color(&opt, ANSI_RESET));
        } else {
            printf("    %s[%zu]%s %s%-28s%s",
                   ui_color(&opt, COL_GRIS_OSCURO), i + 1, ui_color(&opt, ANSI_RESET),
                   ui_color(&opt, COL_GRIS), texto, ui_color(&opt, ANSI_RESET));
        }

        if (!opt.modoCompacto && descripciones != NULL && descripciones[i] != NULL) {
            printf(" %s%s%s",
                   activa ? ui_color(&opt, COL_CELESTE_SUAVE) : ui_color(&opt, COL_GRIS_OSCURO),
                   descripciones[i],
                   ui_color(&opt, ANSI_RESET));
        }
        fputc('\n', stdout);
    }
}

void ui_mostrarTabla(
    const char * const *encabezados,
    const int *anchosColumnas,
    size_t numColumnas,
    const char * const *celdasFilas,
    size_t numFilas
) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    if (encabezados == NULL || anchosColumnas == NULL || numColumnas == 0) {
        return;
    }

    const char *sepV = opt.usarUnicode ? BOX_V_UTF8 : "|";

    /* Imprime la fila de encabezados de la tabla */
    printf("  ");
    for (size_t col = 0; col < numColumnas; ++col) {
        int ancho = anchosColumnas[col] > 0 ? anchosColumnas[col] : 12;
        printf("%s%-*s%s",
               ui_color(&opt, COL_AZUL_ELECTRICO),
               ancho,
               encabezados[col] != NULL ? encabezados[col] : "",
               ui_color(&opt, ANSI_RESET));
        if (col + 1 < numColumnas) {
            printf(" %s%s%s ", ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET));
        }
    }
    fputc('\n', stdout);

    /* Linea separadora debajo de los encabezados */
    printf("  %s", ui_color(&opt, COL_GRIS_OSCURO));
    for (size_t col = 0; col < numColumnas; ++col) {
        int ancho = anchosColumnas[col] > 0 ? anchosColumnas[col] : 12;
        ui_imprimirTrazoHorizontal(&opt, ancho, false);
        if (col + 1 < numColumnas) {
            ui_imprimirTrazoHorizontal(&opt, 3, false);
        }
    }
    printf("%s\n", ui_color(&opt, ANSI_RESET));

    /* Imprime cada fila de datos */
    if (celdasFilas == NULL || numFilas == 0) {
        return;
    }

    for (size_t fila = 0; fila < numFilas; ++fila) {
        printf("  ");
        for (size_t col = 0; col < numColumnas; ++col) {
            int ancho = anchosColumnas[col] > 0 ? anchosColumnas[col] : 12;
            const char *celda = celdasFilas[fila * numColumnas + col];
            const char *colorCelda = (col == 0)
                ? ui_color(&opt, COL_BLANCO_SUAVE)
                : ui_color(&opt, COL_CELESTE);

            printf("%s%-*s%s",
                   colorCelda,
                   ancho,
                   celda != NULL ? celda : "--",
                   ui_color(&opt, ANSI_RESET));
            if (col + 1 < numColumnas) {
                printf(" %s%s%s ",
                       ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET));
            }
        }
        fputc('\n', stdout);
    }
}

void ui_mostrarEstadoSistema(
    const char *nombreSistema,
    UiEstadoSistema estado,
    const char *detalle
) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *etiqueta = "[INFO]  ";
    const char *colorEstado = ui_color(&opt, COL_CELESTE);

    /* Seleccionamos la etiqueta en espanol y su color segun el estado */
    switch (estado) {
        case UI_ESTADO_OK:
            etiqueta    = "[OK]    ";
            colorEstado = ui_color(&opt, COL_VERDE);
            break;
        case UI_ESTADO_WAIT:
            etiqueta    = "[ESPERA]";
            colorEstado = ui_color(&opt, COL_AMARILLO);
            break;
        case UI_ESTADO_WARN:
            etiqueta    = "[AVISO] ";
            colorEstado = ui_color(&opt, COL_AMARILLO);
            break;
        case UI_ESTADO_FAIL:
            etiqueta    = "[FALLO] ";
            colorEstado = ui_color(&opt, COL_ROJO);
            break;
        case UI_ESTADO_INFO:
        default:
            etiqueta    = "[INFO]  ";
            colorEstado = ui_color(&opt, COL_CELESTE);
            break;
    }

    printf("  %s%s%s  %s%-35s%s",
           colorEstado, etiqueta, ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE),
           nombreSistema != NULL ? nombreSistema : "Subsistema",
           ui_color(&opt, ANSI_RESET));

    if (detalle != NULL && detalle[0] != '\0' && !opt.modoCompacto) {
        printf(" %s%s%s", ui_color(&opt, COL_GRIS), detalle, ui_color(&opt, ANSI_RESET));
    }
    fputc('\n', stdout);
}

void ui_mostrarEvento(const UiEventoMision *evento) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    if (evento == NULL) {
        return;
    }

    const char *tagNivel = "[EVENTO]";
    const char *colorNivel = ui_color(&opt, COL_AZUL_ELECTRICO);

    switch (evento->nivel) {
        case UI_NIVEL_EVENTO:
            tagNivel   = "[EVENTO]";
            colorNivel = ui_color(&opt, COL_AZUL_ELECTRICO);
            break;
        case UI_NIVEL_INFO:
            tagNivel   = "[INFO]  ";
            colorNivel = ui_color(&opt, COL_CELESTE);
            break;
        case UI_NIVEL_OK:
            tagNivel   = "[OK]    ";
            colorNivel = ui_color(&opt, COL_VERDE);
            break;
        case UI_NIVEL_WARN:
            tagNivel   = "[AVISO] ";
            colorNivel = ui_color(&opt, COL_AMARILLO);
            break;
        case UI_NIVEL_CRITICAL:
            tagNivel   = "[FALLO] ";
            colorNivel = ui_color(&opt, COL_ROJO);
            break;
        default:
            break;
    }

    const char *texto = (evento->descripcion != NULL && evento->descripcion[0] != '\0')
        ? evento->descripcion
        : ui_descripcionEventoPorDefecto(evento->tipo);

    printf("  %sT+ %4.0fs%s  %s%s%s  %s%s%s\n",
           ui_color(&opt, COL_CELESTE), evento->tiempoSegundos, ui_color(&opt, ANSI_RESET),
           colorNivel, tagNivel, ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), texto, ui_color(&opt, ANSI_RESET));
}

void ui_mostrarListaEventos(const UiEventoMision *eventos, size_t cantidad) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    printf("  %sREGISTRO DE EVENTOS DE MISION%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO),
           ui_color(&opt, ANSI_RESET));

    if (eventos == NULL || cantidad == 0) {
        printf("  %sNo se registraron eventos de mision.%s\n",
               ui_color(&opt, COL_GRIS),
               ui_color(&opt, ANSI_RESET));
        return;
    }

    for (size_t i = 0; i < cantidad; ++i) {
        ui_mostrarEvento(&eventos[i]);
    }
}

void ui_mostrarAyudaNavegacion(const char *contextoControles) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *flechas = opt.usarUnicode
        ? (SYM_FLECHA_ARR_UTF8 SYM_FLECHA_ABA_UTF8)
        : "Arriba/Abajo";
    const char *sep = opt.usarUnicode ? SYM_PUNTO_UTF8 : "|";

    ui_mostrarSeparador();
    if (contextoControles != NULL && contextoControles[0] != '\0') {
        printf("  %s%s%s\n",
               ui_color(&opt, COL_GRIS),
               contextoControles,
               ui_color(&opt, ANSI_RESET));
    } else {
        printf("  %s%s%s %sNavegar%s   %s%s%s   %s1-7%s %sAcceso directo%s   %s%s%s   "
               "%sEnter%s %sSeleccionar%s   %s%s%s   %sH/?%s %sAyuda%s   %s%s%s   %sQ%s %sSalir%s\n",
               ui_color(&opt, COL_CELESTE), flechas, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sep, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sep, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sep, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sep, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
    }
}

void ui_mostrarAyudaContextual(void) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    ui_mostrarEncabezado("AYUDA RAPIDA DE NAVEGACION", UI_MISION_STANDBY);

    printf("\n  %sCONTROLES DE TECLADO DISPONIBLES%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));

    ui_mostrarTextoDestacado("Flecha Arriba / K", "Desplazar seleccion a la opcion anterior");
    ui_mostrarTextoDestacado("Flecha Abajo  / J", "Desplazar seleccion a la opcion siguiente");
    ui_mostrarTextoDestacado("Teclas 1 a 7",      "Seleccionar directamente el numero de opcion");
    ui_mostrarTextoDestacado("Enter (Intro)",     "Ingresar a la seccion resaltada");
    ui_mostrarTextoDestacado("Escape / Q",        "Regresar al menu principal o salir del programa");
    ui_mostrarTextoDestacado("H o ?",             "Mostrar este panel de ayuda contextual");

    printf("\n  %sMODO DE COMPATIBILIDAD UNIVERSAL%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sSi la terminal no soporta lectura inmediata de teclas, escriba el numero\n"
           "  de la opcion deseada (1-7) o la letra del comando (Q, H) y presione Enter.%s\n\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
}

/* ==========================================================================
 * 6. PANTALLAS DEL SIMULADOR (SECCION 5: A - L)
 * ========================================================================== */

/**
 * @brief Dibuja el logotipo de ARTEMIS CLI con degradado en tonos azules/celestes,
 *        o un recuadro compacto cuando la ventana de la consola es angosta.
 */
static void ui_renderizarLogo(const UiOpcionesVisuales *opt) {
    if (opt->modoCompacto) {
        const char *tl = opt->usarUnicode ? BOX_TL_UTF8 : "+";
        const char *tr = opt->usarUnicode ? BOX_TR_UTF8 : "+";
        const char *bl = opt->usarUnicode ? BOX_BL_UTF8 : "+";
        const char *br = opt->usarUnicode ? BOX_BR_UTF8 : "+";
        const char *v  = opt->usarUnicode ? BOX_V_UTF8  : "|";

        printf("\n  %s%s", ui_color(opt, COL_AZUL_ELECTRICO), tl);
        ui_imprimirTrazoHorizontal(opt, 46, false);
        printf("%s%s\n", tr, ui_color(opt, ANSI_RESET));

        printf("  %s%s%s  %sA R T E M I S   C L I%s             %sv%-8s%s %s%s%s\n",
               ui_color(opt, COL_AZUL_ELECTRICO), v, ui_color(opt, ANSI_RESET),
               ui_color(opt, COL_BLANCO_SUAVE), ui_color(opt, ANSI_RESET),
               ui_color(opt, COL_CELESTE), UI_VERSION_SISTEMA, ui_color(opt, ANSI_RESET),
               ui_color(opt, COL_AZUL_ELECTRICO), v, ui_color(opt, ANSI_RESET));

        printf("  %s%s%s  %sControl de Mision SLS / Simulacion de Vuelo%s %s%s%s\n",
               ui_color(opt, COL_AZUL_ELECTRICO), v, ui_color(opt, ANSI_RESET),
               ui_color(opt, COL_GRIS), ui_color(opt, ANSI_RESET),
               ui_color(opt, COL_AZUL_ELECTRICO), v, ui_color(opt, ANSI_RESET));

        printf("  %s%s", ui_color(opt, COL_AZUL_ELECTRICO), bl);
        ui_imprimirTrazoHorizontal(opt, 46, false);
        printf("%s%s\n", br, ui_color(opt, ANSI_RESET));
        return;
    }

    /* Degradado de 6 tonos de azul electrico a celeste sobre el logo principal */
    static const char * const coloresGradiente[6] = {
        COL_AZUL_PROFUNDO,
        COL_AZUL_ELECTRICO,
        COL_AZUL_ELECTRICO,
        COL_AZUL_MEDIO,
        COL_CELESTE,
        COL_CELESTE_SUAVE
    };

    fputc('\n', stdout);
    for (int i = 0; i < 6; ++i) {
        const char *linea = opt->usarUnicode ? K_LOGO_ARTEMIS_UTF8[i] : K_LOGO_ARTEMIS_ASCII[i];
        printf("%s%s%s\n",
               ui_color(opt, coloresGradiente[i]),
               linea,
               ui_color(opt, ANSI_RESET));
    }

    printf("\n  %sCONTROL DE MISION SLS%s   %s%s%s   %sVersion %s%s\n",
           ui_color(opt, COL_BLANCO_SUAVE), ui_color(opt, ANSI_RESET),
           ui_color(opt, COL_GRIS_OSCURO),
           opt->usarUnicode ? SYM_PUNTO_UTF8 : "|",
           ui_color(opt, ANSI_RESET),
           ui_color(opt, COL_CELESTE), UI_VERSION_SISTEMA, ui_color(opt, ANSI_RESET));
    printf("  %s%s%s\n",
           ui_color(opt, COL_GRIS),
           UI_SUBTITULO_SISTEMA,
           ui_color(opt, ANSI_RESET));
}

void ui_mostrarBienvenida(void) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    ui_mostrarPantallaPrincipal(UI_MENU_INICIAR_MISION, UI_MISION_STANDBY, &opt);
}

void ui_mostrarMenuPrincipal(UiOpcionMenu opcionSeleccionada) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    size_t idx = 0;
    if (opcionSeleccionada >= UI_MENU_INICIAR_MISION &&
        opcionSeleccionada <= UI_MENU_TOTAL_OPCIONES) {
        idx = (size_t)(opcionSeleccionada - 1);
    }

    const char *cursor = opt.usarUnicode ? SYM_CURSOR_UTF8 : ">";

    for (size_t i = 0; i < (size_t)UI_MENU_TOTAL_OPCIONES; ++i) {
        bool activa = (i == idx);
        if (activa) {
            printf("  %s%s%s %s%zu. %-25s%s",
                   ui_color(&opt, COL_AZUL_ELECTRICO), cursor, ui_color(&opt, ANSI_RESET),
                   ui_color(&opt, COL_BLANCO_SUAVE), i + 1, K_MENU_OPCIONES[i],
                   ui_color(&opt, ANSI_RESET));
            if (!opt.modoCompacto) {
                printf("  %s[%s]%s",
                       ui_color(&opt, COL_CELESTE),
                       K_MENU_ETIQUETAS_SEC[i],
                       ui_color(&opt, ANSI_RESET));
            }
            fputc('\n', stdout);
        } else {
            printf("    %s%zu. %-25s%s",
                   ui_color(&opt, COL_GRIS), i + 1, K_MENU_OPCIONES[i],
                   ui_color(&opt, ANSI_RESET));
            if (!opt.modoCompacto) {
                printf("  %s%s%s",
                       ui_color(&opt, COL_GRIS_OSCURO),
                       K_MENU_ETIQUETAS_SEC[i],
                       ui_color(&opt, ANSI_RESET));
            }
            fputc('\n', stdout);
        }
    }

    /* Muestra la descripcion de la opcion actualmente resaltada */
    printf("\n  %s%s%s %s%s%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO),
           opt.usarUnicode ? SYM_PUNTO_UTF8 : "*",
           ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_CELESTE_SUAVE),
           K_MENU_DESCRIPCIONES[idx],
           ui_color(&opt, ANSI_RESET));
}

void ui_mostrarPantallaPrincipal(
    UiOpcionMenu opcionSeleccionada,
    UiEstadoMisionGeneral estadoActual,
    const UiOpcionesVisuales *opciones
) {
    UiOpcionesVisuales optDetectada = ui_obtenerOpcionesVisuales();
    const UiOpcionesVisuales *opt = (opciones != NULL) ? opciones : &optDetectada;
    const char *punto = opt->usarUnicode ? SYM_CIRCULO_UTF8 : "*";

    ui_renderizarLogo(opt);
    ui_mostrarSeparador();

    /* Panel de resumen de estado del sistema en la pantalla de bienvenida */
    printf("  %s%-14s%s %s%-26s%s",
           ui_color(opt, COL_GRIS), "Mision", ui_color(opt, ANSI_RESET),
           ui_color(opt, COL_BLANCO_SUAVE), "ARTEMIS / SLS BLOQUE 1", ui_color(opt, ANSI_RESET));
    if (!opt->modoCompacto) {
        printf("  %s%-14s%s %s%s%s",
               ui_color(opt, COL_GRIS), "Entorno", ui_color(opt, ANSI_RESET),
               ui_color(opt, COL_CELESTE), "SIMULACION (CONSOLA C11)", ui_color(opt, ANSI_RESET));
    }
    fputc('\n', stdout);

    printf("  %s%-14s%s %s%-26s%s",
           ui_color(opt, COL_GRIS), "Escenario", ui_color(opt, ANSI_RESET),
           ui_color(opt, COL_CELESTE), UI_ESCENARIO_CORTO, ui_color(opt, ANSI_RESET));
    if (!opt->modoCompacto) {
        printf("  %s%-14s%s %s%s %s%s",
               ui_color(opt, COL_GRIS), "Estado", ui_color(opt, ANSI_RESET),
               ui_colorEstadoMision(opt, estadoActual), punto,
               ui_nombreEstadoMision(estadoActual),
               ui_color(opt, ANSI_RESET));
    } else {
        printf("\n  %s%-14s%s %s%s %s%s",
               ui_color(opt, COL_GRIS), "Estado", ui_color(opt, ANSI_RESET),
               ui_colorEstadoMision(opt, estadoActual), punto,
               ui_nombreEstadoMision(estadoActual),
               ui_color(opt, ANSI_RESET));
    }
    fputc('\n', stdout);

    ui_mostrarSeparador();
    fputc('\n', stdout);

    ui_mostrarMenuPrincipal(opcionSeleccionada);
    fputc('\n', stdout);
    ui_mostrarAyudaNavegacion(NULL);
}

/* --------------------------------------------------------------------------
 * SECCION C: CONFIGURACION DE MISION
 * Nota para el companero de `validaciones.c` y `simulador.c`:
 * Esta funcion recibe los parametros actuales y dibuja la tabla. Si pasan
 * `parametroResaltado` entre 0 y 6, marca con el cursor `❯` que fila se esta
 * editando en ese momento.
 * -------------------------------------------------------------------------- */
void ui_mostrarConfiguracionMision(
    const UiParametrosMision *parametros,
    int parametroResaltado
) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    UiParametrosMision ref = ui_obtenerParametrosReferencia();
    const UiParametrosMision *p = (parametros != NULL) ? parametros : &ref;

    ui_mostrarEncabezado(
        "CONFIGURACION DE PARAMETROS DE MISION",
        p->personalizados ? UI_MISION_CONFIGURADA : UI_MISION_STANDBY
    );

    printf("\n  %sPARAMETROS FISICOS DEL ESCENARIO 1 (ASCENSO Y PROPULSION SLS)%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sValores de referencia para las ecuaciones cinematicas (MRUV por fases).%s\n\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));

    static const char * const nombresParam[7] = {
        "Velocidad inicial (v0)",
        "Altitud inicial (h0)",
        "Aceleracion fase 1 (a1)",
        "Tiempo de separacion SRB (t_SRB)",
        "Aceleracion fase 2 (a2)",
        "Altitud de corte (h_corte)",
        "Gravedad terrestre (g)"
    };
    static const char * const unidadesParam[7] = {
        "m/s", "m", "m/s^2", "s", "m/s^2", "m", "m/s^2"
    };
    static const char * const notasParam[7] = {
        "Reposo en plataforma de lanzamiento",
        "Nivel de referencia de despegue",
        "Empuje combinado Etapa Central + 2 SRB",
        "Desacople de propulsores solidos",
        "Propulsion principal (4 motores RS-25)",
        "Apagado de motores principales (MECO)",
        "Aceleracion gravitatoria de referencia"
    };

    double valores[7];
    valores[0] = p->velocidadInicial;
    valores[1] = p->altitudInicial;
    valores[2] = p->aceleracionFase1;
    valores[3] = p->tiempoSeparacionSRB;
    valores[4] = p->aceleracionFase2;
    valores[5] = p->altitudCorte;
    valores[6] = p->gravedad;

    const char *cursor = opt.usarUnicode ? SYM_CURSOR_UTF8 : ">";
    const char *sepV   = opt.usarUnicode ? BOX_V_UTF8 : "|";

    printf("  %s%-3s %-33s %s %-14s %s %s%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), "#", "PARAMETRO FISICO", sepV, "VALOR ACTUAL", sepV,
           opt.modoCompacto ? "" : "DESCRIPCION OPERATIVA",
           ui_color(&opt, ANSI_RESET));
    ui_mostrarSeparador();

    for (int i = 0; i < 7; ++i) {
        bool activo = (i == parametroResaltado);
        char valFormateado[32];
        snprintf(valFormateado, sizeof(valFormateado), "%.2f %s", valores[i], unidadesParam[i]);

        printf("  %s%s%d%s  %s%-33s%s %s%s%s %s%-14s%s",
               activo ? ui_color(&opt, COL_AZUL_ELECTRICO) : ui_color(&opt, COL_GRIS_OSCURO),
               activo ? cursor : " ",
               i + 1,
               ui_color(&opt, ANSI_RESET),
               activo ? ui_color(&opt, COL_BLANCO_SUAVE) : ui_color(&opt, COL_GRIS),
               nombresParam[i],
               ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), valFormateado, ui_color(&opt, ANSI_RESET));

        if (!opt.modoCompacto) {
            printf(" %s%s%s %s%s%s",
                   ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET),
                   ui_color(&opt, COL_GRIS), notasParam[i], ui_color(&opt, ANSI_RESET));
        }
        fputc('\n', stdout);
    }

    ui_mostrarSeparador();
    printf("\n  %sESTADO DEL COMPONENTE DE ENTRADA:%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    ui_mostrarInfo("Perfil activo: Escenario 1 Predeterminado (valores nominales de referencia).");
    ui_mostrarAdvertencia(
        "La lectura y validacion de datos ingresados se realiza a traves de validaciones.c."
    );
}

/* --------------------------------------------------------------------------
 * SECCION D: PREPARACION DEL LANZAMIENTO (LISTA DE VERIFICACION PRE-VUELO)
 * -------------------------------------------------------------------------- */
void ui_mostrarPreparacionLanzamiento(
    const UiItemChecklist *items,
    size_t cantidad,
    bool esDemostracion
) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();

    /* Plantilla por defecto en espanol si `simulador.c` pasa `items = NULL` */
    static const UiItemChecklist checklistPorDefecto[6] = {
        {
            "Sistema de propulsion principal",
            "4 motores RS-25",
            UI_ESTADO_OK,
            "Presion criogenica LH2/LOX nominal"
        },
        {
            "Verificacion de trayectoria",
            "Perfil de ascenso",
            UI_ESTADO_OK,
            "Perfil de ascenso cargado (h_corte = 160.000 m)"
        },
        {
            "Propulsores de cohete solido (SRB)",
            "2 propulsores laterales",
            UI_ESTADO_OK,
            "Separacion programada en T+ 120 s"
        },
        {
            "Computadora de vuelo",
            "Avionica e inercial",
            UI_ESTADO_OK,
            "Guiado inercial y cronometro sincronizados"
        },
        {
            "Conexion de telemetria",
            "Enlace de control",
            UI_ESTADO_OK,
            "Enlace de telemetria activo sin perdida de paquetes"
        },
        {
            "Autorizacion de lanzamiento",
            "Director de vuelo",
            UI_ESTADO_WAIT,
            "Aguardando confirmacion de secuencia de lanzamiento"
        }
    };

    const UiItemChecklist *lista = (items != NULL && cantidad > 0) ? items : checklistPorDefecto;
    size_t total = (items != NULL && cantidad > 0) ? cantidad : 6;

    ui_mostrarEncabezado("PREPARACION DEL LANZAMIENTO / VERIFICACION PREVIA", UI_MISION_PREFLIGHT);

    if (esDemostracion) {
        printf("\n");
        ui_mostrarAdvertencia(
            "MODO DEMOSTRACION VISUAL: Estados ilustrativos (sin verificacion fisica activa)."
        );
    }

    printf("\n  %sLISTA DE VERIFICACION DE SISTEMAS (PRE-VUELO)%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));

    size_t conteoOk   = 0;
    size_t conteoFail = 0;
    size_t conteoWait = 0;

    for (size_t i = 0; i < total; ++i) {
        ui_mostrarEstadoSistema(lista[i].sistema, lista[i].estado, lista[i].detalle);
        if (lista[i].estado == UI_ESTADO_OK) {
            ++conteoOk;
        } else if (lista[i].estado == UI_ESTADO_FAIL) {
            ++conteoFail;
        } else {
            ++conteoWait;
        }
    }

    printf("\n");
    ui_mostrarSeparador();

    if (conteoFail > 0) {
        ui_mostrarError("Anomalia detectada en subsistemas. Secuencia retenida.");
    } else if (conteoWait > 0) {
        ui_mostrarInfo("Todos los sistemas listos para la simulacion.");
    } else if (conteoOk == total) {
        ui_mostrarExito("AUTORIZACION CONFIRMADA: Todos los subsistemas verificados nominalmente.");
    }
}

/* --------------------------------------------------------------------------
 * SECCION E: CUENTA REGRESIVA (SECUENCIA DE LANZAMIENTO)
 * -------------------------------------------------------------------------- */
void ui_mostrarCuentaRegresiva(
    double segundosRestantes,
    const char *mensajeEstado,
    bool esDemostracion
) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    char relojStr[32];
    double segPos = (segundosRestantes < 0.0) ? -segundosRestantes : segundosRestantes;
    long totalSeg = (long)(segPos + 0.5);
    long minutos  = totalSeg / 60L;
    long segs     = totalSeg % 60L;
    snprintf(relojStr, sizeof(relojStr), "T - %02ld:%02ld", minutos, segs);

    ui_mostrarEncabezado("SECUENCIA DE LANZAMIENTO / CUENTA REGRESIVA", UI_MISION_CUENTA_REGRESIVA);

    if (esDemostracion) {
        printf("\n");
        ui_mostrarAdvertencia(
            "VISTA DEMOSTRATIVA: El tiempo restante se recibe por parametro sin bloquear el programa."
        );
    }

    const char *tl = opt.usarUnicode ? BOX_TL_UTF8 : "+";
    const char *tr = opt.usarUnicode ? BOX_TR_UTF8 : "+";
    const char *bl = opt.usarUnicode ? BOX_BL_UTF8 : "+";
    const char *br = opt.usarUnicode ? BOX_BR_UTF8 : "+";
    const char *v  = opt.usarUnicode ? BOX_V_UTF8  : "|";

    printf("\n  %sSECUENCIA DE LANZAMIENTO%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));

    printf("      %s%s", ui_color(&opt, COL_AZUL_ELECTRICO), tl);
    ui_imprimirTrazoHorizontal(&opt, 34, false);
    printf("%s%s\n", tr, ui_color(&opt, ANSI_RESET));

    printf("      %s%s%s            %s%-12s%s          %s%s%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), v, ui_color(&opt, ANSI_RESET),
           (segundosRestantes <= 3.0) ? ui_color(&opt, COL_AMARILLO) : ui_color(&opt, COL_CELESTE),
           relojStr,
           ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_AZUL_ELECTRICO), v, ui_color(&opt, ANSI_RESET));

    printf("      %s%s", ui_color(&opt, COL_AZUL_ELECTRICO), bl);
    ui_imprimirTrazoHorizontal(&opt, 34, false);
    printf("%s%s\n\n", br, ui_color(&opt, ANSI_RESET));

    double pctSecuencia = 0.0;
    if (segundosRestantes <= 0.0) {
        pctSecuencia = 100.0;
    } else if (segundosRestantes <= 10.0) {
        pctSecuencia = (10.0 - segundosRestantes) * 10.0;
    }
    ui_mostrarProgresoEtiquetado("PROGRESO DE IGNICION TERMINAL (T-10s -> T-00s)", pctSecuencia, 34);

    printf("\n  %s%s%s\n",
           ui_color(&opt, COL_BLANCO_SUAVE),
           (mensajeEstado != NULL && mensajeEstado[0] != '\0')
               ? mensajeEstado
               : "Iniciando secuencia de lanzamiento...",
           ui_color(&opt, ANSI_RESET));
}

/* --------------------------------------------------------------------------
 * SECCION F Y G: TELEMETRIA EN TIEMPO REAL Y REGISTRO DE EVENTOS
 * Nota para el companero de `simulador.c`:
 * Llamar a esta funcion en cada paso de tiempo de la simulacion pasando los
 * valores calculados por `fisica.c` dentro de la estructura `UiTelemetria`.
 * -------------------------------------------------------------------------- */
void ui_mostrarTelemetria(
    const UiTelemetria *telemetria,
    const UiEventoMision *eventos,
    size_t cantidadEventos,
    bool actualizarEnSitio
) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    ui_limpiarPantalla(actualizarEnSitio);

    bool hayDatos = (telemetria != NULL && telemetria->datosDisponibles);
    UiEstadoMisionGeneral estadoCabecera = hayDatos ? UI_MISION_EN_VUELO : UI_MISION_STANDBY;

    ui_mostrarEncabezado("TELEMETRIA EN TIEMPO REAL", estadoCabecera);

    if (telemetria != NULL && telemetria->esDemostracion) {
        printf("\n");
        ui_mostrarAdvertencia(
            "MODO DEMOSTRACION VISUAL: Instantanea ilustrativa recibida por parametro (no calculada en interfaz.c)."
        );
    }

    char tiempoStr[32];
    char altitudStr[48];
    char velocidadStr[48];
    char aceleracionStr[48];
    double progreso = 0.0;
    const char *faseStr = "EN ESPERA";
    const char *srbStr  = "ACOPLADOS";
    const char *coreStr = "LISTO";

    if (hayDatos) {
        ui_formatearTiempoMision(telemetria->tiempoTranscurrido, true, tiempoStr, sizeof(tiempoStr));
        snprintf(altitudStr, sizeof(altitudStr), "%.2f m  (%.2f km)",
                 telemetria->altitud, telemetria->altitud / 1000.0);
        snprintf(velocidadStr, sizeof(velocidadStr), "%.2f m/s", telemetria->velocidad);
        snprintf(aceleracionStr, sizeof(aceleracionStr), "%.2f m/s^2", telemetria->aceleracion);
        progreso = telemetria->progresoAscenso;
        faseStr  = ui_nombreFaseVuelo(telemetria->faseActual);
        srbStr   = ui_nombreEstadoSRB(telemetria->estadoSRB);
        coreStr  = ui_nombreEstadoCore(telemetria->estadoMotorPrincipal);
    } else {
        snprintf(tiempoStr, sizeof(tiempoStr), "T+ 00:00:00");
        snprintf(altitudStr, sizeof(altitudStr), "-- m");
        snprintf(velocidadStr, sizeof(velocidadStr), "-- m/s");
        snprintf(aceleracionStr, sizeof(aceleracionStr), "-- m/s^2");
    }

    printf("\n  %sARTEMIS / TELEMETRIA EN TIEMPO REAL%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    ui_mostrarSeparador();

    printf("\n  %s%-24s%s %s%s%s\n\n",
           ui_color(&opt, COL_GRIS), "TIEMPO DE MISION", ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), tiempoStr, ui_color(&opt, ANSI_RESET));

    printf("  %s%-24s%s %s%s%s\n",
           ui_color(&opt, COL_GRIS), "ALTITUD", ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_CELESTE), altitudStr, ui_color(&opt, ANSI_RESET));
    printf("  %s%-24s%s %s%s%s\n",
           ui_color(&opt, COL_GRIS), "VELOCIDAD", ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_CELESTE), velocidadStr, ui_color(&opt, ANSI_RESET));
    printf("  %s%-24s%s %s%s%s\n\n",
           ui_color(&opt, COL_GRIS), "ACELERACION", ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_CELESTE), aceleracionStr, ui_color(&opt, ANSI_RESET));

    printf("  %s%-24s%s %s%s%s\n",
           ui_color(&opt, COL_GRIS), "FASE DE VUELO", ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), faseStr, ui_color(&opt, ANSI_RESET));
    printf("  %s%-24s%s %s%s%s\n",
           ui_color(&opt, COL_GRIS), "ESTADO PROPULSORES SRB", ui_color(&opt, ANSI_RESET),
           (hayDatos && telemetria->estadoSRB == UI_SRB_SEPARATED)
               ? ui_color(&opt, COL_AMARILLO)
               : ui_color(&opt, COL_VERDE),
           srbStr,
           ui_color(&opt, ANSI_RESET));
    printf("  %s%-24s%s %s%s%s\n\n",
           ui_color(&opt, COL_GRIS), "MOTOR PRINCIPAL (CORE)", ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_VERDE), coreStr, ui_color(&opt, ANSI_RESET));

    ui_mostrarProgresoEtiquetado(
        "PROGRESO DE ASCENSO (HACIA ALTITUD DE CORTE: 160.000 m)",
        progreso,
        opt.modoCompacto ? 24 : 34
    );

    printf("\n");
    ui_mostrarSeparador();
    printf("\n");

    ui_mostrarListaEventos(eventos, cantidadEventos);
}

/* --------------------------------------------------------------------------
 * SECCION H: ANALISIS CIENTIFICO
 * Nota para el companero de `fisica.c`:
 * Cuando la simulacion calcule los 4 resultados del escenario, armar una
 * variable `UiAnalisisCientifico`, poner `resultadosDisponibles = true` y
 * pasarla a esta funcion.
 * -------------------------------------------------------------------------- */
void ui_mostrarAnalisisCientifico(const UiAnalisisCientifico *analisis) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    bool disponible = (analisis != NULL && analisis->resultadosDisponibles);

    ui_mostrarEncabezado(
        "ANALISIS CIENTIFICO DE LA MISION",
        disponible ? UI_MISION_COMPLETADA : UI_MISION_STANDBY
    );

    if (analisis != NULL && analisis->esDemostracion && disponible) {
        printf("\n");
        ui_mostrarAdvertencia(
            "MODO DEMOSTRACION VISUAL: Estructura de ejemplo para verificar el formato de presentacion."
        );
    }

    if (!disponible) {
        printf("\n");
        ui_mostrarAdvertencia("Sin resultados disponibles. Ejecute una simulacion.");
    }

    /* Categoria 1 */
    printf("\n  %s1. VELOCIDAD Y ALTITUD DURANTE LA SEPARACION SRB%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sConcepto: MRUV en Fase 1 con aceleracion constante a1 hasta t = t_SRB.%s\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
    if (disponible) {
        char bufVel[40];
        char bufAlt[40];
        char bufTmp[40];
        snprintf(bufTmp, sizeof(bufTmp), "T+ %.2f s", analisis->tiempoSeparacionSRB);
        snprintf(bufVel, sizeof(bufVel), "%.2f m/s", analisis->velocidadSeparacionSRB);
        snprintf(bufAlt, sizeof(bufAlt), "%.2f m (%.2f km)",
                 analisis->altitudSeparacionSRB, analisis->altitudSeparacionSRB / 1000.0);
        ui_mostrarTextoDestacado("Instante de separacion:", bufTmp);
        ui_mostrarTextoDestacado("Velocidad en separacion:", bufVel);
        ui_mostrarTextoDestacado("Altitud en separacion:", bufAlt);
    } else {
        printf("  %s-> Sin resultados disponibles. Ejecute una simulacion.%s\n",
               ui_color(&opt, COL_AMARILLO), ui_color(&opt, ANSI_RESET));
    }

    /* Categoria 2 */
    printf("\n  %s2. VELOCIDAD MAXIMA AL CORTE DE MOTORES (MECO)%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sConcepto: MRUV en Fase 2 con aceleracion a2 desde h_SRB hasta alcanzar h_corte.%s\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
    if (disponible) {
        char bufVelMax[40];
        char bufAltCorte[40];
        char bufTmpCorte[40];
        snprintf(bufAltCorte, sizeof(bufAltCorte), "%.2f m", analisis->altitudCorteMotores);
        snprintf(bufTmpCorte, sizeof(bufTmpCorte), "T+ %.2f s", analisis->tiempoCorteMotores);
        snprintf(bufVelMax, sizeof(bufVelMax), "%.2f m/s", analisis->velocidadMaximaCorte);
        ui_mostrarTextoDestacado("Altitud de corte (MECO):", bufAltCorte);
        ui_mostrarTextoDestacado("Instante de corte:", bufTmpCorte);
        ui_mostrarTextoDestacado("Velocidad maxima al corte:", bufVelMax);
    } else {
        printf("  %s-> Sin resultados disponibles. Ejecute una simulacion.%s\n",
               ui_color(&opt, COL_AMARILLO), ui_color(&opt, ANSI_RESET));
    }

    /* Categoria 3 */
    printf("\n  %s3. ALTITUD MAXIMA POSTERIOR AL APAGADO (APOGEO BALISTICO)%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sConcepto: Ascenso inercial desacelerado por la gravedad g tras el apagado de motores.%s\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
    if (disponible) {
        char bufApogeo[48];
        char bufGanancia[40];
        char bufTmpApogeo[40];
        snprintf(bufApogeo, sizeof(bufApogeo), "%.2f m (%.2f km)",
                 analisis->altitudMaximaApogeo, analisis->altitudMaximaApogeo / 1000.0);
        snprintf(bufGanancia, sizeof(bufGanancia), "+%.2f m tras el corte",
                 analisis->gananciaAltitudBalistica);
        snprintf(bufTmpApogeo, sizeof(bufTmpApogeo), "T+ %.2f s", analisis->tiempoApogeo);
        ui_mostrarTextoDestacado("Altitud maxima total:", bufApogeo);
        ui_mostrarTextoDestacado("Tramo inercial libre:", bufGanancia);
        ui_mostrarTextoDestacado("Tiempo hasta apogeo:", bufTmpApogeo);
    } else {
        printf("  %s-> Sin resultados disponibles. Ejecute una simulacion.%s\n",
               ui_color(&opt, COL_AMARILLO), ui_color(&opt, ANSI_RESET));
    }

    /* Categoria 4 */
    printf("\n  %s4. ACELERACION EXPRESADA EN MULTIPLOS DE GRAVEDAD (UNIDADES G)%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sConcepto: Relacion adimensional entre la aceleracion de cada fase y la gravedad g.%s\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
    if (disponible) {
        char bufG1[32];
        char bufG2[32];
        char bufGMax[32];
        snprintf(bufG1, sizeof(bufG1), "%.2f g", analisis->aceleracionFase1EnG);
        snprintf(bufG2, sizeof(bufG2), "%.2f g", analisis->aceleracionFase2EnG);
        snprintf(bufGMax, sizeof(bufGMax), "%.2f g", analisis->aceleracionMaximaEnG);
        ui_mostrarTextoDestacado("Aceleracion Fase 1:", bufG1);
        ui_mostrarTextoDestacado("Aceleracion Fase 2:", bufG2);
        ui_mostrarTextoDestacado("Factor de carga maximo:", bufGMax);
    } else {
        printf("  %s-> Sin resultados disponibles. Ejecute una simulacion.%s\n",
               ui_color(&opt, COL_AMARILLO), ui_color(&opt, ANSI_RESET));
    }

    printf("\n");
}

/* --------------------------------------------------------------------------
 * SECCION I: HISTORIAL DE MISIONES
 * -------------------------------------------------------------------------- */
void ui_mostrarHistorialMisiones(const UiRegistroMision *registros, size_t cantidad) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    ui_mostrarEncabezado("HISTORIAL DE MISIONES", UI_MISION_STANDBY);

    printf("\n  %sHISTORIAL DE MISIONES%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));

    /* Si todavia no se registraron misiones, muestra un estado vacio elegante */
    if (registros == NULL || cantidad == 0) {
        const char *tl = opt.usarUnicode ? BOX_TL_UTF8 : "+";
        const char *tr = opt.usarUnicode ? BOX_TR_UTF8 : "+";
        const char *bl = opt.usarUnicode ? BOX_BL_UTF8 : "+";
        const char *br = opt.usarUnicode ? BOX_BR_UTF8 : "+";
        const char *v  = opt.usarUnicode ? BOX_V_UTF8  : "|";

        printf("  %s%s", ui_color(&opt, COL_GRIS_OSCURO), tl);
        ui_imprimirTrazoHorizontal(&opt, 52, false);
        printf("%s%s\n", tr, ui_color(&opt, ANSI_RESET));

        printf("  %s%s%s  %sAun no hay misiones registradas.%s                  %s%s%s\n",
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_BLANCO_SUAVE), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET));
        printf("  %s%s%s  %sSin misiones registradas en la sesion actual.%s     %s%s%s\n",
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET));
        printf("  %s%s%s                                                    %s%s%s\n",
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET));
        printf("  %s%s%s  %sComplete su primera simulacion para%s               %s%s%s\n",
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET));
        printf("  %s%s%s  %sgenerar resultados de mision.%s                     %s%s%s\n",
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), v, ui_color(&opt, ANSI_RESET));

        printf("  %s%s", ui_color(&opt, COL_GRIS_OSCURO), bl);
        ui_imprimirTrazoHorizontal(&opt, 52, false);
        printf("%s%s\n\n", br, ui_color(&opt, ANSI_RESET));
        return;
    }

    const char *sepV = opt.usarUnicode ? BOX_V_UTF8 : "|";
    printf("  %s%-8s %s %-14s %s %-18s %s %-14s %s %-12s %s %-10s%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO),
           "MISION", sepV, "SESION", sepV, "ESTADO FINAL", sepV,
           "ALT. MAX (m)", sepV, "VEL. MAX", sepV, "DURACION",
           ui_color(&opt, ANSI_RESET));
    ui_mostrarSeparador();

    for (size_t i = 0; i < cantidad; ++i) {
        char idStr[16];
        char altStr[20];
        char velStr[20];
        char durStr[20];

        snprintf(idStr, sizeof(idStr), "#%03d", registros[i].idMision);
        snprintf(altStr, sizeof(altStr), "%.1f m", registros[i].altitudMaxima);
        snprintf(velStr, sizeof(velStr), "%.1f m/s", registros[i].velocidadMaxima);
        snprintf(durStr, sizeof(durStr), "%.1f s", registros[i].duracionTotal);

        printf("  %s%-8s%s %s%s%s %s%-14s%s %s%s%s %s%-18s%s %s%s%s %s%-14s%s %s%s%s %s%-12s%s %s%s%s %s%-10s%s\n",
               ui_color(&opt, COL_BLANCO_SUAVE), idStr, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS),
               registros[i].identificadorSesion != NULL ? registros[i].identificadorSesion : "SESION-LOCAL",
               ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET),
               ui_colorEstadoMision(&opt, registros[i].estadoFinal),
               ui_nombreEstadoMision(registros[i].estadoFinal),
               ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), altStr, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_CELESTE), velStr, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_GRIS_OSCURO), sepV, ui_color(&opt, ANSI_RESET),
               ui_color(&opt, COL_BLANCO_SUAVE), durStr, ui_color(&opt, ANSI_RESET));
    }
    printf("\n");
}

/* --------------------------------------------------------------------------
 * SECCION J: REPORTE FINAL DE MISION
 * Nota para el companero de `reportes.c`:
 * Esta funcion muestra el resumen completo al terminar la simulacion.
 * -------------------------------------------------------------------------- */
void ui_mostrarReporteFinal(const UiReporteFinal *reporte) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();

    if (reporte == NULL || !reporte->disponible) {
        ui_mostrarEncabezado("REPORTE FINAL DE MISION", UI_MISION_STANDBY);
        printf("\n");
        ui_mostrarAdvertencia("Sin resultados disponibles. Ejecute una simulacion.");
        printf("\n");
        return;
    }

    ui_mostrarEncabezado("REPORTE FINAL DE MISION", reporte->estadoFinal);

    if (reporte->esDemostracion) {
        printf("\n");
        ui_mostrarAdvertencia(
            "MODO DEMOSTRACION VISUAL: Reporte renderizado a partir de una estructura de prueba."
        );
    }

    char bufId[32];
    char bufVelSrb[40];
    char bufAltSrb[40];
    char bufVelCorte[40];
    char bufAltMax[48];
    char bufGMax[32];
    char bufDuracion[32];

    snprintf(bufId, sizeof(bufId), "ARTEMIS-SLS #%03d", reporte->idMision);
    snprintf(bufVelSrb, sizeof(bufVelSrb), "%.2f m/s", reporte->velocidadSeparacionSRB);
    snprintf(bufAltSrb, sizeof(bufAltSrb), "%.2f m", reporte->altitudSeparacionSRB);
    snprintf(bufVelCorte, sizeof(bufVelCorte), "%.2f m/s", reporte->velocidadCorteMotores);
    snprintf(bufAltMax, sizeof(bufAltMax), "%.2f m (%.2f km)",
             reporte->altitudMaximaCalculada, reporte->altitudMaximaCalculada / 1000.0);
    snprintf(bufGMax, sizeof(bufGMax), "%.2f g", reporte->aceleracionMaximaG);
    snprintf(bufDuracion, sizeof(bufDuracion), "%.2f s", reporte->duracionTotalSegundos);

    printf("\n  %s1. RESUMEN EJECUTIVO DE MISION%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    ui_mostrarTextoDestacado("Identificador:", bufId);
    ui_mostrarTextoDestacado("Estado final:", ui_nombreEstadoMision(reporte->estadoFinal));
    ui_mostrarTextoDestacado("Duracion total:", bufDuracion);

    printf("\n  %s2. PARAMETROS FISICOS UTILIZADOS%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sv0 = %.2f m/s  |  h0 = %.2f m  |  a1 = %.2f m/s^2  |  t_SRB = %.2f s%s\n",
           ui_color(&opt, COL_GRIS),
           reporte->parametrosUtilizados.velocidadInicial,
           reporte->parametrosUtilizados.altitudInicial,
           reporte->parametrosUtilizados.aceleracionFase1,
           reporte->parametrosUtilizados.tiempoSeparacionSRB,
           ui_color(&opt, ANSI_RESET));
    printf("  %sa2 = %.2f m/s^2 |  h_corte = %.2f m  |  g = %.2f m/s^2%s\n",
           ui_color(&opt, COL_GRIS),
           reporte->parametrosUtilizados.aceleracionFase2,
           reporte->parametrosUtilizados.altitudCorte,
           reporte->parametrosUtilizados.gravedad,
           ui_color(&opt, ANSI_RESET));

    printf("\n  %s3. INDICADORES DE DESEMPENO CINEMATICO%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    ui_mostrarTextoDestacado("Velocidad en sep. SRB:", bufVelSrb);
    ui_mostrarTextoDestacado("Altitud en sep. SRB:", bufAltSrb);
    ui_mostrarTextoDestacado("Velocidad al corte:", bufVelCorte);
    ui_mostrarTextoDestacado("Altitud maxima (apogeo):", bufAltMax);
    ui_mostrarTextoDestacado("Aceleracion maxima:", bufGMax);

    printf("\n");
    ui_mostrarSeparador();
    printf("\n");
    ui_mostrarListaEventos(reporte->eventosRelevantes, reporte->cantidadEventos);

    if (reporte->observaciones != NULL && reporte->observaciones[0] != '\0') {
        printf("\n");
        ui_mostrarInfo(reporte->observaciones);
    }

    printf("\n  %s[Enter / Q]%s %sVolver al menu principal%s\n",
           ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), ui_color(&opt, ANSI_RESET));
}

/* --------------------------------------------------------------------------
 * SECCION K: INSTRUCCIONES
 * -------------------------------------------------------------------------- */
void ui_mostrarInstrucciones(void) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    ui_mostrarEncabezado("INSTRUCCIONES Y MANUAL OPERATIVO", UI_MISION_STANDBY);

    printf("\n  %s1. OBJETIVO DEL SIMULADOR%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sModelar y analizar el Escenario 1 de la mision Artemis: el ascenso vertical y la\n"
           "  propulsion por etapas del cohete SLS desde el despegue hasta alcanzar la altitud\n"
           "  de corte de motores y su posterior altitud maxima (apogeo balistico).%s\n",
           ui_color(&opt, COL_BLANCO_SUAVE), ui_color(&opt, ANSI_RESET));

    printf("\n  %s2. FUNCIONAMIENTO GENERAL Y CONFIGURACION%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %s- En 'Configuracion de mision' se consultan los 7 parametros fisicos del escenario.\n"
           "  - Al 'Iniciar mision' se presenta la verificacion pre-vuelo, la cuenta regresiva y\n"
           "    la telemetria paso a paso alimentada por el modulo simulador.c.\n"
           "  - Al finalizar, 'Analisis cientifico' y 'Reporte final' exponen los resultados.%s\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));

    printf("\n  %s3. COMO INTERPRETAR LA TELEMETRIA%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %s- TIEMPO DE MISION (T+): Segundos transcurridos desde el despegue.\n"
           "  - ALTITUD / VELOCIDAD / ACELERACION: Magnitudes cinematicas en unidades del SI.\n"
           "  - ESTADO PROPULSORES SRB / MOTOR PRINCIPAL: Estado de cada etapa de propulsion.\n"
           "  - PROGRESO DE ASCENSO: Porcentaje completado hacia la altitud de corte (160.000 m).%s\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));

    printf("\n  %s4. GLOSARIO TECNICO AEROESPACIAL%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    ui_mostrarTextoDestacado("SLS (Sistema Lanzador)",
                             "Vehiculo lanzador espacial superpesado del programa Artemis.");
    ui_mostrarTextoDestacado("SRB (Propulsor Solido)",
                             "Propulsores laterales de combustible solido activos durante los primeros 120 s.");
    ui_mostrarTextoDestacado("MRUV",
                             "Movimiento Rectilineo Uniformemente Variado (aceleracion constante por tramo).");
    ui_mostrarTextoDestacado("MECO (Corte de Motores)",
                             "Apagado de los motores principales al alcanzar la altitud objetivo de corte.");

    printf("\n  %s5. CONTROLES DE NAVEGACION%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %sUtilice las flechas Arriba/Abajo (o teclas 1-7) para desplazarse, Enter para abrir\n"
           "  la seccion seleccionada, H/? para ayuda rapida y Escape/Q para regresar.%s\n\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
}

/* --------------------------------------------------------------------------
 * SECCION L: INFORMACION DEL SISTEMA
 * -------------------------------------------------------------------------- */
void ui_mostrarInformacionSistema(const UiEstadoModulo *modulos, size_t cantidadModulos) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    ui_mostrarEncabezado("INFORMACION DEL SISTEMA", UI_MISION_STANDBY);

    printf("\n  %sFICHA TECNICA DEL SISTEMA%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));

    ui_mostrarTextoDestacado("Nombre del proyecto:", "Simulador de Misiones Artemis (ARTEMIS CLI)");
    ui_mostrarTextoDestacado("Version de interfaz:", UI_VERSION_SISTEMA);
    ui_mostrarTextoDestacado("Lenguaje y estandar:", "Lenguaje C - Estandar ISO/IEC 9899:2011 (C11)");
    ui_mostrarTextoDestacado("Escenario asignado:", UI_ESCENARIO_NOMBRE);
    ui_mostrarTextoDestacado("Universidad:", UI_UNIVERSIDAD_INFO);
    ui_mostrarTextoDestacado("Asignatura:", UI_ASIGNATURA_INFO);

    printf("\n  %sARQUITECTURA MODULAR DEL PROYECTO%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));
    printf("  %smain.c  --->  simulador.c  --->  [ interfaz.c | fisica.c | validaciones.c | reportes.c ]%s\n\n",
           ui_color(&opt, COL_CELESTE), ui_color(&opt, ANSI_RESET));

    /* Estado real de los modulos (no declara completos modulos pendientes) */
    static const UiEstadoModulo estadoRealPorDefecto[6] = {
        { "src/interfaz.c + include/interfaz.h", "Interfaz CLI, componentes visuales y navegacion", UI_MODULO_IMPLEMENTADO },
        { "src/main.c",                          "Punto de entrada e inicializacion general",       UI_MODULO_PENDIENTE },
        { "src/simulador.c + simulador.h",       "Coordinacion de fases, turnos y estado de mision", UI_MODULO_PENDIENTE },
        { "src/fisica.c + fisica.h",             "Ecuaciones de MRUV, apogeo y conversion a G",     UI_MODULO_PENDIENTE },
        { "src/validaciones.c + validaciones.h", "Lectura segura y validacion de rangos numericos", UI_MODULO_PENDIENTE },
        { "src/reportes.c + reportes.h",         "Consolidacion de metricas y reporte final",       UI_MODULO_PENDIENTE }
    };

    const UiEstadoModulo *lista = (modulos != NULL && cantidadModulos > 0)
        ? modulos
        : estadoRealPorDefecto;
    size_t total = (modulos != NULL && cantidadModulos > 0) ? cantidadModulos : 6;

    printf("  %sESTADO DE IMPLEMENTACION DE LOS MODULOS%s\n\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET));

    for (size_t i = 0; i < total; ++i) {
        UiEstadoSistema estVisual = UI_ESTADO_WAIT;
        const char *detalleEstado = "PENDIENTE DE INTEGRACION";

        if (lista[i].estado == UI_MODULO_IMPLEMENTADO) {
            estVisual     = UI_ESTADO_OK;
            detalleEstado = "IMPLEMENTADO - Operativo";
        } else if (lista[i].estado == UI_MODULO_EN_DESARROLLO) {
            estVisual     = UI_ESTADO_INFO;
            detalleEstado = "EN DESARROLLO";
        }

        char detalleCompleto[128];
        snprintf(detalleCompleto, sizeof(detalleCompleto), "%s (%s)",
                 detalleEstado,
                 lista[i].responsabilidad != NULL ? lista[i].responsabilidad : "");

        ui_mostrarEstadoSistema(lista[i].nombreModulo, estVisual, detalleCompleto);
    }
    printf("\n");
}

void ui_mostrarDespedida(void) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    printf("\n");
    ui_mostrarSeparadorDoble();
    printf("  %sARTEMIS CLI%s %s- Sesion de consola finalizada correctamente.%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO), ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_BLANCO_SUAVE), ui_color(&opt, ANSI_RESET));
    printf("  %sControl de Mision SLS / Sistema de Simulacion de Vuelo - UTN ISI%s\n",
           ui_color(&opt, COL_GRIS), ui_color(&opt, ANSI_RESET));
    ui_mostrarSeparadorDoble();
    printf("\n");
}

/* ==========================================================================
 * 7. SISTEMA DE NAVEGACION Y LECTURA DE TECLADO (SECCION 6)
 * ==========================================================================
 * Nota tecnica para el equipo:
 * Aqui se implementa la lectura de teclado en dos niveles:
 *   1. Nivel interactivo (flechas Arriba/Abajo, Enter, Esc, Q, H, numeros 1-7).
 *   2. Nivel alternativo con `fgets()` para terminales basicas o entrada
 *      redirigida, evitando por completo funciones inseguras como `gets()`.
 */

/**
 * @brief Convierte un caracter ASCII simple en una accion del enum `UiAccionTecla`.
 */
static UiAccionTecla ui_mapearCaracterSimple(int ch) {
    switch (ch) {
        case '\r':
        case '\n':
            return UI_TECLA_ENTER;
        case 27:
            return UI_TECLA_ESCAPE;
        case 'q':
        case 'Q':
            return UI_TECLA_SALIR;
        case 'h':
        case 'H':
        case '?':
            return UI_TECLA_AYUDA;
        case 'd':
        case 'D':
            return UI_TECLA_DEMO_TOGGLE;
        case 'c':
        case 'C':
            return UI_TECLA_COLOR_TOGGLE;
        case 'w':
        case 'W':
            return UI_TECLA_COMPACTO_TOGGLE;
        case 'k':
        case 'K':
            return UI_TECLA_ARRIBA;
        case 'j':
        case 'J':
            return UI_TECLA_ABAJO;
        case '1': return UI_TECLA_NUMERO_1;
        case '2': return UI_TECLA_NUMERO_2;
        case '3': return UI_TECLA_NUMERO_3;
        case '4': return UI_TECLA_NUMERO_4;
        case '5': return UI_TECLA_NUMERO_5;
        case '6': return UI_TECLA_NUMERO_6;
        case '7': return UI_TECLA_NUMERO_7;
        case '8': return UI_TECLA_NUMERO_8;
        case '9': return UI_TECLA_NUMERO_9;
        default:
            return UI_TECLA_NINGUNA;
    }
}

/**
 * @brief Alternativa universal de navegacion: lee una linea con `fgets()`
 *        cuando la terminal no permite capturar teclas individuales.
 */
static UiAccionTecla ui_leerAccionPorLinea(void) {
    char buffer[64];
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();

    printf("  %s%s%s %sIngrese opcion (1-7, Enter, H, Q): %s",
           ui_color(&opt, COL_AZUL_ELECTRICO),
           opt.usarUnicode ? SYM_CURSOR_UTF8 : ">",
           ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_GRIS),
           ui_color(&opt, ANSI_RESET));
    fflush(stdout);

    if (fgets(buffer, (int)sizeof(buffer), stdin) == NULL) {
        return UI_TECLA_SALIR;
    }

    size_t i = 0;
    while (buffer[i] != '\0' && isspace((unsigned char)buffer[i])) {
        ++i;
    }

    if (buffer[i] == '\0') {
        return UI_TECLA_ENTER;
    }

    return ui_mapearCaracterSimple((unsigned char)buffer[i]);
}

UiAccionTecla ui_leerAccionTeclado(void) {
    /* Si se definio ARTEMIS_LINE_INPUT=1, usa directamente el modo por linea */
    const char *forzarLinea = getenv("ARTEMIS_LINE_INPUT");
    if (forzarLinea != NULL && forzarLinea[0] != '\0' && strcmp(forzarLinea, "0") != 0) {
        return ui_leerAccionPorLinea();
    }

#ifdef _WIN32
    /* Captura de eventos de teclado de la consola de Windows sin usar <conio.h> */
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD modoConsola = 0;
    if (hIn != INVALID_HANDLE_VALUE && GetConsoleMode(hIn, &modoConsola)) {
        INPUT_RECORD registro;
        DWORD leidos = 0;

        while (ReadConsoleInputA(hIn, &registro, 1, &leidos) && leidos > 0) {
            /* Ignoramos eventos de raton o de soltar una tecla (solo procesamos al presionar) */
            if (registro.EventType != KEY_EVENT || !registro.Event.KeyEvent.bKeyDown) {
                continue;
            }

            WORD vk = registro.Event.KeyEvent.wVirtualKeyCode;
            char ch = registro.Event.KeyEvent.uChar.AsciiChar;

            switch (vk) {
                case VK_UP:     return UI_TECLA_ARRIBA;
                case VK_DOWN:   return UI_TECLA_ABAJO;
                case VK_LEFT:   return UI_TECLA_IZQUIERDA;
                case VK_RIGHT:  return UI_TECLA_DERECHA;
                case VK_RETURN: return UI_TECLA_ENTER;
                case VK_ESCAPE: return UI_TECLA_ESCAPE;
                default:
                    break;
            }

            if (ch != 0) {
                UiAccionTecla accion = ui_mapearCaracterSimple((unsigned char)ch);
                if (accion != UI_TECLA_NINGUNA) {
                    return accion;
                }
            }
        }
    }
    return ui_leerAccionPorLinea();

#elif defined(__unix__) || defined(__APPLE__)
    /* Captura de teclas en Linux / macOS usando termios en modo no canonico */
    if (isatty(STDIN_FILENO)) {
        struct termios original;
        if (tcgetattr(STDIN_FILENO, &original) == 0) {
            struct termios crudo = original;
            crudo.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
            crudo.c_cc[VMIN]  = 1;
            crudo.c_cc[VTIME] = 0;

            if (tcsetattr(STDIN_FILENO, TCSANOW, &crudo) == 0) {
                unsigned char ch = 0;
                ssize_t n = read(STDIN_FILENO, &ch, 1);
                UiAccionTecla resultado = UI_TECLA_NINGUNA;

                if (n == 1) {
                    if (ch == 27) {
                        /* En terminales POSIX, las flechas envian "\033[A", "\033[B", etc. */
                        fd_set setLectura;
                        struct timeval espera;
                        FD_ZERO(&setLectura);
                        FD_SET(STDIN_FILENO, &setLectura);
                        espera.tv_sec  = 0;
                        espera.tv_usec = 40000; /* Espera 40 ms para distinguir Esc de una flecha */

                        if (select(STDIN_FILENO + 1, &setLectura, NULL, NULL, &espera) > 0) {
                            unsigned char seq[2] = {0, 0};
                            if (read(STDIN_FILENO, &seq[0], 1) == 1 && seq[0] == '[') {
                                if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                                    switch (seq[1]) {
                                        case 'A': resultado = UI_TECLA_ARRIBA;    break;
                                        case 'B': resultado = UI_TECLA_ABAJO;     break;
                                        case 'C': resultado = UI_TECLA_DERECHA;   break;
                                        case 'D': resultado = UI_TECLA_IZQUIERDA; break;
                                        default:  resultado = UI_TECLA_ESCAPE;    break;
                                    }
                                }
                            } else {
                                resultado = UI_TECLA_ESCAPE;
                            }
                        } else {
                            resultado = UI_TECLA_ESCAPE;
                        }
                    } else {
                        resultado = ui_mapearCaracterSimple(ch);
                    }
                }

                /* Restauramos el estado original de la terminal antes de salir */
                tcsetattr(STDIN_FILENO, TCSANOW, &original);
                if (resultado != UI_TECLA_NINGUNA) {
                    return resultado;
                }
            }
        }
    }
    return ui_leerAccionPorLinea();
#else
    return ui_leerAccionPorLinea();
#endif
}

void ui_esperarConfirmacion(const char *mensaje) {
    UiOpcionesVisuales opt = ui_obtenerOpcionesVisuales();
    const char *prompt = (mensaje != NULL && mensaje[0] != '\0')
        ? mensaje
        : "Presione Enter, Escape o Q para regresar al menu...";

    ui_mostrarSeparador();
    printf("  %s%s%s %s%s%s\n",
           ui_color(&opt, COL_AZUL_ELECTRICO),
           opt.usarUnicode ? SYM_CURSOR_UTF8 : ">",
           ui_color(&opt, ANSI_RESET),
           ui_color(&opt, COL_CELESTE),
           prompt,
           ui_color(&opt, ANSI_RESET));
    fflush(stdout);

    for (;;) {
        UiAccionTecla accion = ui_leerAccionTeclado();
        if (accion == UI_TECLA_ENTER ||
            accion == UI_TECLA_ESCAPE ||
            accion == UI_TECLA_SALIR ||
            accion == UI_TECLA_NUMERO_1) {
            break;
        }
    }
}

UiOpcionMenu ui_ejecutarMenuInteractivo(
    UiOpcionMenu opcionInicial,
    UiEstadoMisionGeneral estadoSistema,
    const UiOpcionesVisuales *opciones
) {
    UiOpcionMenu actual = opcionInicial;
    if (actual < UI_MENU_INICIAR_MISION || actual > UI_MENU_TOTAL_OPCIONES) {
        actual = UI_MENU_INICIAR_MISION;
    }

    for (;;) {
        ui_limpiarPantalla(false);
        ui_mostrarPantallaPrincipal(actual, estadoSistema, opciones);

        UiAccionTecla tecla = ui_leerAccionTeclado();
        switch (tecla) {
            case UI_TECLA_ARRIBA:
                /* Si sube desde la primera opcion, da la vuelta hasta la ultima (Salir) */
                actual = (actual == UI_MENU_INICIAR_MISION)
                    ? UI_MENU_SALIR
                    : (UiOpcionMenu)(actual - 1);
                break;

            case UI_TECLA_ABAJO:
                /* Si baja desde la ultima opcion, vuelve a la primera (Iniciar mision) */
                actual = (actual == UI_MENU_SALIR)
                    ? UI_MENU_INICIAR_MISION
                    : (UiOpcionMenu)(actual + 1);
                break;

            case UI_TECLA_ENTER:
                return actual;

            case UI_TECLA_SALIR:
            case UI_TECLA_ESCAPE:
                return UI_MENU_SALIR;

            case UI_TECLA_AYUDA:
                ui_limpiarPantalla(false);
                ui_mostrarAyudaContextual();
                ui_esperarConfirmacion(NULL);
                break;

            /* Seleccion directa presionando los numeros del 1 al 7 */
            case UI_TECLA_NUMERO_1: return UI_MENU_INICIAR_MISION;
            case UI_TECLA_NUMERO_2: return UI_MENU_CONFIGURACION;
            case UI_TECLA_NUMERO_3: return UI_MENU_ANALISIS_CIENTIFICO;
            case UI_TECLA_NUMERO_4: return UI_MENU_HISTORIAL;
            case UI_TECLA_NUMERO_5: return UI_MENU_INSTRUCCIONES;
            case UI_TECLA_NUMERO_6: return UI_MENU_INFO_SISTEMA;
            case UI_TECLA_NUMERO_7: return UI_MENU_SALIR;

            default:
                break;
        }
    }
}

/* ==========================================================================
 * 8. FUNCIONES PUENTE DE COMPATIBILIDAD CON EL README DEL PROYECTO
 * ========================================================================== */

void mostrarBienvenida(void) {
    ui_mostrarBienvenida();
}

void mostrarMenu(void) {
    ui_mostrarMenuPrincipal(UI_MENU_INICIAR_MISION);
}

void mostrarInstrucciones(void) {
    ui_mostrarInstrucciones();
}

void mostrarFaseActual(int fase) {
    UiFaseVuelo faseEnum = UI_FASE_STANDBY;
    if (fase >= (int)UI_FASE_STANDBY && fase <= (int)UI_FASE_ABORTADA) {
        faseEnum = (UiFaseVuelo)fase;
    }
    ui_mostrarTextoDestacado("FASE OPERATIVA ACTUAL:", ui_nombreFaseVuelo(faseEnum));
}

void mostrarDespedida(void) {
    ui_mostrarDespedida();
}
