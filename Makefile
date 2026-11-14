# Compilacion del Simulador ARTEMIS
#   make        -> compila en bin/
#   make test   -> compila y ejecuta la autoprueba (exit code 0 = OK)
#   make clean  -> borra el ejecutable
# La carpeta bin/ ya viene creada en el repositorio (bin/.gitkeep), asi el
# Makefile funciona igual desde Linux, MSYS2 o mingw32-make en Windows.

ifeq ($(OS),Windows_NT)
    EXE = .exe
else
    EXE =
endif

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
LIBS    = -lm
TARGET  = bin/artemis$(EXE)

SRC = src/main.c src/interfaz.c src/simulador.c src/fisica.c src/validaciones.c src/reportes.c
HDR = include/fisica.h include/interfaz.h include/simulador.h include/validaciones.h include/reportes.h

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

test: $(TARGET)
	./$(TARGET) -t

clean:
	-rm -f bin/artemis bin/artemis.exe reporte_mision_artemis.txt

.PHONY: all test clean
