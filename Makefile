ifeq ($(OS),Windows_NT)
    TARGET = bin/artemis.exe
    RM = del /Q /F
    MKDIR = if not exist bin mkdir bin
else
    TARGET = bin/artemis
    RM = rm -rf
    MKDIR = mkdir -p bin
endif

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
LIBS = -lm

SRC = src/main.c src/interfaz.c src/simulador.c src/fisica.c src/validaciones.c src/reportes.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(MKDIR)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

clean:
	$(RM) $(TARGET) *.o

.PHONY: all clean
